#include <assert.h>

#include "ffgraph.h"

static char INFO_BUFFER[256];

FFGraph* ffgraph_init(SourceHandle* video_sourcehandle) {
    FFGraphFormat* video = ffgraphfmt_init(video_sourcehandle, AVMEDIA_TYPE_VIDEO);
    if (video) videoconverter_init(video->codec_ctx, &video->ffgraphconv);

    if (!video) {
        printf("ffgraph_init() failed, no audio/video stream available.\n");
        return NULL;
    }


    FFGraph* ffgraph = malloc(sizeof(FFGraph));
    assert(ffgraph);
    ffgraph->video = video;

    return ffgraph;
}

void ffgraph_destroy(FFGraph* ffgraph) {
    if (ffgraph->video) ffgraphfmt_destroy(ffgraph->video);

    free(ffgraph);
}

void ffgraph_get_streams_info(FFGraph* ffgraph, FFGraphInfo* output_info) {
    if (!output_info) return;

    memset(output_info, 0x00, sizeof(FFGraphInfo));

    output_info->video_has_stream = ffgraph->video != NULL;
    if (output_info->video_has_stream) {
        output_info->video_seconds_duration = ffgraphfmt_get_stream_duration(ffgraph->video);
        videoconverter_get_stream_info(
            &ffgraph->video->ffgraphconv, &output_info->video_width, &output_info->video_height, &output_info->video_fps
        );
    }
}

double ffgraph_read_video_frame(FFGraph* ffgraph, void** out_frame, int32_t* out_frame_size) {
    if (!ffgraph->video || ffgraph->video->has_ended) {
        return -2.0;
    }

    if (!ffgraphfmt_read(ffgraph->video)) {
        return -1.0;
    }

    // do not care, required for non-monotonous timetamp
    const double seconds = -1.0;

    return videoconverter_read(&ffgraph->video->ffgraphconv, seconds, out_frame, out_frame_size);
}

double ffgraph_read_video_frame2(FFGraph* ffgraph, uint8_t* buffer, int32_t buffer_size) {
    void* frame;
    int32_t frame_size;

    double ret = ffgraph_read_video_frame(ffgraph, &frame, &frame_size);
    if (ret < 0) return ret;

    int32_t to_copy = frame_size < buffer_size ? frame_size : buffer_size;
    memcpy(buffer, frame, to_copy);

    return ret;
}

const char* ffgraph_get_runtime_info() {
    int version_avformat = avformat_version();
    int version_avcodec = avcodec_version();
    int version_swscale = swscale_version();
    int version_avutil = avutil_version();

    int ret = snprintf(
        INFO_BUFFER, sizeof(INFO_BUFFER),
        "avf=%d.%d.%d avc=%d.%d.%d sws=%d.%d.%d avu=%d.%d.%d",
        (version_avformat >> 16) & 0xFFFF,
        (version_avformat >> 8) & 0x00FF,
        version_avformat & 0x00FF,
        (version_avcodec >> 16) & 0xFFFF,
        (version_avcodec >> 8) & 0x00FF,
        version_avcodec & 0x00FF,
        (version_swscale >> 16) & 0xFFFF,
        (version_swscale >> 8) & 0x00FF,
        version_swscale & 0x00FF,
        (version_avutil >> 16) & 0xFFFF,
        (version_avutil >> 8) & 0x00FF,
        version_avutil & 0x00FF
    );

    return ret < 0 ? NULL : INFO_BUFFER;
}
