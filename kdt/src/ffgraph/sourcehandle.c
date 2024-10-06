#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sourcehandle.h"

#ifdef SNDBRIDGE_DREAMCAST
#define MAX_SIZE_IN_MEMORY (1 * 1024 * 1024) // 1MiB
#else
#define MAX_SIZE_IN_MEMORY (128 * 1024 * 1024) // 128MiB
#endif



_Noreturn static void assert_with_msg(const char* reason_message, const char* error_message) {
    printf("[e] filehandle: %s %s\n", reason_message, error_message);
    abort();
}

static int32_t file_length(FILE* file) {
    long offset = ftell(file);
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, offset, SEEK_SET);

    assert(length < INT32_MAX);

    return length;
}


typedef struct {
    SourceHandle handle;
    FILE* file;
    bool allow_dispose;
    int64_t file_length;
} FileHandle;
static int32_t FileHandle_Read(SourceHandle* handle, void* buffer, int32_t buffer_size) {
    FileHandle* filehandle = (FileHandle*)handle;
    if (buffer_size < 0) return -1;

    size_t ret = fread(buffer, 1, (size_t)buffer_size, filehandle->file);
    if (!ret && ferror(filehandle->file)) return -1;

    return (int32_t)ret;
}
static int32_t FileHandle_Seek(SourceHandle* handle, int64_t offset, int whence) {
    FileHandle* filehandle = (FileHandle*)handle;
    return fseek(filehandle->file, offset, whence);
}
static int64_t FileHandle_Tell(SourceHandle* handle) {
    FileHandle* filehandle = (FileHandle*)handle;
    return ftell(filehandle->file);
}
static int64_t FileHandle_Length(SourceHandle* handle) {
    FileHandle* filehandle = (FileHandle*)handle;
    return filehandle->file_length;
}
static void FileHandle_Destroy(SourceHandle* handle) {
    FileHandle* filehandle = (FileHandle*)handle;
    if (!filehandle->file) return;
    if (filehandle->allow_dispose) fclose(filehandle->file);
    filehandle->file = NULL;
}

typedef struct {
    SourceHandle handle;
    int32_t size;
    int32_t offset;
    uint8_t* data;
    bool allow_dispose;
} BufferHandle;
static int32_t BufferHandle_Read(SourceHandle* handle, void* buffer, int32_t buffer_size) {
    BufferHandle* bufferhandle = (BufferHandle*)handle;
    if (buffer_size < 0) return -1;
    if (bufferhandle->offset >= bufferhandle->size) return 0;

    int32_t end = bufferhandle->offset + buffer_size;
    if (end > bufferhandle->size) buffer_size = (int32_t)(bufferhandle->size - bufferhandle->offset);

    memcpy(buffer, bufferhandle->data + bufferhandle->offset, (size_t)buffer_size);
    bufferhandle->offset += buffer_size;

    return buffer_size;
}
static int32_t BufferHandle_Seek(SourceHandle* handle, int64_t offset, int whence) {
    BufferHandle* bufferhandle = (BufferHandle*)handle;
    int64_t newoffset;

    switch (whence) {
        case SEEK_SET:
            newoffset = offset;
            break;
        case SEEK_CUR:
            newoffset = bufferhandle->offset + offset;
            break;
        case SEEK_END:
            newoffset = bufferhandle->size - offset;
            break;
        default:
            return EINVAL;
    }

    if (newoffset < 0 || newoffset > bufferhandle->size) return ESPIPE;
    bufferhandle->offset = newoffset;
    return 0;
}
static int64_t BufferHandle_Tell(SourceHandle* handle) {
    BufferHandle* bufferhandle = (BufferHandle*)handle;
    return bufferhandle->offset;
}
static int64_t BufferHandle_Length(SourceHandle* handle) {
    BufferHandle* bufferhandle = (BufferHandle*)handle;
    return bufferhandle->size;
}
static void BufferHandle_Destroy(SourceHandle* handle) {
    BufferHandle* bufferhandle = (BufferHandle*)handle;
    if (!bufferhandle->data) return;
    if (bufferhandle->allow_dispose) free(bufferhandle->data);

    bufferhandle->data = NULL;
    bufferhandle->size = bufferhandle->offset = -1;
}


static SourceHandle* from_buffer(void* data, int32_t size, bool allow_dispose) {
    BufferHandle* obj = malloc(sizeof(BufferHandle));
    if (!obj) {
        return NULL;
    }

    obj->data = (uint8_t*)data;
    obj->size = size;
    obj->offset = 0;
    obj->allow_dispose = allow_dispose;
    obj->handle.read = BufferHandle_Read;
    obj->handle.seek = BufferHandle_Seek;
    obj->handle.tell = BufferHandle_Tell;
    obj->handle.length = BufferHandle_Length;
    obj->handle.destroy = BufferHandle_Destroy;

    return (SourceHandle*)obj;
}

static SourceHandle* from_file(FILE* file, int32_t file_length, bool allow_dispose) {
    FileHandle* obj = malloc(sizeof(FileHandle));
    if (!obj) {
        return NULL;
    }

    obj->file = file;
    obj->allow_dispose = allow_dispose;
    obj->file_length = file_length;
    obj->handle.read = FileHandle_Read;
    obj->handle.seek = FileHandle_Seek;
    obj->handle.tell = FileHandle_Tell;
    obj->handle.length = FileHandle_Length;
    obj->handle.destroy = FileHandle_Destroy;

    return (SourceHandle*)obj;
}


SourceHandle* filehandle_init1(const char* fullpath, bool try_load_in_ram) {
#ifdef SNDBRIDGE_DREAMCAST
    char fullpath2[256];
    size_t str_len = strlen(fullpath);
    strncpy(fullpath2 + 1, fullpath + strlen("/cd"), 255 < str_len ? 255 : str_len);
    
    fullpath2[0] = '.';
    FILE* file = fopen(fullpath2, "rb");
#else
    FILE* file = fopen(fullpath, "rb");
#endif

    if (!file) {
        printf("[e] filehandle_init1: failed to open %s\n", fullpath);
        return NULL;
    }

    int32_t length = file_length(file);
    if (!file || length < 1) {
        printf("[e] filehandle_init1: failed to get the length of %s\n", fullpath);
        return NULL;
    }

    if (length > MAX_SIZE_IN_MEMORY || !try_load_in_ram) {
        return from_file(file, length, true);
    }

    void* buffer = malloc((size_t)length);
    if (!buffer) {
        printf("[!] filehandle_init1: failed to buffer the whole contents of %s\n", fullpath);
        return from_file(file, length, true);
    }

    size_t readed = fread(buffer, 1, (size_t)length, file);
    if (!readed) {
        fclose(file);
        return NULL;
    }
    if (readed < (size_t)length) {
        buffer = realloc(buffer, readed);
    }

    fclose(file);

    return from_buffer(buffer, (int32_t)readed, true);
}

SourceHandle* filehandle_init2(void* buffer, int32_t size) {
    if (!buffer) return NULL;

    return from_buffer(buffer, size, false);
}

SourceHandle* filehandle_init3(void* buffer, int32_t buffer_length, int32_t offset, int32_t size) {
    if (!buffer) return NULL;
    if (offset >= buffer_length || offset > size || offset < 0) assert_with_msg("filehandle_init3 out-of-range", "offset");
    if (size > buffer_length || size < offset || size < 0) assert_with_msg("filehandle_init3 out-of-range", "size");

    return from_buffer(buffer, offset, size);
}
