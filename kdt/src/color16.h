#ifndef _color16_h
#define _color16_h

#include <stdint.h>


typedef struct __attribute__((packed)) {
    uint8_t r, g, b;
} RGB24;

typedef union {
    struct {
        uint8_t r, g, b, a;
    };
    uint32_t texel;
} RGBA8;

typedef struct {
    uint16_t pixel1, pixel2;
} YUV422Pixels;

typedef struct {
    RGB24 pixel1, pixel2;
} RGB24Pixels;


static inline uint32_t clamp_u8(int32_t value) {
    if (value < 0)
        return 0;
    else if (value > UINT8_MAX)
        return UINT8_MAX;
    else
        return (uint32_t)value;
}

static inline uint32_t clamp_float_u8(float value) {
    if (value < 0)
        return 0;
    else if (value > UINT8_MAX)
        return UINT8_MAX;
    else
        return (uint32_t)value;
}

static inline float clamp_i8_to_float(int32_t value) {
    float percent = value / 255.0f;

    if (percent < 0.0f)
        return 0.0f;
    else if (percent > 1.0f)
        return 1.0f;
    else
        return percent;
}

static inline RGB24 yuv422_to_rgb24(uint8_t y, uint8_t u, uint8_t v) {
    return (RGB24){
        .r = clamp_float_u8(y + 1.375f * (v - 128)),
        .g = clamp_float_u8(y - 0.6875f * (v - 128) - 0.34375f * (u - 128)),
        .b = clamp_float_u8(y + 1.71875f * (u - 128))
    };
}

static inline RGB24Pixels yuv422_to_rgb24_packed(uint16_t texel1, uint16_t texel2) {
    uint8_t y1 = (texel1 & 0xFF00) >> 8;
    uint8_t y2 = (texel2 & 0xFF00) >> 8;
    uint8_t u = (texel1 & 0x00FF);
    uint8_t v = (texel2 & 0x00FF);

    return (RGB24Pixels){
        .pixel1 = yuv422_to_rgb24(y1, u, v),
        .pixel2 = yuv422_to_rgb24(y2, u, v),
    };
}

static inline RGB24 rgb565_to_rgb24(uint16_t texel) {
    return (RGB24){
        .r = (texel & 0xF800) >> 8,
        .g = (texel & 0x07E0) >> 3,
        .b = (texel & 0x001F) << 3
    };
}

static inline RGBA8 argb4444_to_rgba8(uint16_t texel) {
    RGBA8 value;
    value.r = (texel & 0x0F00) >> 4;
    value.g = (texel & 0x00F0) >> 0;
    value.b = (texel & 0x000F) << 4;
    value.a = (texel & 0xF000) >> 8;
    return value;
}

static inline RGBA8 argb1555_to_rgba8(uint16_t texel) {
    RGBA8 value;
    value.r = (texel & 0x7C00) >> 7;
    value.g = (texel & 0x03E0) >> 2;
    value.b = (texel & 0x001F) << 3;
    value.a = (texel & 0x8000) ? 0xFF : 0x00;
    return value;
}


static inline uint16_t rgba8_to_argb4444(RGBA8 texel) {
    uint16_t value = 0x00;
    value |= (texel.a << 8) & 0xF000;
    value |= (texel.r << 4) & 0x0F00;
    value |= (texel.g >> 0) & 0x00F0;
    value |= (texel.b >> 4) & 0x000F;
    return value;
}

static inline uint16_t rgba8_to_argb1555(RGBA8 texel) {
    uint16_t value = texel.a > 0x7F ? 0x8000 : 0x00;
    value |= (texel.r << 7) & 0x7C00;
    value |= (texel.g << 2) & 0x03E0;
    value |= (texel.b >> 3) & 0x001F;
    return value;
}

static inline RGB24 rgba8_to_rgb24(RGBA8 texel) {
    float r = clamp_i8_to_float(texel.r);
    float g = clamp_i8_to_float(texel.g);
    float b = clamp_i8_to_float(texel.b);
    float a = clamp_i8_to_float(texel.a);

    return (RGB24){
        .r = clamp_float_u8(r * a),
        .g = clamp_float_u8(g * a),
        .b = clamp_float_u8(b * a),
    };
}

static inline YUV422Pixels rgb24_to_yuv422(RGB24 texel1, RGB24 texel2) {
    uint32_t y0 = clamp_float_u8(0.299f * texel1.r + 0.587f * texel1.g + 0.114f * texel1.b);
    uint32_t y1 = clamp_float_u8(0.299f * texel2.r + 0.587f * texel2.g + 0.114f * texel2.b);

    float avg_r = (texel1.r + texel2.r) / 2.0f;
    float avg_g = (texel1.g + texel2.g) / 2.0f;
    float avg_b = (texel1.b + texel2.b) / 2.0f;

    uint32_t u = clamp_float_u8((-0.169f * avg_r) - (0.331f * avg_g) + (0.499f * avg_b) + 128.0f);
    uint32_t v = clamp_float_u8((0.499f * avg_r) - (0.418f * avg_g) - (0.0813f * avg_b) + 128.0f);

    return (YUV422Pixels){
        .pixel1 = (uint16_t)((y0 << 8) | u),
        .pixel2 = (uint16_t)((y1 << 8) | v),
    };
}

#endif
