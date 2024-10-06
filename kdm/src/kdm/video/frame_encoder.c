#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include "../../logger.h"
#include "../kdm.h"
#include "../kdm_internal.h"
#include "../twopass_buffer.h"


struct _KDM_Video {
    int32_t width;
    int32_t height;
    double fps;

    AVCodecContext* enc_ctx;
    AVPacket* enc_pkt;
    bool force_first_frame_as_keyframe;
    TwoPassLogBuffer* pass2_log;
};


static int create_output_stream(KDM_Video* videoenc, int bitrate_kbps, int gop, TwoPassLogBuffer* pass2_log, bool pass2) {
    const AVCodec* encoder = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
    if (!encoder) {
        av_log(NULL, AV_LOG_FATAL, "Encoder not found:%s\n", avcodec_get_name(AV_CODEC_ID_MPEG1VIDEO));
        return AVERROR_INVALIDDATA;
    }

    AVCodecContext* enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx) {
        av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context for %s\n", encoder->name);
        return AVERROR(ENOMEM);
    }

    enc_ctx->width = videoenc->width;
    enc_ctx->height = videoenc->height;
    enc_ctx->time_base = av_d2q(1.0 / videoenc->fps, INT_MAX);
    enc_ctx->framerate = av_d2q(videoenc->fps, INT_MAX);
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (bitrate_kbps == -2) {
        // high quality
        enc_ctx->gop_size = gop < 1 ? 15 : gop;
        enc_ctx->bit_rate = 2040 * 1024;
#ifdef ALLOW_VBR_VIDEO
        enc_ctx->rc_min_rate = 1150 * 1024;
        enc_ctx->rc_max_rate = 4800 * 1024;
        enc_ctx->rc_buffer_size = 576860;
#endif
    } else if (bitrate_kbps == -1) {
        // low quality
        enc_ctx->gop_size = gop < 1 ? 18 : gop;
        enc_ctx->bit_rate = 1150 * 1024;
#ifdef ALLOW_VBR_VIDEO
        enc_ctx->rc_min_rate = 1150 * 1024;
        enc_ctx->rc_max_rate = 2040 * 1024;
        enc_ctx->rc_buffer_size = 327680;
#endif
    } else {
        // custom quality
        if (gop > 0) enc_ctx->gop_size = gop;
        enc_ctx->bit_rate = bitrate_kbps * 1024;
    }

    if (pass2) {
        enc_ctx->flags |= AV_CODEC_FLAG_PASS2;
        enc_ctx->stats_in = pass2_log->buffer;
    } else {
        enc_ctx->flags |= AV_CODEC_FLAG_PASS1;
    }

    int ret = avcodec_open2(enc_ctx, encoder, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to open %s encoder for output video stream\n", encoder->name);
        return ret;
    }

    if (!pass2) {
        twopass_log_append(pass2_log, enc_ctx->stats_out);
    }

    videoenc->enc_ctx = enc_ctx;

    return 0;
}

static int encode_frame(KDM_Video* videoenc, AVFrame* raw_frame) {
    AVPacket* enc_pkt = videoenc->enc_pkt;
    AVCodecContext* enc_ctx = videoenc->enc_ctx;

    av_packet_unref(enc_pkt);

    if (raw_frame && raw_frame->pts != AV_NOPTS_VALUE) {
        raw_frame->pts = av_rescale_q(raw_frame->pts, raw_frame->time_base, enc_ctx->time_base);
    }

    int ret = avcodec_send_frame(enc_ctx, raw_frame);
    if (ret < 0) {
        return ret;
    }

    if ((enc_ctx->flags & AV_CODEC_FLAG_PASS1) != 0) {
        twopass_log_append(videoenc->pass2_log, enc_ctx->stats_out);
    }

    return avcodec_receive_packet(enc_ctx, enc_pkt);
}

static int flush_encoder(KDM_Video* videoenc) {
    if (videoenc->enc_ctx->codec->capabilities & AV_CODEC_CAP_DELAY)
        return encode_frame(videoenc, NULL);
    else
        return 0;
}

static double calculate_seconds(KDM_Video* videoenc) {
    double time_base = videoenc->enc_ctx->time_base.num / (double)videoenc->enc_ctx->time_base.den;
    double seconds = videoenc->enc_pkt->pts * time_base;

    return seconds;
}


KDM_Video* kdm_videoenc_init(int32_t width, int32_t height, int bitrate_kbps, int gop, double target_fps, TwoPassLogBuffer* pass2_log, bool pass2) {
    if (width < 1 || width > KDM_MAX_TEXTURE_DIMMEN || height < 1 || height > KDM_MAX_TEXTURE_DIMMEN) {
        logger(
            "kdm_videoenc_init() failed, the resolution %ix%i can not be bigger than %ix%i\n",
            width, height,
            KDM_MAX_TEXTURE_DIMMEN, KDM_MAX_TEXTURE_DIMMEN
        );
        return NULL;
    }

    AVPacket* enc_pkt = av_packet_alloc();
    assert(enc_pkt);

    KDM_Video* videoenc = malloc(sizeof(KDM_Video));
    *videoenc = (KDM_Video){
        .width = width,
        .height = height,
        .fps = target_fps,

        .enc_ctx = NULL,
        .enc_pkt = enc_pkt,
        .force_first_frame_as_keyframe = true,
        .pass2_log = pass2_log
    };

    int ret = create_output_stream(videoenc, bitrate_kbps, gop, pass2_log, pass2);
    if (ret != 0) {
        printf("kdm_videoenc_init() failed: %s\n", av_err2str(ret));
        kdm_videoenc_destroy(videoenc);
        videoenc = NULL;
    }

    return videoenc;
}

void kdm_videoenc_destroy(KDM_Video* videoenc) {
    if (videoenc->enc_ctx && videoenc->enc_ctx->stats_in)
        videoenc->enc_ctx->stats_in = NULL;

    av_packet_free(&videoenc->enc_pkt);
    avcodec_free_context(&videoenc->enc_ctx);
    free(videoenc);
}


KDM_PacketResult kdm_videoenc_encode_frame(KDM_Video* videoenc, FILE* mediafile, AVFrame* image) {
    int ret;
    if (image) {
        ret = encode_frame(videoenc, image);
    } else {
        ret = flush_encoder(videoenc);
    }

    if (ret == AVERROR(EAGAIN)) {
        return (KDM_PacketResult){
            .is_keyframe = false,
            .write_success = true,
            .needs_more_data = true,
            .written_bytes = 0,
            .pts = 0.0
        };
    } else if (ret == AVERROR_EOF) {
        return (KDM_PacketResult){
            .is_keyframe = false,
            .write_success = true,
            .needs_more_data = false,
            .written_bytes = 0
        };
    } else if (ret != 0) {
        printf("kdm_videoenc_encode_frame() failed: %s\n", av_err2str(ret));
        return (KDM_PacketResult){
            .is_keyframe = false,
            .write_success = false,
            .needs_more_data = false,
            .written_bytes = 0,
            .pts = 0.0
        };
    }

    assert(videoenc->enc_pkt->size >= 0);

    bool is_keyframe = (videoenc->enc_pkt->flags & AV_PKT_FLAG_KEY) != 0;
    /*if (!is_keyframe && videoenc->force_first_frame_as_keyframe) {
        videoenc->force_first_frame_as_keyframe = false;
        is_keyframe = true;
    }*/

    // write frame
    size_t data_size = (size_t)videoenc->enc_pkt->size;
    bool write_success;

    if (mediafile)
        write_success = fwrite(videoenc->enc_pkt->data, data_size, 1, mediafile) == 1;
    else
        write_success = true;

    return (KDM_PacketResult){
        .written_bytes = data_size,
        .write_success = write_success,
        .needs_more_data = data_size == 0,
        .is_keyframe = is_keyframe,
        .pts = calculate_seconds(videoenc)
    };
}

KDM_PacketResult kdm_videoenc_flush(KDM_Video* videoenc, FILE* mediafile) {
    return kdm_videoenc_encode_frame(videoenc, mediafile, NULL);
}
