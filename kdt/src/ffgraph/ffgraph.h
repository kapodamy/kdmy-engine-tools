#ifndef _ffgraph_decoder_h
#define _ffgraph_decoder_h

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>

#include "sourcehandle.h"

struct BufferNode_t;

typedef struct BufferNode_t {
    uint8_t* data;
    int data_size;
    int64_t seconds;
    struct BufferNode_t* next_node;
} BufferNode;


typedef struct {
    BufferNode* head;
    BufferNode* tail;
} Buffering;

struct FFGraphConversor_t;

typedef bool (*ConverterCallback)(struct FFGraphConversor_t* conv_ctx, const AVCodecContext* codec_ctx, const AVFrame* av_frame);
typedef void (*DestroyCallback)(struct FFGraphConversor_t* conv_ctx);
typedef void (*FlushCallback)(struct FFGraphConversor_t* conv_ctx);
typedef void (*BufferSeekCallback)(struct FFGraphConversor_t* conv_ctx, double seconds);

typedef struct FFGraphConversor_t {
    ConverterCallback convert_cb;
    DestroyCallback destroy_cb;
    FlushCallback flush_cb;
    BufferSeekCallback bufferseek_cb;
    void* priv_data;
    const AVStream* av_stream;
} FFGraphConversor;


typedef struct {
    AVFormatContext* fmt_ctx;
    AVPacket* packet;

    FFGraphConversor ffgraphconv;

    AVIOContext* iohandle;

    AVCodecContext* codec_ctx;
    const AVCodec* av_codec;
    AVFrame* av_frame;
    int stream_idx;
    bool has_ended;
} FFGraphFormat;


FFGraphFormat* ffgraphfmt_init(SourceHandle* sourcehandle, const enum AVMediaType required_type);
void ffgraphfmt_destroy(FFGraphFormat* ffgraphdec);
bool ffgraphfmt_read(FFGraphFormat* ffgraphfmt);
double ffgraphfmt_get_stream_duration(FFGraphFormat* ffgraphfmt);

bool videoconverter_init(const AVCodecContext* codec_ctx, FFGraphConversor* conv_ctx);
void videoconverter_get_stream_info(FFGraphConversor* ffgraphconv, int* out_width, int* out_height, int* out_fps);
double videoconverter_read(FFGraphConversor* ffgraphconv, double seconds, void** frame_out, int32_t* frame_size_out);

AVIOContext* iohandler_init(SourceHandle* sourcehandle);
void iohandler_destroy(AVIOContext** iohandle);

/*

////////////
// UNUSED //
////////////
Buffering* buffering_init();
void buffering_destroy(Buffering** buffering);
void buffering_add(Buffering* buffering, void* data, int data_size, int64_t seconds);
bool buffering_get(Buffering* buffering, void** data, int* data_size, int64_t* seconds);
void buffering_free_data(void* data);
void buffering_clear(Buffering* buffering);
*/

static inline double calculate_seconds(const AVStream* av_stream, const AVFrame* av_frame) {
    int64_t start_pts = av_stream->start_time;
    if (start_pts == AV_NOPTS_VALUE) start_pts = 0;

    double time_base = av_stream->time_base.num / (double)av_stream->time_base.den;
    double seconds = (av_frame->pts - start_pts) * time_base;

    return seconds;
}


typedef struct {
    FFGraphFormat* video;
} FFGraph;

typedef struct {
    int32_t video_has_stream;

    int32_t video_width;
    int32_t video_height;
    int32_t video_fps;

    double video_seconds_duration;
} FFGraphInfo;


extern FFGraph* ffgraph_init(SourceHandle* video_sourcehandle);
extern void ffgraph_destroy(FFGraph* ffgraph);
extern void ffgraph_get_streams_info(FFGraph* ffgraph, FFGraphInfo* output_info);
extern double ffgraph_read_video_frame(FFGraph* ffgraph, void** out_frame, int32_t* out_frame_size);
extern double ffgraph_read_video_frame2(FFGraph* ffgraph, uint8_t* buffer, int32_t buffer_size);
extern void ffgraph_seek(FFGraph* ffgraph, double time_in_seconds);
extern void ffgraph_seek2(FFGraph* ffgraph, double time_in_seconds, bool audio_or_video);

extern const char* ffgraph_get_runtime_info();

typedef void (*DimmensFn)(int32_t src_width, int32_t src_height, int32_t* dst_width, int32_t* dst_height);
void videoconverter_set_custom(bool enable, enum AVPixelFormat pixel_format, int scaling_algorithm, DimmensFn dimmen_setter, const char* dither);

#endif
