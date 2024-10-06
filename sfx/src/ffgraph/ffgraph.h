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

#include "decoderhandle.h"
#include "sourcehandle.h"

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
void ffgraphfmt_seek(FFGraphFormat* ffgraphfmt, double seconds);
double ffgraphfmt_get_stream_duration(FFGraphFormat* ffgraphfmt);

bool audioconverter_init(const AVCodecContext* codec_ctx, FFGraphConversor* ffgraphconv);
void audioconverter_get_stream_info(FFGraphConversor* ffgraphconv, int* out_channels, int* out_rate);
int32_t audioconverter_read(FFGraphConversor* ffgraphconv, void* samples_out, int32_t max_samples_per_channel);

AVIOContext* iohandler_init(SourceHandle* sourcehandle);
void iohandler_destroy(AVIOContext** iohandle);

void audioconverter_enable_custom_output(bool enable_custom, int rate, bool force_mono, bool pcmu8);

static inline double calculate_seconds(const AVStream* av_stream, const AVFrame* av_frame) {
    int64_t start_pts = av_stream->start_time;
    if (start_pts == AV_NOPTS_VALUE) start_pts = 0;

    double time_base = av_stream->time_base.num / (double)av_stream->time_base.den;
    double seconds = (av_frame->pts - start_pts) * time_base;

    return seconds;
}


typedef struct {
    FFGraphFormat* audio;
} FFGraph;

typedef struct {
    int32_t audio_channels;
    int32_t audio_sample_rate;
    double audio_seconds_duration;
} FFGraphInfo;


extern FFGraph* ffgraph_init(SourceHandle* audio_sourcehandle);
extern void ffgraph_destroy(FFGraph* ffgraph);
extern void ffgraph_init_audioconverter(FFGraph* ffgraph);
extern void ffgraph_get_original_stream_info(FFGraph* ffgraph, FFGraphInfo* output_info);
extern void ffgraph_get_stream_info(FFGraph* ffgraph, FFGraphInfo* output_info);
extern int32_t ffgraph_read_audio_samples(FFGraph* ffgraph, void* out_samples, int32_t max_samples_per_channel);
extern void ffgraph_seek(FFGraph* ffgraph, double time_in_seconds);

extern const char* ffgraph_get_runtime_info();

#endif
