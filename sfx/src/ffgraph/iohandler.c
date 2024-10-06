#include "ffgraph.h"

#define BUFFER_SIZE (256 * 1024)                                // 256KiB
#define BUFFER_SIZE_BIG_FILES (1024 * 1024 * 4)                 // 4MiB
#define SOURCEHANDLE_MAX_BUFFERED_FILE_LENGTH (128 * 1024 * 1024) // 128MiB


static int iohandler_read(void* opaque, uint8_t* buf, int buf_size) {
    SourceHandle* sourcehandle = opaque;

    int32_t readed = sourcehandle->read(sourcehandle, buf, buf_size);

    if (readed == 0)
        return AVERROR_EOF;
    else if (readed < 0)
        return AVERROR_UNKNOWN;
    else
        return readed;
}

static int64_t iohandler_seek(void* opaque, int64_t offset, int whence) {
    SourceHandle* sourcehandle = opaque;
    if (whence == AVSEEK_SIZE) return sourcehandle->length(sourcehandle);

    whence &= ~AVSEEK_FORCE;

    int32_t ret = sourcehandle->seek(sourcehandle, offset, whence);

    if (ret)
        return AVERROR_UNKNOWN;
    else
        return sourcehandle->tell(sourcehandle);
}


AVIOContext* iohandler_init(SourceHandle* sourcehandle) {
    SourceHandle* src_hnd = (SourceHandle*)sourcehandle;

    // just in case
    sourcehandle->seek(sourcehandle, 0, SEEK_SET);

    int buffer_size;
    if (sourcehandle->length(sourcehandle) > SOURCEHANDLE_MAX_BUFFERED_FILE_LENGTH)
        buffer_size = BUFFER_SIZE_BIG_FILES;
    else
        buffer_size = BUFFER_SIZE;

    uint8_t* buffer = av_malloc((size_t)buffer_size);
    if (!buffer) {
        printf("iohandler_init() Can not allocate the buffer.\n");
        return NULL;
    }

    AVIOContext* avio_ctx = avio_alloc_context(
        buffer, buffer_size, 0, src_hnd, iohandler_read, NULL, iohandler_seek
    );
    if (!avio_ctx) {
        printf("iohandler_init() call to avio_alloc_context() failed.\n");
        return NULL;
    }

    return avio_ctx;
}

void iohandler_destroy(AVIOContext** iohandle) {
    AVIOContext* avio_ctx = *iohandle;

    if (avio_ctx->buffer) av_free(avio_ctx->buffer);
    avio_context_free(iohandle);
}
