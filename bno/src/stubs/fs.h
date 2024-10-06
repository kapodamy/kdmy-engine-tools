#ifndef _fs_h
#define _fs_h

#include <assert.h>
#include <malloc.h>
#include <stdio.h>

#include "arraybuffer.h"

//
//  all functions here are stub
//

static inline ArrayBuffer fs_readarraybuffer(const char* src) {
    FILE* file = fopen(src, "rb");
    fseek(file, 0, SEEK_END);
    long l = ftell(file);
    fseek(file, 0, SEEK_SET);

    ArrayBuffer buffer = malloc(sizeof(struct ArrayBuffer_s) + (size_t)l);
    assert(buffer);

    fread((void*)buffer->data, sizeof(char), (size_t)l, file);

    buffer->length = (size_t)l;
    return buffer;
}


#endif