#include <assert.h>

#include "ffgraph.h"

static char INFO_BUFFER[256];

FFGraph* ffgraph_init(SourceHandle* audio_sourcehandle) {
    FFGraphFormat* audio = ffgraphfmt_init(audio_sourcehandle, AVMEDIA_TYPE_AUDIO);

    if (!audio) {
        printf("ffgraph_init() failed, no audio stream available.\n");
        return NULL;
    }

    FFGraph* ffgraph = malloc(sizeof(FFGraph));
    assert(ffgraph);

    ffgraph->audio = audio;

    return ffgraph;
}

void ffgraph_destroy(FFGraph* ffgraph) {
    ffgraphfmt_destroy(ffgraph->audio);
    free(ffgraph);
}

void ffgraph_init_audioconverter(FFGraph* ffgraph) {
    audioconverter_init(ffgraph->audio->codec_ctx, &ffgraph->audio->ffgraphconv);
}

void ffgraph_get_original_stream_info(FFGraph* ffgraph, FFGraphInfo* output_info) {
    if (!output_info) return;

    output_info->audio_seconds_duration = ffgraphfmt_get_stream_duration(ffgraph->audio);
    output_info->audio_channels = ffgraph->audio->codec_ctx->ch_layout.nb_channels;
    output_info->audio_sample_rate = ffgraph->audio->codec_ctx->sample_rate;
}

void ffgraph_get_stream_info(FFGraph* ffgraph, FFGraphInfo* output_info) {
    if (!output_info) return;

    output_info->audio_seconds_duration = ffgraphfmt_get_stream_duration(ffgraph->audio);
    audioconverter_get_stream_info(
        &ffgraph->audio->ffgraphconv, &output_info->audio_channels, &output_info->audio_sample_rate
    );
}

int32_t ffgraph_read_audio_samples(FFGraph* ffgraph, void* out_samples, int32_t max_samples_per_channel) {
    if (max_samples_per_channel < 1) {
        return 0;
    }

    int32_t ret = audioconverter_read(&ffgraph->audio->ffgraphconv, out_samples, max_samples_per_channel);

    if (ret > 0) {
        return ret;
    } else if (ret < 1 && ffgraph->audio->has_ended) {
        return -1;
    }

    // ignore return value
    ffgraphfmt_read(ffgraph->audio);

    return audioconverter_read(&ffgraph->audio->ffgraphconv, out_samples, max_samples_per_channel);
}

void ffgraph_seek(FFGraph* ffgraph, double time_in_seconds) {
    ffgraphfmt_seek(ffgraph->audio, time_in_seconds);
}

const char* ffgraph_get_runtime_info() {
    unsigned int version_avformat = avformat_version();
    unsigned int version_avcodec = avcodec_version();
    unsigned int version_swresample = swresample_version();
    unsigned int version_avutil = avutil_version();

    int ret = snprintf(
        INFO_BUFFER, sizeof(INFO_BUFFER),
        "avf=%d.%d.%d avc=%d.%d.%d swr=%d.%d.%d avu=%d.%d.%d",
        (version_avformat >> 16) & 0xFFFF,
        (version_avformat >> 8) & 0x00FF,
        version_avformat & 0x00FF,
        (version_avcodec >> 16) & 0xFFFF,
        (version_avcodec >> 8) & 0x00FF,
        version_avcodec & 0x00FF,
        (version_swresample >> 16) & 0xFFFF,
        (version_swresample >> 8) & 0x00FF,
        version_swresample & 0x00FF,
        (version_avutil >> 16) & 0xFFFF,
        (version_avutil >> 8) & 0x00FF,
        version_avutil & 0x00FF
    );

    return ret < 0 ? NULL : INFO_BUFFER;
}
