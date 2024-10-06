#ifndef _morton2d_h
#define _morton2d_h

#include <stdint.h>

void MakeTwiddled16(void* src_pix, void* dst_pix, int32_t w, int32_t h);
void UnMakeTwiddled16(void* src_pix, void* dst_pix, int32_t w, int32_t h);
void UnMakeTwiddled8(void* src_pix, void* dst_pix, int32_t w, int32_t h);

#endif
