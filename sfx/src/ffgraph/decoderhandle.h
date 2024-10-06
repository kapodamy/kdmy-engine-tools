#ifndef __decoderhandle_h
#define __decoderhandle_h

#include <stdbool.h>
#include <stdint.h>

typedef struct DecoderHandle DecoderHandle;


typedef enum {
    SampleFormat_FLOAT32,
    SampleFormat_PCM_S16LE,
    SampleFormat_PCM_U8,
    SampleFormat_ADPCM_4_YAMAHA
} SampleFormat;

struct DecoderHandle {
    int32_t (*read)(DecoderHandle* decoder, void* buffer, uint32_t samples_per_channel);
    SampleFormat (*getInfo)(DecoderHandle* decoder, uint32_t* rate, uint32_t* channels, double* duration_in_seconds);
    bool (*seek)(DecoderHandle* decoder, double seconds);
    void (*getLoopPoints)(DecoderHandle* decoder, int64_t* loop_start, int64_t* loop_length);
    void (*destroy)(DecoderHandle* decoder);
};

#endif
