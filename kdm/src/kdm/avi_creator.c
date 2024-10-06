#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include "avi_creator.h"
#include "twopass_buffer.h"

typedef struct {
    AVCodecContext* enc_ctx;
    AVPacket* enc_pkt;
    int stream_index;
    int width;
    int height;
    int bitrate_kbps;
    double fps;
    TwoPassLogBuffer* log;
} StreamInfo;


static char filename_suffix[] = "___temp.avi";


static int encode(AVFormatContext* ofmt_ctx, StreamInfo* info, AVFrame* frame) {
    AVPacket* enc_pkt = info->enc_pkt;

    av_packet_unref(enc_pkt);

    if (frame && frame->pts != AV_NOPTS_VALUE) {
        frame->pts = av_rescale_q(frame->pts, frame->time_base, info->enc_ctx->time_base);
    }

    int ret = avcodec_send_frame(info->enc_ctx, frame);
    if (ret < 0) {
        return ret;
    }

    if ((info->enc_ctx->flags & AV_CODEC_FLAG_PASS1) != 0) {
        twopass_log_append(info->log, info->enc_ctx->stats_out);
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
        return encode(ofmt_ctx, info, NULL);
    else
        return 0;
}

static int create_output_stream(AVFormatContext* ofmt_ctx, StreamInfo* info, bool pass2) {
    AVStream* out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if (!out_stream) {
        av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream #%i\n", ofmt_ctx->nb_streams);
        return AVERROR_UNKNOWN;
    }

    const AVCodec* encoder = avcodec_find_encoder_by_name("libxvid");
    if (!encoder) {
        av_log(NULL, AV_LOG_FATAL, "Encoder not found: libxvid\n");
        return AVERROR_INVALIDDATA;
    }

    AVCodecContext* enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx) {
        av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context for %s\n", encoder->name);
        return AVERROR(ENOMEM);
    }

    enc_ctx->width = info->width;
    enc_ctx->height = info->height;
    enc_ctx->time_base = av_d2q(1.0 / info->fps, INT_MAX);
    enc_ctx->framerate = av_d2q(info->fps, INT_MAX);
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    enc_ctx->bit_rate = info->bitrate_kbps * 1024;
    enc_ctx->rc_min_rate = info->bitrate_kbps * 1024;
    enc_ctx->rc_max_rate = info->bitrate_kbps * 1024;
    enc_ctx->rc_buffer_size = info->bitrate_kbps * 1024;
    enc_ctx->mb_decision = FF_MB_DECISION_RD;
    enc_ctx->flags |= AV_CODEC_FLAG_4MV | AV_CODEC_FLAG_AC_PRED;
    enc_ctx->trellis = 2;
    enc_ctx->me_cmp = 2;
    enc_ctx->me_sub_cmp = 2;
    enc_ctx->codec_tag = MKTAG('x', 'v', 'i', 'd');

    if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (pass2) {
        enc_ctx->flags |= AV_CODEC_FLAG_PASS2;
        enc_ctx->stats_in = info->log->buffer;
    } else {
        enc_ctx->flags |= AV_CODEC_FLAG_PASS1;
    }

    int ret = avcodec_open2(enc_ctx, encoder, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to open %s encoder for stream #%i\n", encoder->name, info->stream_index);
        return ret;
    }

    if (!pass2) {
        twopass_log_append(info->log, enc_ctx->stats_out);
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

static int init_output_file(const char* filename, AVFormatContext** ofmt, StreamInfo* video, bool pass2) {
    int ret;
    AVFormatContext* ofmt_ctx = NULL;

    const AVOutputFormat* fmt = av_guess_format("avi", NULL, "video/x-msvideo");
    assert(fmt);

    avformat_alloc_output_context2(&ofmt_ctx, fmt, NULL, filename);
    if (!ofmt_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Failed to create output context for %s\n", filename);
        return AVERROR_UNKNOWN;
    }

    ret = create_output_stream(ofmt_ctx, video, pass2);
    if (ret != 0) {
        return ret;
    }

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


char* avicreator_process_video(FFGraph* ffgraph, CB_Progress progress, void* userdata, const char* filename_to_suffix, int bitrate_kbps, bool volatile* has_int_signal) {
    size_t filename_to_suffix_length = strlen(filename_to_suffix);
    size_t filename_suffix_length = strlen(filename_suffix) + 1;

    char* filename = malloc(filename_to_suffix_length + filename_suffix_length);
    assert(filename);
    memcpy(filename, filename_to_suffix, filename_to_suffix_length);
    memcpy(filename + filename_to_suffix_length, filename_suffix, filename_suffix_length);

    AVFormatContext* ofmt_ctx;

    FFGraphInfo ffgraph_info;
    ffgraph_get_streams_info(ffgraph, &ffgraph_info);

    StreamInfo info = {
        .width = ffgraph_info.video_width,
        .height = ffgraph_info.video_height,
        .bitrate_kbps = bitrate_kbps,
        .fps = ffgraph_info.video_fps,
        .log = twopass_log_init(),
        .enc_pkt = av_packet_alloc()
    };

    assert(info.enc_pkt);

    av_log(NULL, AV_LOG_INFO, "Preparing pass 1...\n");

    int ret = init_output_file(filename, &ofmt_ctx, &info, false);
    if (ret != 0) {
        goto L_return;
    }

    AVFrame* frame;
    double time;
    while ((time = ffgraph_read_video_frame(ffgraph, &frame)) >= 0.0) {
        if (has_int_signal && *has_int_signal) {
            ret = -1;
            goto L_return;
        }

        if (!frame) break;

        ret = encode(ofmt_ctx, &info, frame);
        progress(userdata, time);

        if (ret != 0) {
            goto L_return;
        }
    }

    ret = flush_encoder(ofmt_ctx, &info);
    if (ret != 0) {
        goto L_return;
    }

    // prepare second pass
    progress(userdata, -1.0);
    av_log(NULL, AV_LOG_INFO, "Preparing pass 2...\n");
    ffgraph_seek(ffgraph, 0.0);
    av_write_trailer(ofmt_ctx);
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);
    avcodec_free_context(&info.enc_ctx);

    ret = init_output_file(filename, &ofmt_ctx, &info, true);
    if (ret != 0) {
        goto L_return;
    }

    while ((time = ffgraph_read_video_frame(ffgraph, &frame)) >= 0.0) {
        if (has_int_signal && *has_int_signal) {
            ret = -1;
            goto L_return;
        }

        if (!frame) break;

        ret = encode(ofmt_ctx, &info, frame);
        progress(userdata, time);

        if (ret != 0) {
            goto L_return;
        }
    }

    ret = flush_encoder(ofmt_ctx, &info);
    if (ret != 0) {
        goto L_return;
    }

    ffgraph_seek(ffgraph, 0.0);

    printf("\n");
    av_dump_format(ofmt_ctx, 0, filename, 1);
    av_log(NULL, AV_LOG_INFO, "Two-pass encoding success\n");

L_return:
    twopass_log_destroy(info.log);
    if (ret == 0) av_write_trailer(ofmt_ctx);
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);
    avcodec_free_context(&info.enc_ctx);
    av_packet_free(&info.enc_pkt);

    if (has_int_signal && *has_int_signal) {
        av_log(NULL, AV_LOG_ERROR, "two-pass encoding canceled\n");
        unlink(filename);
        free(filename);
        return NULL;
    }

    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to encode frame, reason: %s\n", av_err2str(ret));
    }

    return ret == 0 ? filename : NULL;
}
