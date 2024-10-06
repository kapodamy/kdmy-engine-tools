#ifndef _vq_h
#define _vq_h

#include <stdint.h>
#include <stdbool.h>

#include "kdt.h"


typedef struct _VQ VQ;


VQ* vq_init(int32_t width, int32_t height, size_t pixels_per_codebook_entry, int32_t quality, int32_t codebook_size, KDT_PixelFormat pixfmt);
void vq_destroy(VQ* vq);
void vq_do(VQ* vq, uint16_t* pixels, uint16_t** codebook, uint8_t** indexes);

#endif
