#include <assert.h>

#include "decoderhandle.h"
#include "ffgraph.h"

typedef struct {
    DecoderHandle dechnd;
    FFGraph* ffgraph;
} FFGraphDecoderHandle;



static void stub_destroy(DecoderHandle* decoder) {
    (void)decoder;
}

static bool stub_seek(DecoderHandle* decoder, double timestamp) {
    (void)decoder;
    (void)timestamp;
    return false;
}

static void stub_loop(DecoderHandle* decoder, int64_t* loop_start, int64_t* loop_length) {
    (void)decoder;
    *loop_start = -1;
    *loop_length = -1;
}


static int32_t read(DecoderHandle* decoder, void* buffer, uint32_t sample_per_channel) {
    FFGraph* ffgraph = ((FFGraphDecoderHandle*)decoder)->ffgraph;
    int32_t ret = ffgraph_read_audio_samples(ffgraph, buffer, (int32_t)sample_per_channel);
    return ret;
}

static SampleFormat info(DecoderHandle* decoder, uint32_t* rate, uint32_t* channels, double* duration) {
    FFGraph* ffgraph = ((FFGraphDecoderHandle*)decoder)->ffgraph;
    FFGraphInfo info;
    ffgraph_get_streams_info(ffgraph, &info);

    *rate = (uint32_t)info.audio_sample_rate;
    *channels = (uint32_t)info.audio_channels;
    *duration = info.audio_seconds_duration * 1000.0;

    return SampleFormat_PCM_S16LE;
}

static bool seek(DecoderHandle* decoder, double timestamp) {
    FFGraph* ffgraph = ((FFGraphDecoderHandle*)decoder)->ffgraph;
    ffgraphfmt_seek(ffgraph->audio, timestamp / 1000.0);
    return true;
}

static void destroy(DecoderHandle* decoder) {
    FFGraph* ffgraph = ((FFGraphDecoderHandle*)decoder)->ffgraph;
    ffgraph_destroy(ffgraph);
}


DecoderHandle* ffgraph_sndbridge_create_helper(FFGraph* ffgraph, bool allow_seek, bool allow_destroy) {
    if (!ffgraph->audio) return NULL;

    FFGraphDecoderHandle* ffgraph_sndbridge = malloc(sizeof(FFGraphDecoderHandle));
    assert(ffgraph_sndbridge);

    ffgraph_sndbridge->dechnd.destroy = allow_destroy ? destroy : stub_destroy;
    ffgraph_sndbridge->dechnd.getInfo = info;
    ffgraph_sndbridge->dechnd.read = read;
    ffgraph_sndbridge->dechnd.seek = allow_seek ? seek : stub_seek;
    ffgraph_sndbridge->dechnd.getLoopPoints = stub_loop;
    ffgraph_sndbridge->ffgraph = ffgraph;

    return (DecoderHandle*)ffgraph_sndbridge;
}

void ffgraph_sndbridge_destroy_helper(DecoderHandle* ffgraph_sndbridge) {
    if (!ffgraph_sndbridge) return;
    free(ffgraph_sndbridge);
}
