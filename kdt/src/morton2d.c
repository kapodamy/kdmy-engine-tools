#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "morton2d.h"

typedef struct {
    uint32_t x_mask, y_mask;
    uint32_t x_inc, y_inc;
} Morton2D;


#define M2MakeMask(width) ((width != 0) ? (0xffffffffu >> (32 - (width))) : 0)
#define M2DIncX(m2d, v) (((v) + (m2d).x_inc) & (m2d).x_mask)
#define M2DIncY(m2d, v) (((v) + (m2d).y_inc) & (m2d).y_mask)


static Morton2D Morton2DInit(uint32_t x_bits, uint32_t y_bits) {
    Morton2D m2d;

    uint32_t shared = y_bits < x_bits ? y_bits : x_bits;
    uint32_t x_extra = x_bits - shared;
    uint32_t y_extra = y_bits - shared;

    shared = (shared - 1) * 2;
    m2d.x_mask = 0xAAAAAAAA & M2MakeMask(shared);
    m2d.x_mask |= (M2MakeMask(x_extra) << (shared));
    m2d.x_inc = 0x2 | ~m2d.x_mask;

    m2d.y_mask = 0x55555555 & M2MakeMask(shared);
    m2d.y_mask |= (M2MakeMask(y_extra) << shared);
    m2d.y_inc = 0x1 | ~m2d.y_mask;

    return m2d;
}


void MakeTwiddled16(void* src_pix, void* dst_pix, int32_t w, int32_t h) {
    uint16_t* dst = dst_pix;
    uint16_t* src = src_pix;

    Morton2D m2d = Morton2DInit(__builtin_ffs(w), __builtin_ffs(h));

    int32_t xmorton = 0, ymorton = 0;
    int32_t i, j;
    for (j = 0; j < h; j++) {
        for (i = 0, xmorton = 0; i < w; i++) {
            dst[xmorton | ymorton] = *src++;
            xmorton = M2DIncX(m2d, xmorton);
        }
        ymorton = M2DIncY(m2d, ymorton);
    }
}

void UnMakeTwiddled16(void* src_pix, void* dst_pix, int32_t w, int32_t h) {
    uint16_t* dst = dst_pix;
    uint16_t* src = src_pix;

    Morton2D m2d = Morton2DInit(__builtin_ffs(w), __builtin_ffs(h));

    int32_t xmorton = 0, ymorton = 0;
    int32_t i, j;
    for (j = 0; j < h; j++) {
        for (i = 0, xmorton = 0; i < w; i++) {
            *dst = src[xmorton | ymorton];
            dst++;
            xmorton = M2DIncX(m2d, xmorton);
        }
        ymorton = M2DIncY(m2d, ymorton);
    }
}

void UnMakeTwiddled8(void* src_pix, void* dst_pix, int32_t w, int32_t h) {
    uint8_t* dst = dst_pix;
    uint8_t* src = src_pix;

    Morton2D m2d = Morton2DInit(__builtin_ffs(w), __builtin_ffs(h));

    int32_t xmorton = 0, ymorton = 0;
    int32_t i, j;
    for (j = 0; j < h; j++) {
        for (i = 0, xmorton = 0; i < w; i++) {
            *dst = src[xmorton | ymorton];
            dst++;
            xmorton = M2DIncX(m2d, xmorton);
        }
        ymorton = M2DIncY(m2d, ymorton);
    }
}
