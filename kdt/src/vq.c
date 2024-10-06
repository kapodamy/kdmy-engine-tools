#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/elbg.h>

#include "vq.h"


#define PIXEL_COMPONENTS 4

struct _VQ {
    size_t total_components_per_pixel;
    int32_t* int_points;
    int32_t dimensions;
    int32_t points_count;
    int32_t* int_codebook;
    int32_t codebook_size;
    int32_t num_steps;
    int32_t* int_indexes;
    size_t total_components_per_pixel_in_codebook;

    struct ELBGContext* elbgcxt;
    AVLFG randcxt;

    uint16_t* codebook;
    uint8_t* indexes;

    size_t total_pixels;
    int32_t total_pixels_in_codebook;

    void (*unpack)(uint16_t texel, int* points);
    uint16_t (*pack)(int* points);
};


static void unpack_yuv442(uint16_t texel, int* points) {
    points[0] = (texel >> 8) & 0xFF;
    points[1] = (texel >> 0) & 0xFF;
    points[2] = 0x00;
    points[3] = 0x00;
}

static uint16_t pack_yuv442(int* points) {
    return (points[0] << 8) | (points[1] << 0);
}

static void unpack_rgb565(uint16_t texel, int* points) {
    points[0] = (texel & 0xF800) >> 11;
    points[1] = (texel & 0x07E0) >> 5;
    points[2] = (texel & 0x001F) >> 0;
    points[3] = 0xFF;
}

static uint16_t pack_rgb565(int* points) {
    uint16_t texel = points[0] << 11;
    texel |= points[1] << 5;
    texel |= points[2] << 0;
    return texel;
}

static void unpack_argb1555(uint16_t texel, int* points) {
    points[3] = (texel & 0x8000) >> 15;
    points[0] = (texel & 0x7C00) >> 10;
    points[1] = (texel & 0x03E0) >> 5;
    points[2] = (texel & 0x001F) >> 0;
}

static uint16_t pack_argb1555(int* points) {
    uint16_t texel = points[3] << 15;
    texel |= points[0] << 10;
    texel |= points[1] << 5;
    texel |= points[2] << 0;

    return texel;
}

static void unpack_argb4444(uint16_t texel, int* points) {
    points[3] = (texel & 0xF000) >> 12;
    points[0] = (texel & 0x0F00) >> 8;
    points[1] = (texel & 0x00F0) >> 4;
    points[2] = (texel & 0x000F) >> 0;
}

static uint16_t pack_argb4444(int* points) {
    uint16_t texel = points[3] << 12;
    texel |= points[0] << 8;
    texel |= points[1] << 4;
    texel |= points[2] << 0;

    return texel;
}


VQ* vq_init(int32_t width, int32_t height, size_t pixels_per_codebook_entry, int32_t quality, int32_t codebook_size, KDT_PixelFormat pixfmt) {
    size_t total_pixels = width * height;
    size_t total_components_per_pixel = total_pixels * PIXEL_COMPONENTS;
    int32_t dimensions = pixels_per_codebook_entry * PIXEL_COMPONENTS;

    assert((total_pixels % pixels_per_codebook_entry) == 0);

    int32_t points_count = total_pixels / pixels_per_codebook_entry;
    int32_t* int_points = malloc(total_components_per_pixel * sizeof(int32_t));

    int32_t total_components_per_pixel_in_codebook = codebook_size * dimensions;
    int32_t total_pixels_in_codebook = codebook_size * pixels_per_codebook_entry;
    int32_t* int_codebook = malloc(total_components_per_pixel_in_codebook * sizeof(int32_t));
    int32_t* int_indexes = malloc(points_count * sizeof(int32_t));

    uint16_t* codebook = malloc(total_components_per_pixel_in_codebook);
    uint8_t* indexes = malloc(points_count);

    VQ* vq = malloc(sizeof(VQ));
    assert(vq);

    *vq = (VQ){
        .total_components_per_pixel = total_components_per_pixel,
        .int_points = int_points,
        .dimensions = dimensions,
        .points_count = points_count,
        .int_codebook = int_codebook,
        .codebook_size = codebook_size,
        .num_steps = quality < 1 ? 1 : quality,
        .int_indexes = int_indexes,
        .total_components_per_pixel_in_codebook = total_components_per_pixel_in_codebook,
        .elbgcxt = NULL,
        .codebook = codebook,
        .indexes = indexes,
        .total_pixels = total_pixels,
        .total_pixels_in_codebook = total_pixels_in_codebook,
        .pack = NULL,
        .unpack = NULL
    };

    switch (pixfmt) {
        case KDT_PixelFormat_YUV422:
            vq->pack = pack_yuv442;
            vq->unpack = unpack_yuv442;
            break;
        case KDT_PixelFormat_RGB565:
            vq->pack = pack_rgb565;
            vq->unpack = unpack_rgb565;
            break;
        case KDT_PixelFormat_ARGB1555:
            vq->pack = pack_argb1555;
            vq->unpack = unpack_argb1555;
            break;
        case KDT_PixelFormat_ARGB4444:
            vq->pack = pack_argb4444;
            vq->unpack = unpack_argb4444;
            break;
        default:
            assert(false);
            break;
    }

    av_lfg_init(&vq->randcxt, 1);

    return vq;
}

void vq_destroy(VQ* vq) {
    free(vq->int_points);
    free(vq->int_indexes);
    free(vq->int_codebook);
    free(vq->codebook);
    avpriv_elbg_free(&vq->elbgcxt);
    free(vq);
}

void vq_do(VQ* vq, uint16_t* pixels, uint16_t** codebook, uint8_t** indexes) {
    //
    // Note: the "Enhanced LBG Algorithm Based" expects each pixel channel separated,
    // do not try "int_points[i * 4] = pixels[i]" for RGB565 otherwise
    // the codebook (for some reason) will be unreadable.
    //
    int* int_points = vq->int_points;
    for (size_t i = 0; i < vq->total_pixels; i++) {
        vq->unpack(pixels[i], int_points);
        int_points += 4;
    }

    int errval = avpriv_elbg_do(
        &vq->elbgcxt,
        vq->int_points, vq->dimensions, vq->points_count,
        vq->int_codebook, vq->codebook_size,
        vq->num_steps, vq->int_indexes,
        &vq->randcxt, 0
    );
    assert(errval == 0);


    int* int_codebook = vq->int_codebook;
    for (int32_t i = 0; i < vq->total_pixels_in_codebook; i++) {
        vq->codebook[i] = vq->pack(int_codebook);
        int_codebook += 4;
    }

    // reuse "int_indexes" as "byte_indexes"
    uint8_t* byte_indexes = (uint8_t*)vq->int_indexes;

    for (int32_t i = 0; i < vq->points_count; i++) {
        byte_indexes[i] = (uint8_t)vq->int_indexes[i];
    }

    *codebook = vq->codebook;
    *indexes = byte_indexes;
}
