#include <signal.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include "../src/logger.h"
#include "kdm_decoder.h"


typedef struct {
    int width, height, sample_rate, channels;
    double fps;

    AVCodecContext* enc_ctx;
    AVFrame* raw_frame;
    AVPacket* enc_pkt;

    int stream_index;

    enum AVCodecID codec_id;
    enum AVMediaType codec_type;
    int format_pixel_or_sample;
    int audio_frame_size;
} StreamInfo;

static volatile bool has_int_signal = false;

static int encode(AVFormatContext* ofmt_ctx, StreamInfo* info, bool flush) {
    AVFrame* raw_frame = flush ? NULL : info->raw_frame;
    AVPacket* enc_pkt = info->enc_pkt;

    av_packet_unref(enc_pkt);

    if (raw_frame && raw_frame->pts != AV_NOPTS_VALUE) {
        raw_frame->pts = av_rescale_q(raw_frame->pts, raw_frame->time_base, info->enc_ctx->time_base);
    }

    int ret = avcodec_send_frame(info->enc_ctx, raw_frame);
    if (ret < 0) {
        return ret;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(info->enc_ctx, enc_pkt);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }

        enc_pkt->stream_index = info->stream_index;
        av_packet_rescale_ts(enc_pkt, info->enc_ctx->time_base, ofmt_ctx->streams[info->stream_index]->time_base);

        ret = av_interleaved_write_frame(ofmt_ctx, enc_pkt);
    }

    return ret;
}

static int flush_encoder(AVFormatContext* ofmt_ctx, StreamInfo* info) {
    if (info->enc_ctx->codec->capabilities & AV_CODEC_CAP_DELAY)
        return encode(ofmt_ctx, info, true);
    else
        return 0;
}

static int create_output_stream(AVFormatContext* ofmt_ctx, StreamInfo* info) {
    AVStream* out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if (!out_stream) {
        av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream #%i\n", ofmt_ctx->nb_streams);
        return AVERROR_UNKNOWN;
    }

    const AVCodec* encoder = avcodec_find_encoder(info->codec_id);
    if (!encoder) {
        av_log(NULL, AV_LOG_FATAL, "Encoder not found:%s\n", avcodec_get_name(info->codec_id));
        return AVERROR_INVALIDDATA;
    }

    AVCodecContext* enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx) {
        av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context for %s\n", encoder->name);
        return AVERROR(ENOMEM);
    }

    if (info->codec_type == AVMEDIA_TYPE_VIDEO) {
        enc_ctx->width = info->width;
        enc_ctx->height = info->height;
        enc_ctx->time_base = av_make_q(1, info->fps);
        enc_ctx->framerate = av_make_q(info->fps, 1);
        enc_ctx->pix_fmt = info->format_pixel_or_sample;
    } else {
        enc_ctx->sample_rate = info->sample_rate;
        enc_ctx->sample_fmt = info->format_pixel_or_sample;
        enc_ctx->ch_layout.opaque = NULL;
        av_channel_layout_default(&enc_ctx->ch_layout, info->channels);
        enc_ctx->sample_fmt = encoder->sample_fmts[0];
        enc_ctx->time_base = (AVRational){1, info->sample_rate};
        if (info->audio_frame_size > 0) enc_ctx->frame_size = info->audio_frame_size;
    }

    if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    int ret = avcodec_open2(enc_ctx, encoder, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to open %s encoder for stream #%i\n", encoder->name, info->stream_index);
        return ret;
    }

    ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%i\n", info->stream_index);
        return ret;
    }

    info->enc_ctx = enc_ctx;
    info->stream_index = out_stream->index;
    out_stream->time_base = enc_ctx->time_base;

    return 0;
}

static int init_output_file(const char* filename, AVFormatContext** ofmt, StreamInfo* video, StreamInfo* audio) {
    int ret;
    AVFormatContext* ofmt_ctx = NULL;

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
    if (!ofmt_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Failed to create output context for %s\n", filename);
        return AVERROR_UNKNOWN;
    }

    if (audio) {
        ret = create_output_stream(ofmt_ctx, audio);
        if (ret != 0) {
            return ret;
        }
    }

    if (video) {
        ret = create_output_stream(ofmt_ctx, video);
        if (ret != 0) {
            return ret;
        }
    }

    av_dump_format(ofmt_ctx, 0, filename, 1);

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Failed to open output file %s\n", filename);
            return ret;
        }
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to write output file header\n");
        return ret;
    }

    *ofmt = ofmt_ctx;
    return 0;
}

static void handle_INT(int sig) {
    // restore INT handler
    signal(sig, SIG_IGN);
    has_int_signal = true;
}


int main(int argc, char** argv) {
    int ret;
    StreamInfo* info_video = NULL;
    StreamInfo* info_audio = NULL;
    AVFormatContext* ofmt_ctx = NULL;
    char* output_filename = NULL;

    if (argc < 2) {
        printf(
            "KDM decoder (Proof-of-concept) v0.1 by kapodamy\n"
            "Usage:\n"
            "       %s <input kdm file> <output media filename>\n"
            "\nSuggested output filename extensions are mkv and avi, the output codecs are HUFFYUV and FLAC.\n"
            "The output file size can be up to 1GiB\n"
            "The decoding does not have hardware acceleration in order to test the correct usage of pl_mpeg library\n",
            argv[0]
        );
        return 1;
    }

    if (argc > 2) {
        output_filename = _strdup(argv[2]);
    } else {
        char* dot_ptr = strchr(argv[1], '.');
        size_t len_in = dot_ptr == NULL ? strlen(argv[1]) : (size_t)(dot_ptr - argv[1]);
        size_t output_filename_length = sizeof(".mkv") + len_in;

        output_filename = malloc(output_filename_length);
        memcpy(output_filename, argv[1], len_in);
        memcpy(output_filename + len_in, ".mkv", sizeof(".mkv"));
    }

    if (!kdmdec_init(argv[1])) {
        av_log(NULL, AV_LOG_ERROR, "Failed to read kdm file: %s\n", output_filename);
        free(output_filename);
        return 1;
    }

    if (kdmdec_has_video()) {
        info_video = calloc(1, sizeof(StreamInfo));
        info_video->codec_type = AVMEDIA_TYPE_VIDEO;
        info_video->codec_id = AV_CODEC_ID_HUFFYUV;
        info_video->format_pixel_or_sample = AV_PIX_FMT_RGB24;

        kdmdec_get_video_params(&info_video->width, &info_video->height, &info_video->fps);
    }

    if (kdmdec_has_audio()) {
        info_audio = calloc(1, sizeof(StreamInfo));
        info_audio->codec_type = AVMEDIA_TYPE_AUDIO;
        info_audio->codec_id = AV_CODEC_ID_FLAC;
        info_audio->format_pixel_or_sample = AV_SAMPLE_FMT_S16;

        kdmdec_get_audio_params(&info_audio->channels, &info_audio->sample_rate);

        if (info_video)
            info_audio->audio_frame_size = (int)ceil((info_audio->sample_rate / (double)info_video->fps) * 1.1);
        else
            info_audio->audio_frame_size = -1;
    }

    if ((ret = init_output_file(output_filename, &ofmt_ctx, info_video, info_audio)) < 0) {
        goto L_dispose_resources;
    }

    if (info_video) {
        info_video->enc_pkt = av_packet_alloc();
        info_video->raw_frame = av_frame_alloc();
        info_video->raw_frame->format = info_video->enc_ctx->pix_fmt;
        info_video->raw_frame->width = info_video->width;
        info_video->raw_frame->height = info_video->height;
        info_video->raw_frame->time_base = av_make_q(1, info_video->fps);

        ret = av_frame_get_buffer(info_video->raw_frame, 0);
        if (ret < 0) goto L_dispose_resources;
    }

    if (info_audio) {
        info_audio->enc_pkt = av_packet_alloc();
        info_audio->raw_frame = av_frame_alloc();
        info_audio->raw_frame->nb_samples = info_audio->audio_frame_size;
        info_audio->raw_frame->format = AV_SAMPLE_FMT_S16;
        info_audio->raw_frame->sample_rate = info_audio->sample_rate;
        info_audio->raw_frame->time_base = av_make_q(1, info_audio->sample_rate);
        av_channel_layout_default(&info_audio->raw_frame->ch_layout, info_audio->channels);

        ret = av_frame_get_buffer(info_audio->raw_frame, 0);
        if (ret < 0) goto L_dispose_resources;
    }

    KDMCue* cue_table;
    int32_t cue_table_length = kdmdec_parse_cue_table(&cue_table);

    if (cue_table_length < 0) {
        av_log(NULL, AV_LOG_ERROR, "Call to kdmdec_parse_cue_table() failed, file is probably truncated.\n");
        ret = AVERROR_EOF;
        goto L_dispose_resources;
    }

    bool read_video = info_video != NULL;
    bool read_audio = info_audio != NULL;

    int64_t fake_pts_video = 0;
    int64_t fake_pts_audio = 0;

    long next_progress = 0;

    signal(SIGINT, handle_INT);

    printf("\n\nDecoding... Press Ctrl-C to stop\n");
    while (read_video || read_audio) {
        bool eof_reached = !kdmdec_parse_next_packet();

        if (info_audio) {
            ret = av_frame_make_writable(info_video->raw_frame);
            if (ret < 0) goto L_dispose_resources;

            int samples_readed = kdmdec_read_audio_samples(
                info_audio->raw_frame->data[0], info_audio->raw_frame->nb_samples, eof_reached
            );

            if (samples_readed > 0) {
                info_audio->raw_frame->pts = fake_pts_audio;
                fake_pts_audio += samples_readed;

                ret = encode(ofmt_ctx, info_audio, false);
                if (ret < 0)
                    goto L_dispose_resources;
            }
        }

        if (read_video) {
            if (kdmdec_read_video_frame(info_video->raw_frame->data[0])) {
                info_video->raw_frame->pts = fake_pts_video++;

                ret = av_frame_make_writable(info_video->raw_frame);
                if (ret < 0) goto L_dispose_resources;

                ret = encode(ofmt_ctx, info_video, false);
                if (ret < 0)
                    goto L_dispose_resources;
            }
        }

        long progess = kdmdec_get_progress();
        if (progess >= next_progress) {
            printf("\r%li%% decoded     ", progess);
            next_progress = progess + 1;
        }

        if (eof_reached) {
            break;
        }
        if (has_int_signal) {
            putchar('\n');
            printf("Decoding was stopped\n");
            break;
        }
    }

    putchar('\n');
    printf("Flusing encoders...\n");
    if (info_video) {
        ret = flush_encoder(ofmt_ctx, info_video);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Flushing video encoder failed\n");
            goto L_dispose_resources;
        }
    }
    if (info_audio) {
        ret = flush_encoder(ofmt_ctx, info_audio);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Flushing audio encoder failed\n");
            goto L_dispose_resources;
        }
    }

    printf("Writing output container trailer...\n");
    av_write_trailer(ofmt_ctx);

L_dispose_resources:
    kdmdec_destroy();

    if (info_audio) {
        if (info_audio->enc_ctx) avcodec_free_context(&info_audio->enc_ctx);
        if (info_audio->raw_frame) av_frame_free(&info_audio->raw_frame);
        if (info_audio->enc_pkt) av_packet_free(&info_audio->enc_pkt);
        free(info_audio);
    }
    if (info_video) {
        if (info_video->enc_ctx) avcodec_free_context(&info_video->enc_ctx);
        if (info_video->raw_frame) av_frame_free(&info_video->raw_frame);
        if (info_video->enc_pkt) av_packet_free(&info_video->enc_pkt);
        free(info_video);
    }

    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&ofmt_ctx->pb);
    }

    avformat_free_context(ofmt_ctx);

    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "An error occurred: %s\n", av_err2str(ret));
    }

    free(output_filename);

    printf("Decoding completed\n\n");
    return ret ? 1 : 0;
}

void logger(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
