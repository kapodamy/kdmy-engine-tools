#ifndef _arraybuffer_h
#define _arraybuffer_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "malloc_utils.h"


typedef struct ArrayBuffer_s {
    size_t length;
    int32_t references;
    const uint8_t data[];
}* ArrayBuffer;


static inline ArrayBuffer arraybuffer_init(size_t length) {
    ArrayBuffer buffer = malloc_chk(sizeof(struct ArrayBuffer_s) + length);
    buffer->length = length;
    return buffer;
}

static inline void arraybuffer_destroy(ArrayBuffer* arraybuffer) {
    if (*arraybuffer) {
        free_chk(*arraybuffer);
        *arraybuffer = NULL;
    }
}



#endif
