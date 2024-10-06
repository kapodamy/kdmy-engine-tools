#include <assert.h>

#include "ffgraph.h"

static char INFO_BUFFER[256];

FFGraph* ffgraph_init(SourceHandle* video_sourcehandle, SourceHandle* audio_sourcehandle) {
    if (video_sourcehandle == audio_sourcehandle) {
        printf("ffgraph_init() error: due to design constraints both sourcehandles can not be equal.\n");
        return NULL;
    }

    FFGraphFormat* video = ffgraphfmt_init(video_sourcehandle, AVMEDIA_TYPE_VIDEO);
    if (video) videoconverter_init(video->codec_ctx, &video->ffgraphconv);

    FFGraphFormat* audio = ffgraphfmt_init(audio_sourcehandle, AVMEDIA_TYPE_AUDIO);
    if (audio) audioconverter_init(audio->codec_ctx, &audio->ffgraphconv);

    if (!video && !audio) {
        printf("ffgraph_init() failed, no audio/video stream available.\n");
        return NULL;
    }


    FFGraph* ffgraph = malloc(sizeof(FFGraph));
    assert(ffgraph);
    ffgraph->video = video;
    ffgraph->audio = audio;

    return ffgraph;
}

void ffgraph_destroy(FFGraph* ffgraph) {
    if (ffgraph->audio) ffgraphfmt_destroy(ffgraph->audio);
    if (ffgraph->video) ffgraphfmt_destroy(ffgraph->video);

    free(ffgraph);
}

void ffgraph_get_streams_info(FFGraph* ffgraph, FFGraphInfo* output_info) {
    if (!output_info) return;

    memset(output_info, 0x00, sizeof(FFGraphInfo));

    output_info->audio_has_stream = ffgraph->audio != NULL;
    if (output_info->audio_has_stream) {
        output_info->audio_seconds_duration = ffgraphfmt_get_stream_duration(ffgraph->audio);
        audioconverter_get_stream_info(
            &ffgraph->audio->ffgraphconv, &output_info->audio_channels, &output_info->audio_sample_rate
        );
    }

    output_info->video_has_stream = ffgraph->video != NULL;
    if (output_info->video_has_stream) {
        output_info->video_seconds_duration = ffgraphfmt_get_stream_duration(ffgraph->video);
        videoconverter_get_stream_info(
            &ffgraph->video->ffgraphconv, &output_info->video_width, &output_info->video_height, &output_info->video_fps
        );

        output_info->video_original_width = ffgraph->video->fmt_ctx->streams[ffgraph->video->stream_idx]->codecpar->width;
        output_info->video_original_height = ffgraph->video->fmt_ctx->streams[ffgraph->video->stream_idx]->codecpar->height;
    }
}

int32_t ffgraph_read_audio_samples(FFGraph* ffgraph, void* out_samples, int32_t max_samples_per_channel) {
    if (!ffgraph->audio) {
        return -1;
    }
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

double ffgraph_read_video_frame(FFGraph* ffgraph, AVFrame** out_frame) {
    if (!ffgraph->video || ffgraph->video->has_ended) {
        return -2.0;
    }

    if (!ffgraphfmt_read(ffgraph->video)) {
        return -1.0;
    }

    // do not care, required for non-monotonous timetamp
    const double seconds = -1.0;

    return videoconverter_read(&ffgraph->video->ffgraphconv, seconds, out_frame);
}

void ffgraph_seek(FFGraph* ffgraph, double time_in_seconds) {
    if (ffgraph->video)
        ffgraphfmt_seek(ffgraph->video, time_in_seconds);

    if (ffgraph->audio)
        ffgraphfmt_seek(ffgraph->audio, time_in_seconds);
}

void ffgraph_seek2(FFGraph* ffgraph, double time_in_seconds, bool audio_or_video) {
    FFGraphFormat* ffgraphfmt = audio_or_video ? ffgraph->audio : ffgraph->video;
    if (!ffgraphfmt) return;
    ffgraphfmt_seek(ffgraphfmt, time_in_seconds);
}

const char* ffgraph_get_runtime_info() {
    int version_avformat = avformat_version();
    int version_avcodec = avcodec_version();
    int version_swresample = swresample_version();
    int version_swscale = swscale_version();
    int version_avutil = avutil_version();

    int ret = snprintf(
        INFO_BUFFER, sizeof(INFO_BUFFER),
        "avf=%d.%d.%d avc=%d.%d.%d swr=%d.%d.%d sws=%d.%d.%d avu=%d.%d.%d",
        (version_avformat >> 16) & 0xFFFF,
        (version_avformat >> 8) & 0x00FF,
        version_avformat & 0x00FF,
        (version_avcodec >> 16) & 0xFFFF,
        (version_avcodec >> 8) & 0x00FF,
        version_avcodec & 0x00FF,
        (version_swresample >> 16) & 0xFFFF,
        (version_swresample >> 8) & 0x00FF,
        version_swresample & 0x00FF,
        (version_swscale >> 16) & 0xFFFF,
        (version_swscale >> 8) & 0x00FF,
        version_swscale & 0x00FF,
        (version_avutil >> 16) & 0xFFFF,
        (version_avutil >> 8) & 0x00FF,
        version_avutil & 0x00FF
    );

    return ret < 0 ? NULL : INFO_BUFFER;
}
