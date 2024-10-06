#include <assert.h>
#include <string.h>

#include <libavutil/opt.h>

#include "ffgraph.h"


static const enum AVPixelFormat DEFAULT_PIX_FMT = AV_PIX_FMT_RGBA;

static bool custom_enable = false;
static enum AVPixelFormat custom_pixel_format = AV_PIX_FMT_YUYV422;
static int custom_scaling_algorithm = SWS_LANCZOS;
static DimmensFn custom_dimmen_setter = NULL;
static const char* custom_dither = NULL;


typedef struct {
    struct SwsContext* sws_ctx;
    AVFrame* frame;
    uint8_t* buffer;
    int32_t buffer_size;
    int width;
    int height;
    double seconds;
    double frame_duration;
} VideoConverter;


static bool video_process(FFGraphConversor* ffgraphconv, const AVCodecContext* codec_ctx, const AVFrame* av_frame) {
    if (!av_frame) return false;

    VideoConverter* obj = ffgraphconv->priv_data;
    const uint8_t* const* src_data = (const uint8_t* const*)av_frame->data;

    sws_scale(
        obj->sws_ctx,
        src_data, av_frame->linesize,
        0, codec_ctx->height,
        obj->frame->data, obj->frame->linesize
    );

    obj->seconds = calculate_seconds(ffgraphconv->av_stream, av_frame);
    obj->frame_duration = av_frame->duration;

    return false;
}

static void video_destroy(VideoConverter* conv_ctx) {
    sws_freeContext(conv_ctx->sws_ctx);
    if (conv_ctx->buffer) av_free(conv_ctx->buffer);
    if (conv_ctx->frame) av_frame_free(&conv_ctx->frame);
    av_free(conv_ctx);
}

static void video_destroy_from_conversor(FFGraphConversor* ffgraphconv) {
    if (!ffgraphconv || !ffgraphconv->priv_data) return;

    video_destroy(ffgraphconv->priv_data);
    ffgraphconv->priv_data = NULL;
}


bool videoconverter_init(const AVCodecContext* codec_ctx, FFGraphConversor* ffgraphconv) {
    assert(codec_ctx);

    int width = codec_ctx->width;
    int height = codec_ctx->height;
    int sws_flags = 0x00;
    enum AVPixelFormat pix_fmt = DEFAULT_PIX_FMT;
    const char* dither = NULL;

    if (custom_enable) {
        sws_flags = custom_scaling_algorithm;
        pix_fmt = custom_pixel_format;
        if (custom_dimmen_setter) custom_dimmen_setter(width, height, &width, &height);
        dither = custom_dither;
    }


    int size = av_image_get_buffer_size(pix_fmt, width, height, 1);
    if (size < 0) {
        printf("videoconverter_init() call to av_image_get_buffer_size() failed.\n");
        return false;
    }
    uint8_t* buffer = av_malloc(size);
    if (!buffer) {
        printf("videoconverter_init() failed to allocate the buffer.\n");
        return false;
    }

    VideoConverter* obj = av_malloc(sizeof(VideoConverter));
    obj->frame = NULL;
    obj->sws_ctx = NULL;
    obj->buffer = buffer;
    obj->buffer_size = size;
    obj->width = width;
    obj->height = height;
    obj->seconds = -1.0;
    obj->frame_duration = codec_ctx->framerate.den / (double)codec_ctx->framerate.num;

    if (codec_ctx->framerate.num == 0 && codec_ctx->framerate.den == 1) obj->frame_duration = 0.0;

    bool is_jpeg = codec_ctx->pix_fmt == AV_PIX_FMT_YUVJ420P;

L_initialize_sws:
    obj->sws_ctx = sws_getContext(
        codec_ctx->width, codec_ctx->height, is_jpeg ? AV_PIX_FMT_YUV420P : codec_ctx->pix_fmt,
        width, height, pix_fmt,
        sws_flags | SWS_ACCURATE_RND,
        NULL, NULL, NULL
    );
    if (!obj->sws_ctx) {
        printf("videoconverter_init() call to sws_getContext() failed.\n");
        goto L_failed;
    }

    // avoid warning "[swscaler] deprecated pixel format used, make sure you did set range correctly"
    if (is_jpeg) {
        int* inv_table;
        int* table;
        int srcRange, dstRange, brightness, contrast, saturation;

        int ret = sws_getColorspaceDetails(
            obj->sws_ctx, &inv_table, &srcRange, &table, &dstRange, &brightness, &contrast, &saturation
        );
        if (ret < 0) {
            is_jpeg = false;
            goto L_initialize_sws;
        }

        // set propert JPEG range
        srcRange |= 1;

        ret = sws_setColorspaceDetails(
            obj->sws_ctx, inv_table, srcRange, table, dstRange, brightness, contrast, saturation
        );
        if (ret < 0) {
            is_jpeg = false;
            goto L_initialize_sws;
        }
    }

    if (dither != NULL) {
        int ret = av_opt_set(obj->sws_ctx, "sws_dither", dither, 0x00);
        if (ret != 0) {
            printf("videoconverter_init() failed to set \"sws_dither\" value:%s\n", av_err2str(ret));
            goto L_failed;
        }
    }

    obj->frame = av_frame_alloc();
    if (!obj->frame) {
        printf("videoconverter_init() call to av_frame_alloc() failed.\n");
        goto L_failed;
    }

    av_image_fill_arrays(
        obj->frame->data, obj->frame->linesize, buffer, pix_fmt, width, height, 1
    );

    ffgraphconv->priv_data = obj;
    ffgraphconv->convert_cb = video_process;
    ffgraphconv->destroy_cb = video_destroy_from_conversor;
    ffgraphconv->flush_cb = NULL;
    ffgraphconv->bufferseek_cb = NULL;
    return true;

L_failed:
    video_destroy(obj);
    return false;
}

void videoconverter_get_stream_info(FFGraphConversor* ffgraphconv, int* out_width, int* out_height, int* out_fps) {
    VideoConverter* obj = ffgraphconv->priv_data;
    *out_width = obj->width;
    *out_height = obj->height;
    *out_fps = (int32_t)av_q2d(ffgraphconv->av_stream->r_frame_rate);
}

double videoconverter_read(FFGraphConversor* ffgraphconv, double seconds, void** frame_out, int32_t* frame_size_out) {
    VideoConverter* obj = ffgraphconv->priv_data;

    // check if there no frame at all
    if (obj->seconds < 0) return -1.0;

    // check if the buffered frame is future
    if (seconds >= 0 && obj->seconds > 0 && obj->seconds > seconds) {
        return -1.0;
    }

    *frame_out = obj->buffer;
    *frame_size_out = obj->buffer_size;

    return obj->seconds;
}


void videoconverter_set_custom(bool enable, enum AVPixelFormat pixel_format, int scaling_algorithm, DimmensFn dimmen_setter, const char* dither) {
    custom_enable = enable;
    custom_pixel_format = pixel_format;
    custom_scaling_algorithm = scaling_algorithm;
    custom_dimmen_setter = dimmen_setter;
    custom_dither = dither;
}
