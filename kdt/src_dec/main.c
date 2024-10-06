#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>

#include "../src/color16.h"
#include "../src/kdt.h"
#include "../src/lzss.h"
#include "../src/morton2d.h"

#define MAX_DIMMEN 1024
#define CODEBOOK_ENTRY_COUNT 256
#define VQ_COORD(x, y, width) ((x) + ((y) * width))
#define STRING_EQUALS(str1, str2) (strcmp(str1, str2) == 0)
#define CHECK_OPTION_VALUE(value_string, jump_label) \
    if (value_string == NULL || value_string[0] == '\0') goto jump_label;


typedef struct __attribute__((__packed__)) {
    uint16_t texels[4];
} CodebookEntry;

typedef struct {
    const char* input_filename;
    char* output_filename;
    bool no_scale_factor;
    int32_t scale_algorithm;
} Arguments;

static inline void decode_vq_twiddled(void* data, uint16_t* buffer, int32_t frame_width, int32_t frame_height) {
    int total_pixels = frame_width * frame_height;
    int total_indexes = total_pixels / 4;
    CodebookEntry* codebook = (CodebookEntry*)data;
    uint8_t* indexes = (uint8_t*)data + (sizeof(CodebookEntry) * CODEBOOK_ENTRY_COUNT);
    size_t half_width = frame_width / 2;
    size_t half_height = frame_height / 2;
    size_t i = 0;

    // untwiddle indexes instead of pixels, otherwise it will produce a "blocky" image
    memcpy(buffer, indexes, total_indexes);
    UnMakeTwiddled8((uint8_t*)buffer, indexes, half_width, half_height);

    for (size_t y = 0; y < half_height; y++) {
        for (size_t x = 0; x < half_width; x++) {
            size_t offset_x = x * 2;
            size_t offset_y = y * 2;
            CodebookEntry* entry = codebook + indexes[i++];

            buffer[VQ_COORD(offset_x + 0, offset_y + 0, frame_width)] = entry->texels[0];
            buffer[VQ_COORD(offset_x + 1, offset_y + 0, frame_width)] = entry->texels[2];
            buffer[VQ_COORD(offset_x + 0, offset_y + 1, frame_width)] = entry->texels[1];
            buffer[VQ_COORD(offset_x + 1, offset_y + 1, frame_width)] = entry->texels[3];
        }
    }
}

static inline void decode_vq(void* data, uint16_t* buffer, int32_t frame_width, int32_t frame_height) {
    size_t total_pixels = frame_width * frame_height;
    size_t total_indexes = total_pixels / 4;
    CodebookEntry* codebook = (CodebookEntry*)data;
    uint8_t* indexes = (uint8_t*)data + (sizeof(CodebookEntry) * CODEBOOK_ENTRY_COUNT);
    int32_t x = 0, y = 0;

    for (size_t i = 0; i < total_indexes; i++) {
        CodebookEntry* entry = codebook + indexes[i];

        buffer[VQ_COORD(x + 0, y + 0, frame_width)] = entry->texels[0];
        buffer[VQ_COORD(x + 1, y + 0, frame_width)] = entry->texels[1];
        buffer[VQ_COORD(x + 0, y + 1, frame_width)] = entry->texels[2];
        buffer[VQ_COORD(x + 1, y + 1, frame_width)] = entry->texels[3];

        x += 2;
        if (x >= frame_width) {
            x = 0;
            y += 2;
        }
    }
}


static AVCodecContext* init_png_encoder(enum AVPixelFormat pix_fmt, int32_t width, int32_t height) {
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_PNG);
    if (!codec) {
        printf("Missing encoder for AV_CODEC_ID_PNG\n");
        return NULL;
    }

    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        printf("Failed to allocate codec context\n");
        return NULL;
    }

    ctx->width = width;
    ctx->height = height;
    ctx->sample_aspect_ratio = (AVRational){width, height};
    ctx->pix_fmt = pix_fmt;
    ctx->compression_level = 9;
    ctx->time_base = (AVRational){1, 1};
    ctx->framerate = (AVRational){1, 1};

    int ret = avcodec_open2(ctx, codec, NULL);
    if (ret < 0) {
        printf("Failed to initialize the codec context: %s\n", av_err2str(ret));
        avcodec_free_context(&ctx);
        return NULL;
    }

    return ctx;
}

static bool write_png(const char* filename, bool has_alpha, int32_t width, int32_t height, void* data) {
    enum AVPixelFormat pix_fmt = has_alpha ? AV_PIX_FMT_RGBA : AV_PIX_FMT_RGB24;
    bool success = false;
    size_t pixsize = has_alpha ? sizeof(RGBA8) : sizeof(RGB24);

    AVCodecContext* ctx = init_png_encoder(pix_fmt, width, height);
    if (!ctx) {
        return false;
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        printf("can not allocate the encoder frame\n");
        goto L_return;
    }
    frame->format = pix_fmt;
    frame->width = width;
    frame->height = height;
    frame->time_base = (AVRational){1, 1};
    frame->pts = 0;

    int ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        printf("can not allocate the frame buffer: %s\n", av_err2str(ret));
        goto L_return;
    }

    // calculate the line size manually to avoid padding/aligment
    frame->linesize[0] = width * pixsize;

    memcpy(frame->data[0], data, ctx->width * ctx->height * pixsize);

    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        printf("can not allocate the encoder packet\n");
        goto L_return;
    }

    ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        printf("png encoding failed: %s\n", av_err2str(ret));
        goto L_return;
    }

    ret = avcodec_receive_packet(ctx, packet);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        // this never should happen
        printf("error, no packet from the encoder\n");
        goto L_return;
    } else if (ret < 0) {
        printf("failed to receive the encoder packet: %s\n", av_err2str(ret));
        goto L_return;
    }

    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("failed create the output file: %s\n", filename);
        goto L_return;
    }

    fwrite(packet->data, packet->size, 1, file);
    fclose(file);
    success = true;

L_return:
    if (frame) av_frame_free(&frame);
    if (packet) av_packet_free(&packet);
    return success;
}


/*static void dump_file(void* buffer, int32_t width, int32_t height) {
    static int index = 0;
    char fn[128];
    int at = snprintf(fn, sizeof(fn), "./dump_%i.bin", index++);
    assert(at > 0);
    fn[at] = '\0';

    FILE* f = fopen(fn, "wb");
    fwrite(buffer, sizeof(uint16_t), width * height, f);
    fflush(f);
    fclose(f);
}*/

static inline void convert_yuv422(uint16_t* src, void* dst, int32_t frame_width, int32_t width, int32_t height) {
    RGB24* d = dst;
    size_t frame_stride = frame_width - width;

    for (int32_t y = 0; y < height; y++) {
        for (int32_t x = 0; x < width; x += 2) {
            uint16_t texel1 = *src++;
            uint16_t texel2 = *src++;
            RGB24Pixels packed = yuv422_to_rgb24_packed(texel1, texel2);
            *d++ = packed.pixel1;
            *d++ = packed.pixel2;
        }
        src += frame_stride;
    }
}

static inline void convert_rgb565(uint16_t* src, void* dst, int32_t frame_width, int32_t width, int32_t height) {
    RGB24* d = dst;
    size_t frame_stride = frame_width - width;
    printf("frame_stride was %zu\n", frame_stride);
    // if (frame_stride > 0) frame_stride--;

    for (int32_t y = 0; y < height; y++) {
        for (int32_t x = 0; x < width; x++) {
            *d++ = rgb565_to_rgb24(*src++);
        }
        src += frame_stride;
    }
}

static inline void convert_argb4444(uint16_t* src, void* dst, int32_t frame_width, int32_t width, int32_t height) {
    RGBA8* d = dst;
    size_t frame_stride = frame_width - width;
    // if (frame_stride > 0) frame_stride--;

    for (int32_t y = 0; y < height; y++) {
        for (int32_t x = 0; x < width; x++) {
            *d++ = argb4444_to_rgba8(*src++);
        }
        src += frame_stride;
    }
}

static inline void convert_argb1555(uint16_t* src, void* dst, int32_t frame_width, int32_t width, int32_t height) {
    RGBA8* d = dst;
    size_t frame_stride = frame_width - width;
    // if (frame_stride > 0) frame_stride--;

    for (int32_t y = 0; y < height; y++) {
        for (int32_t x = 0; x < width; x++) {
            *d++ = argb1555_to_rgba8(*src++);
        }
        src += frame_stride;
    }
}


static int32_t calc_slice_block_dimmen(int32_t offset, int32_t subslice_cutoff_offset, int32_t frame_dimmen) {
    if (offset < subslice_cutoff_offset) {
        int32_t block_subslice_dimmen = subslice_cutoff_offset - offset;
        if (block_subslice_dimmen < MAX_DIMMEN) {
            return block_subslice_dimmen;
        }
    } else if (offset == subslice_cutoff_offset) {
        return frame_dimmen - subslice_cutoff_offset;
    }

    int32_t block_dimmen = frame_dimmen - offset;
    if (block_dimmen > MAX_DIMMEN) {
        return MAX_DIMMEN;
    }

    return block_dimmen;
}


static inline bool sws_run(bool a, void* i, void* b, int32_t ow, int32_t oh, int32_t dw, int32_t dh, int32_t ds, Arguments* v) {
    enum AVPixelFormat pixfmt = a ? AV_PIX_FMT_RGBA : AV_PIX_FMT_RGB24;

    struct SwsContext* sws_ctx = sws_getContext(
        ow, oh, pixfmt,
        dw, dh, pixfmt,
        v->scale_algorithm, NULL, NULL, NULL
    );

    if (sws_ctx == NULL) {
        printf("call to sws_getContext() failed\n");
        return false;
    }

    const uint8_t* const srcSlice[4] = {(const uint8_t*)i, NULL, NULL, NULL};
    int srcStride[4];
    av_image_fill_linesizes(srcStride, pixfmt, ow);

    uint8_t* const dstSlice[] = {b, NULL, NULL, NULL};
    int dstStride[4];
    av_image_fill_linesizes(dstStride, pixfmt, ds);

    sws_scale(sws_ctx, srcSlice, srcStride, 0, oh, dstSlice, dstStride);

    sws_freeContext(sws_ctx);
    return true;
}


static void print_avoption_values(const AVOption* options, const char* option_name) {
    for (size_t i = 0;; i++) {
        const char* name = options[i].name;
        const char* unit = options[i].unit;
        if (name == NULL) break;

        if (!unit || strcmp(unit, option_name) != 0 || options[i].offset != 0) {
            continue;
        }

        printf("    %-15s    %s\n", name, options[i].help);
    }
}

static bool has_avoption_value(const AVOption* options, const char* option_name, const char* option_value, int64_t* value) {
    for (size_t i = 0;; i++) {
        const char* name = options[i].name;
        const char* unit = options[i].unit;
        if (name == NULL) break;

        if (!unit || strcmp(unit, option_name) != 0 || options[i].offset != 0) {
            continue;
        }
        if (STRING_EQUALS(options[i].name, option_value)) {
            *value = options[i].default_val.i64;
            return true;
        }
    }

    return false;
}

static void print_usage(int argc, char** argv) {
    const char* program = argc > 0 ? argv[0] : "kdt_dec";

    size_t idx = 0;
    for (size_t i = 0; program[i] != '\0'; i++) {
        if (program[i] == '/' || program[i] == '\\') {
            idx = i + 1;
        }
    }
    program = program + idx;

    const AVClass* sws_avclass = sws_get_class();
    const AVOption* sws_options = sws_avclass->option;

    printf("KDT decoder v0.2 by kapodamy\n");
    printf("Proof-of-Concept decoder, decode KDT texture files into PNG images. ");
    printf("Mipmapping is not implemented\n");
    printf("\n");
    printf("Usage: %s [options...] <input kdt file> <output png file>\n", program);
    printf("\n");
    printf("Common options:\n");
    printf(" -s, --scale-algorithm <scale algorithm>        Used only if the image needs upscaling. Default: lanczos\n");
    printf(" -f, --ignore-scale-factor                      Do not upscale to the original image size, decode as-is.\n");
    printf("Scale algorithms:\n");
    print_avoption_values(sws_options, "sws_flags");
    printf("\n");
}

static void parse_arguments(int argc, char** argv, const AVOption* sws_options, Arguments* values) {
    if (argc < 2 || STRING_EQUALS(argv[1], "-h") || STRING_EQUALS(argv[1], "--help")) {
        print_usage(argc, argv);
        exit(0);
    }

    const char* in_filename = NULL;
    const char* out_filename = NULL;
    size_t option_index = 0, value_index = 0;
    int limit = argc - 1;
    int64_t avoption_value = 0;

    for (int i = 1; i < argc; i++) {
        bool is_option = false;
        const char* option_name = argv[i];
        const char* option_value = NULL;

        if (option_name[0] == '\0') continue;

        if (i < limit) {
            if (option_name[0] == '-' && option_name[1] == '-') {
                option_name += 2;
                is_option = true;
            } else if (option_name[0] == '-') {
                option_name += 1;
                is_option = true;
            }
        }

        if (is_option) {
            option_index = i;
            int j = i + 1;

            if (j < argc) {
                option_value = argv[j];
                value_index = j;
            }
        } else {
            goto L_option_as_filename;
        }

        if (STRING_EQUALS(option_name, "h") || STRING_EQUALS(option_name, "help")) {
            print_usage(argc, argv);
            exit(0);
            return;
        } else if (STRING_EQUALS(option_name, "s") || STRING_EQUALS(option_name, "scale-algorithm")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            if (has_avoption_value(sws_options, "sws_flags", option_value, &avoption_value))
                values->scale_algorithm = (int32_t)avoption_value;
            else
                goto L_invalid_value;
        } else if (STRING_EQUALS(option_name, "f") || STRING_EQUALS(option_name, "no-scale-factor")) {
            values->no_scale_factor = true;
        } else {
            is_option = false;
        }

    L_option_as_filename:
        if (!is_option) {
            if (!in_filename) {
                in_filename = option_name;
            } else if (!out_filename) {
                out_filename = option_name;
            } else {
                printf("unknown option '%s'\n", option_name);
                print_usage(argc, argv);
                exit(1);
            }
        }
    }

    if (!out_filename && !in_filename) {
        printf("missing input and output filenames.\n");
        exit(1);
        return;
    }

    FILE* in_file = fopen(in_filename, "rb");
    if (in_file) {
        fclose(in_file);
    } else {
        printf("failed to open '%s' input file.\n", in_filename);
        exit(1);
        return;
    }

    char* out_filename2;
    if (out_filename) {
        out_filename2 = strdup(out_filename);
        assert(out_filename2);
    } else {
        size_t bar_idx = 0, dot_idx = 0, length = 0;
        for (size_t i = 1; in_filename[i] != '\0'; i++) {
            switch (in_filename[i]) {
                case '/':
                case '\\':
                    bar_idx = i;
                    break;
                case '.':
                    dot_idx = i;
                    break;
            }
            length++;
        }

        if (bar_idx >= dot_idx && dot_idx != 0) {
            dot_idx = length;
        }

        out_filename2 = malloc(dot_idx + sizeof(".kdt"));
        assert(out_filename2);

        memcpy(out_filename2, in_filename, dot_idx);
        out_filename2[dot_idx + 0] = '.';
        out_filename2[dot_idx + 1] = 'p';
        out_filename2[dot_idx + 2] = 'n';
        out_filename2[dot_idx + 3] = 'g';
        out_filename2[dot_idx + 4] = '\0';
    }

    FILE* out_file = fopen(out_filename2, "wb");
    if (out_file) {
        fclose(out_file);
    } else {
        printf("failed to open '%s' output file.\n", out_filename2);
        free(out_filename2);
        exit(1);
        return;
    }
    values->input_filename = in_filename;
    values->output_filename = out_filename2;
    return;

L_missing_value:
    printf("missing value for option '%s'\n", argv[option_index]);
    exit(1);
    return;

L_invalid_value:
    printf("invalid value '%s' for option '%s'\n", argv[value_index], argv[option_index]);
    exit(1);
    return;
}



int main(int argc, char** argv) {
    bool success = false;
    uint16_t* canvas = NULL;
    uint16_t* block_canvas = NULL;
    void* final_image = NULL;

    Arguments values = (Arguments){
        .input_filename = NULL,
        .output_filename = NULL,
        .no_scale_factor = false,
        .scale_algorithm = SWS_LANCZOS
    };

    parse_arguments(argc, argv, sws_get_class()->option, &values);

    FILE* file = fopen(values.input_filename, "rb");
    if (!file) {
        printf("can not open the file: %s\n", values.input_filename);
        free(values.output_filename);
        return 1;
    }

    printf("Decoding:      %s\n", values.input_filename);

    fseek(file, 0, SEEK_END);
    long file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    KDT* kdt = av_malloc(file_length);
    assert(kdt);

    fread(kdt, file_length, 1, file);
    fclose(file);

    if (kdt->signature != KDT_SIGNATURE) {
        printf("the file '%s' is not a KDT texture\n", values.input_filename);
        free(values.output_filename);
        av_free(kdt);
        return 1;
    }

    KDT_PixelFormat pixel_format = (KDT_PixelFormat)kdt->pixel_format;
    bool vq = kdt->flags & KDT_HEADER_FLAG_VQ;
    bool twiddle = kdt->flags & KDT_HEADER_FLAG_TWIDDLE;
    size_t data_size = file_length - offsetof(KDT, data);

    assert(kdt->encoded_width > 0 && kdt->encoded_height > 0);

    assert(kdt->encoded_width <= kdt->original_width);
    assert(kdt->encoded_height <= kdt->original_height);

    assert(kdt->encoded_width <= kdt->frame_width);
    assert(kdt->encoded_height <= kdt->frame_height);

    if (~kdt->flags & KDT_HEADER_FLAG_UPERSLICE)
        assert(kdt->encoded_width <= MAX_DIMMEN && kdt->encoded_height <= MAX_DIMMEN);

    if (kdt->flags & KDT_HEADER_FLAG_SUBSLICE) {
        assert(kdt->subslice_cutoff_x < kdt->frame_width);
        assert(kdt->subslice_cutoff_y < kdt->frame_height);
        assert(kdt->subslice_cutoff_x != 0 || kdt->subslice_cutoff_y != 0);
    } else {
        assert(kdt->subslice_cutoff_x == 0 && kdt->subslice_cutoff_y == 0);
    }

    if (kdt->flags & KDT_HEADER_FLAG_OPACITYSLICE) {
        assert(kdt->padding_width > 0 || kdt->padding_height > 0);
        assert(kdt->padding_width < kdt->original_width);
        assert(kdt->padding_height < kdt->original_height);
    } else {
        assert(kdt->padding_width == 0);
        assert(kdt->padding_height == 0);
    }

    int32_t max_slice_dimmen = kdt->encoded_width > kdt->encoded_height ? kdt->encoded_width : kdt->encoded_height;
    max_slice_dimmen = ((max_slice_dimmen + MAX_DIMMEN - 1) / MAX_DIMMEN) * MAX_DIMMEN;

    int32_t frame_width = kdt->frame_width;
    int32_t frame_height = kdt->frame_height;
    int32_t subslice_cutoff_offset_x, subslice_cutoff_offset_y;
    if (kdt->flags & KDT_HEADER_FLAG_SUBSLICE) {
        subslice_cutoff_offset_x = kdt->subslice_cutoff_x < 1 ? -1 : kdt->subslice_cutoff_x;
        subslice_cutoff_offset_y = kdt->subslice_cutoff_y < 1 ? -1 : kdt->subslice_cutoff_y;
    } else {
        subslice_cutoff_offset_x = -1;
        subslice_cutoff_offset_y = -1;
    }

    if (kdt->flags & KDT_HEADER_FLAG_LZSS) {
        uint8_t* lzss_buffer;
        size_t lzss_buffer_size;
        lzss_decompress(kdt->data, data_size, &lzss_buffer, &lzss_buffer_size);

        assert(lzss_buffer);
        assert(lzss_buffer_size > 0);

        kdt = av_realloc(kdt, lzss_buffer_size + sizeof(KDT));
        assert(kdt);

        memcpy(kdt->data, lzss_buffer, lzss_buffer_size);
        data_size = lzss_buffer_size;

        free(lzss_buffer);
    }

    size_t frame_size = frame_width * frame_height * sizeof(uint16_t);
    canvas = av_malloc(frame_size);
    assert(canvas);

    bool has_alpha = pixel_format == KDT_PixelFormat_ARGB1555 || pixel_format == KDT_PixelFormat_ARGB4444;
    const char* pixfmt_name = NULL;

    switch (pixel_format) {
        case KDT_PixelFormat_YUV422:
            pixfmt_name = "YUV422";
            break;
        case KDT_PixelFormat_RGB565:
            pixfmt_name = "RGB565";
            break;
        case KDT_PixelFormat_ARGB1555:
            pixfmt_name = "ARGB1555";
            break;
        case KDT_PixelFormat_ARGB4444:
            pixfmt_name = "ARGB4444";
            break;
        default:
            printf("unknown pixel format: 0x%x\n", pixel_format);
            goto L_return;
    }

    int32_t output_width = kdt->original_width;
    int32_t output_height = kdt->original_height;

    if (values.no_scale_factor) {
        output_width = kdt->encoded_width;
        output_height = kdt->encoded_height;
        if (kdt->flags & KDT_HEADER_FLAG_OPACITYSLICE) {
            printf("WARNING: the OPACITYSLICE flag is present the 'ignore-scale-factor' option will ignore padding.\n");
        }
    }

    size_t total_bytes = has_alpha ? sizeof(RGBA8) : sizeof(RGB24);
    total_bytes *= (kdt->original_width > kdt->encoded_width ? kdt->original_width : kdt->encoded_width) + kdt->padding_width;
    total_bytes *= (kdt->original_height > kdt->encoded_height ? kdt->original_height : kdt->encoded_height) + kdt->padding_height;

    final_image = av_malloc(total_bytes);
    assert(final_image);

    block_canvas = malloc(MAX_DIMMEN * MAX_DIMMEN * sizeof(uint16_t));
    assert(block_canvas);

    // decode chunks
    uint8_t* data_ptr = kdt->data;
    uint8_t* data_end_ptr = kdt->data + data_size;
    for (int32_t offset_x = 0, offset_y = 0; offset_y < frame_height;) {
        int32_t block_width = calc_slice_block_dimmen(offset_x, subslice_cutoff_offset_x, frame_width);
        int32_t block_height = calc_slice_block_dimmen(offset_y, subslice_cutoff_offset_y, frame_height);

        size_t block_size = block_width * block_height * sizeof(uint16_t);
        if (vq) block_size = (block_size / 8) + 2048;

        assert(data_ptr < data_end_ptr);
        assert((size_t)(data_end_ptr - data_ptr) >= block_size);

        // decode
        if (twiddle) {
            if (vq)
                decode_vq_twiddled(data_ptr, block_canvas, block_width, block_height);
            else
                UnMakeTwiddled16(data_ptr, block_canvas, block_width, block_height);
        } else if (vq) {
            decode_vq(data_ptr, block_canvas, block_width, block_height);
        } else {
            memcpy(block_canvas, data_ptr, block_size);
        }
        // dump_file(block_canvas, block_width, block_height);

        data_ptr += block_size;

        // write to canvas
        uint16_t* canvas_ptr = canvas + offset_x + (frame_width * offset_y);
        uint16_t* block_canvas_ptr = block_canvas;
        for (int32_t y = 0; y < block_height; y++) {
            memcpy(canvas_ptr, block_canvas_ptr, block_width * sizeof(uint16_t));
            canvas_ptr += frame_width;
            block_canvas_ptr += block_width;
        }
        // dump_file(canvas, frame_width, frame_height);

        offset_x += block_width;
        if (offset_x >= frame_width) {
            offset_x = 0;
            offset_y += block_height;
        }
    }

    switch (pixel_format) {
        case KDT_PixelFormat_YUV422:
            convert_yuv422(canvas, final_image, frame_width, kdt->encoded_width, kdt->encoded_height);
            break;
        case KDT_PixelFormat_RGB565:
            convert_rgb565(canvas, final_image, frame_width, kdt->encoded_width, kdt->encoded_height);
            break;
        case KDT_PixelFormat_ARGB4444:
            convert_argb4444(canvas, final_image, frame_width, kdt->encoded_width, kdt->encoded_height);
            break;
        case KDT_PixelFormat_ARGB1555:
            convert_argb1555(canvas, final_image, frame_width, kdt->encoded_width, kdt->encoded_height);
            break;
        default:
            assert(false);
            break;
    }

    int32_t tmp_width = kdt->original_width - kdt->padding_width;
    int32_t tmp_height = kdt->original_height - kdt->padding_height;

    if (!values.no_scale_factor && ((kdt->encoded_width != tmp_width) || (kdt->encoded_height != tmp_height))) {
        void* buffer = av_malloc(total_bytes);
        assert(buffer);

        int32_t stride = kdt->original_width;

        if (!sws_run(has_alpha, final_image, buffer, kdt->encoded_width, kdt->encoded_height, tmp_width, tmp_height, stride, &values)) {
            av_free(buffer);
            goto L_return;
        }

        av_free(final_image);
        final_image = buffer;
    }

    if (!write_png(values.output_filename, has_alpha, output_width, output_height, final_image)) {
        success = false;
        goto L_return;
    }

    // print stats
    printf(
        "Metadata:      %s%s%s%s%s%s%s\n",
        pixfmt_name,
        twiddle ? " TWIDDLE" : "",
        vq ? " VQ" : "",
        (kdt->flags & KDT_HEADER_FLAG_SUBSLICE) ? " SUBSLICE" : "",
        (kdt->flags & KDT_HEADER_FLAG_UPERSLICE) ? " UPERSLICE" : "",
        (kdt->flags & KDT_HEADER_FLAG_LZSS) ? " LZSS" : "",
        (kdt->flags & KDT_HEADER_FLAG_OPACITYSLICE) ? " OPACITYSLICE" : ""
    );
    printf(
        "Dimmensions:   original=%ix%i encoded=%ix%i frame=%ix%i%s\n",
        kdt->original_width, kdt->original_height,
        kdt->encoded_width, kdt->encoded_height,
        frame_width, frame_height,
        values.no_scale_factor ? " (upscale disabled)" : ""
    );
    printf("Succesfully created %s\n", values.output_filename);

L_return:
    if (final_image) av_free(final_image);
    if (canvas) av_free(canvas);
    if (block_canvas) free(block_canvas);
    if (values.output_filename) free(values.output_filename);
    av_free(kdt);
    return success ? 0 : 1;
}
