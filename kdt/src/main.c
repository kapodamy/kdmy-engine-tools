#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

#include "ffgraph/ffgraph.h"

#include "magick.h"

#include "color16.h"
#include "kdt.h"
#include "lzss.h"
#include "morton2d.h"
#include "vq.h"


#define SCALING_LEVELS_TOTAL 14
#define MAX_DIMMEN 1024
#define ICO_DIMMEN 128
#define MAX_SLICE_BLOCKS 3
#define MIN_SUBSLICE_DIMMEN 8
#define MIN_COMPRESSION_RATIO 0.2
#define USE_FFMPEG_SWS_YUV_CONVERSOR

#define STRING_EQUALS(str1, str2) (strcmp(str1, str2) == 0)
#define CHECK_OPTION_VALUE(value_string, jump_label) \
    if (value_string == NULL || value_string[0] == '\0') goto jump_label;
#define SETF(boolean, flag) ((boolean) ? (flag) : 0x00)
#define APPEND_CHAR(value)                 \
    {                                      \
        count++;                           \
        if (count > max_length) return -1; \
        *result++ = value;                 \
    }


typedef enum {
    DownScaler_Prc_CLAMP,
    DownScaler_Prc_MEDIUM,
    DownScaler_Prc_HARD,
} DownScaler_Prc;

typedef struct {
    int32_t max_dimmen;
    double scale_factor;
} ScaleLevel;

typedef struct {
    uint16_t texels[4];
} CodebookEntry;

typedef struct {
    CodebookEntry codebook[256];
    uint8_t indexes[];
} PVR_VQ;

typedef struct {
    const char* input_filename;
    char* output_filename;
    bool rgb565;
    bool vq;
    bool twiddle;
    bool force_vq_on_small;
    bool force_square_vq;
    bool uper_slice;
    bool sub_slice;
    bool opacity_slice;
    bool lzss;
    float scale_factor;
    int32_t scale_factor_limit;
    int32_t max_dimmen;
    int32_t quality;
    int32_t scale_algorithm;
    int32_t uper_slice_max_blocks;
    const char* dither;
    DownScaler_Prc downscale_procedure;
    KDT_PixelFormat pixel_format;
    const char* magick_exe;
} Arguments;

typedef struct __attribute__((__packed__)) {
    uint16_t reserved;
    uint16_t type;
    uint16_t images;
} ICO_header;

typedef struct __attribute__((__packed__)) {
    uint8_t width;
    uint8_t height;
    uint8_t palette_length;
    uint8_t reserved;
    uint16_t color_planes;
    uint16_t bits_per_pixel;
    uint32_t size;
    uint32_t offset;
} ICO_Entry;


static const ScaleLevel SCALING_LEVELS_CLAMP[3][5] = {
    {
        {.max_dimmen = 1024, .scale_factor = 1},
        {.max_dimmen = 2048, .scale_factor = 2},
        {.max_dimmen = 3072, .scale_factor = 2},
        {.max_dimmen = 4096, .scale_factor = 4},
        {.max_dimmen = 8192, .scale_factor = 8},
    },
    {
        {.max_dimmen = 1024, .scale_factor = 1},
        {.max_dimmen = 2048, .scale_factor = 1},
        {.max_dimmen = 3072, .scale_factor = 1},
        {.max_dimmen = 4096, .scale_factor = 2},
        {.max_dimmen = 8192, .scale_factor = 4},
    },
    {
        {.max_dimmen = 1024, .scale_factor = 1},
        {.max_dimmen = 2048, .scale_factor = 1},
        {.max_dimmen = 3072, .scale_factor = 1},
        {.max_dimmen = 4096, .scale_factor = 4.0 / 3.0},
        {.max_dimmen = 8192, .scale_factor = 8.0 / 3.0},
    }
};
static const ScaleLevel SCALING_LEVELS_MEDIUM[3][6] = {
    {
        {.max_dimmen = 512, .scale_factor = 1},
        {.max_dimmen = 1024, .scale_factor = 2},
        {.max_dimmen = 2048, .scale_factor = 4},
        {.max_dimmen = 3072, .scale_factor = 4},
        {.max_dimmen = 4096, .scale_factor = 8},
        {.max_dimmen = 8192, .scale_factor = 8},
    },
    {
        {.max_dimmen = 512, .scale_factor = 1},
        {.max_dimmen = 1024, .scale_factor = 1},
        {.max_dimmen = 2048, .scale_factor = 2},
        {.max_dimmen = 3072, .scale_factor = 2},
        {.max_dimmen = 4096, .scale_factor = 2},
        {.max_dimmen = 8192, .scale_factor = 4},
    },
    {
        {.max_dimmen = 512, .scale_factor = 1},
        {.max_dimmen = 1024, .scale_factor = 1},
        {.max_dimmen = 2048, .scale_factor = 1},
        {.max_dimmen = 3072, .scale_factor = 2},
        {.max_dimmen = 4096, .scale_factor = 4.0 / 3.0},
        {.max_dimmen = 8192, .scale_factor = 8.0 / 3.0},
    }
};
static const ScaleLevel SCALING_LEVELS_HARD[3][7] = {
    {
        {.max_dimmen = 256, .scale_factor = 2},
        {.max_dimmen = 512, .scale_factor = 2},
        {.max_dimmen = 1024, .scale_factor = 4},
        {.max_dimmen = 2048, .scale_factor = 4},
        {.max_dimmen = 3072, .scale_factor = 4},
        {.max_dimmen = 4096, .scale_factor = 8},
        {.max_dimmen = 8192, .scale_factor = 8},
    },
    {
        {.max_dimmen = 256, .scale_factor = 1},
        {.max_dimmen = 512, .scale_factor = 1},
        {.max_dimmen = 1024, .scale_factor = 2},
        {.max_dimmen = 2048, .scale_factor = 2},
        {.max_dimmen = 3072, .scale_factor = 2},
        {.max_dimmen = 4096, .scale_factor = 2},
        {.max_dimmen = 8192, .scale_factor = 4},
    },
    {
        {.max_dimmen = 256, .scale_factor = 1},
        {.max_dimmen = 512, .scale_factor = 1},
        {.max_dimmen = 1024, .scale_factor = 2.0 / 3.0},
        {.max_dimmen = 2048, .scale_factor = 2.0 / 3.0},
        {.max_dimmen = 3072, .scale_factor = 2.0 / 3.0},
        {.max_dimmen = 4096, .scale_factor = 4.0 / 3.0},
        {.max_dimmen = 8192, .scale_factor = 8.0 / 3.0},
    }
};


static const AVOption advanced_options[] = {
    {.offset = 1, .unit = "downscale_procedures", .name = "Downscale procedures"},
    {.offset = 0, .unit = "downscale_procedures", .name = "CLAMP", .help = "Resizes textures with dimensions 2048x2048, 4096x4096 and 8192x8192 to 1024x1024.", .default_val = {.i64 = DownScaler_Prc_CLAMP}},
    {.offset = 0, .unit = "downscale_procedures", .name = "MEDIUM", .help = "Resizes specific texture sizes: 1024x1024->256x256, 2048x2048->512x512, 4096x4096->1024x1024 and 8192x8192->1024x1024", .default_val = {.i64 = DownScaler_Prc_MEDIUM}},
    {.offset = 0, .unit = "downscale_procedures", .name = "HARD", .help = "Resizes specific texture sizes: 256x256->128x128, 512x512->256x256, 1024x1024->256x256, 2048x2048->512x512, 4096x4096->512x512 and 8192x8192->1024x1024", .default_val = {.i64 = DownScaler_Prc_HARD}},
    {.offset = 1, .unit = "pixel_formats", .name = "Pixel formats"},
    {.offset = 0, .unit = "pixel_formats", .name = "AUTO", .help = "The encoder automatically chooses a pixel format", .default_val = {.i64 = KDT_PixelFormat_NONE}},
    {.offset = 0, .unit = "pixel_formats", .name = "YUV422", .help = "Offers higher percieved quality than RGB formats, no transparency", .default_val = {.i64 = KDT_PixelFormat_YUV422}},
    {.offset = 0, .unit = "pixel_formats", .name = "RGB565", .help = "Classic RGB format, no transparency", .default_val = {.i64 = KDT_PixelFormat_RGB565}},
    {.offset = 0, .unit = "pixel_formats", .name = "ARGB1555", .help = "RGB with simple transparency, each pixel can be transparent or opaque", .default_val = {.i64 = KDT_PixelFormat_ARGB1555}},
    {.offset = 0, .unit = "pixel_formats", .name = "ARGB4444", .help = "Full ARGB", .default_val = {.i64 = KDT_PixelFormat_ARGB4444}},
    {.name = NULL}
};

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

static bool string_ends_lowercase_with(const char* str, const char* substr) {
    size_t str_len = strlen(str);
    size_t substr_len = strlen(substr);

    if (substr_len > str_len) {
        return false;
    }

    str += str_len - substr_len;

    for (size_t i = 0; i < substr_len; i++) {
        char c1 = tolower(str[i]);
        char c2 = tolower(substr[i]);

        if (c1 != c2) return false;
    }

    return true;
}


static ssize_t escape_argument(char* result, const char* argument, size_t max_length) {
    size_t argument_length = strlen(argument);
    bool is_escaped = true;
    size_t count = 0;

    for (size_t i = 0; i < argument_length; i++) {
        if (isspace(argument[i]) || argument[i] == '"') {
            is_escaped = false;
            break;
        }
    }

    if (is_escaped) {
        if (argument_length >= max_length) return -1;
        memcpy(result, argument, argument_length);
        return argument_length;
    }

    APPEND_CHAR('"');

    size_t index = 0;
    while (index < argument_length) {
        char c = argument[index++];

        if (c != '\\') {
            if (c == '"') APPEND_CHAR('\\');
            APPEND_CHAR(c);
            continue;
        }

        size_t escapes = 1;
        while (index < argument_length && argument[index] == '\\') {
            index++;
            escapes++;
        }

        if (index == argument_length)
            escapes *= 2;
        else if (argument[index] == '"')
            escapes = (escapes * 2) + 1;

        for (size_t i = 0; i < escapes; i++) {
            APPEND_CHAR('\\');
        }

        if (argument[index] == '"') {
            APPEND_CHAR('"');
            index++;
        }
    }

    APPEND_CHAR('"');

    return count;
}

static char* pack_arguments(const char* exe, ...) {
    size_t exe_length = strlen(exe);
    size_t cmd_length = exe_length * 2;

    va_list arguments;

    va_start(arguments, exe);
    while (true) {
        const char* argument = va_arg(arguments, const char*);
        if (!argument) break;

        cmd_length += (strlen(argument) + 2) * 2;
    }

    char* cmd = malloc(cmd_length + 1);
    assert(cmd);

    memcpy(cmd, exe, exe_length);

    char* result = cmd + exe_length;
    cmd_length -= result - cmd;

    va_start(arguments, exe);
    while (true) {
        const char* argument = va_arg(arguments, const char*);
        if (!argument) break;

        if (argument[0] == '\r') {
            argument++;
            size_t len = strlen(argument);
            if (len > cmd_length) {
                free(cmd);
                return NULL;
            }
            memcpy(result, argument, len);

            cmd_length -= len;
            result += len;
            continue;
        }

        ssize_t count = escape_argument(result, argument, cmd_length);
        if (count < 0) {
            free(cmd);
            return NULL;
        }

        cmd_length -= count;
        result += count;
    }

    *result = '\0';
    return cmd;
}

static bool run_magick(const char* executable_name, const char* in_filename, const char* out_filename, int entry_index) {
    bool executable_name_provided = executable_name != NULL && executable_name[0] != '\0';
    const char* executable_name_alt = NULL;
    FILE* hnd;

    if (!executable_name_provided) {
        if ((hnd = fopen(MAGICK_ImageMagick_CONVERT, "rb"))) {
            executable_name = MAGICK_ImageMagick_CONVERT;
            fclose(hnd);
        }
        if ((hnd = fopen(MAGICK_ImageMagick_EXECUTABLE, "rb"))) {
            executable_name_alt = MAGICK_ImageMagick_EXECUTABLE;
            fclose(hnd);
        }
    }

    if (!executable_name && !executable_name_alt) {
        goto L_missing_executable;
    }

    if (!executable_name && executable_name_alt) {
        executable_name = executable_name_alt;
        executable_name_alt = NULL;
    }

    char version[32];
    char* command;

L_check_executable:
    command = pack_arguments(executable_name, "\r --version", NULL);
    if (!command) {
        goto L_pack_arguments_failed;
    }

    hnd = popen(command, "r");
    free(command);

    // step 1: check version
    size_t i = 0;
    char c;

    while ((c = fgetc(hnd)) != EOF) {
        if (i < sizeof(version)) {
            version[i++] = c;
        }
    }
    pclose(hnd);

    version[sizeof(version) - 1] = '\0';
    if (strstr(version, "ImageMagick") == NULL) {
        // missing, try another executable name
        if (executable_name_alt && executable_name != executable_name_alt) {
            executable_name = executable_name_alt;
            goto L_check_executable;
        }
        goto L_missing_executable;
    }

    // step 2: do conversion, use "out_filename" temporarily
    printf("Decoding '%s' with ImageMagick...\n", in_filename);

    char entry_chooser[16];

    if (entry_index >= 0 && entry_index <= 256) {
        snprintf(entry_chooser, sizeof(entry_chooser), "[%i]", entry_index);
        command = pack_arguments(executable_name, "\r ", in_filename, entry_chooser, "\r bmp:", out_filename, NULL);
    } else {
        command = pack_arguments(executable_name, "\r ", in_filename, "\r bmp:", out_filename, NULL);
    }

    if (!command) {
        goto L_pack_arguments_failed;
    }

    //
    // run command like:
    //          imagemagick.exe orig.ico[123] temp.bmp
    //          imagemagick.exe orig.dds temp.bmp
    //
    int exit_code = system(command);

    if (exit_code != 0) {
        printf("Failed to decode '%s' with ImageMagick. Exit code was %i and command was:\n%s\n", in_filename, exit_code, command);
        free(command);
        return false;
    }

    free(command);
    return true;

L_missing_executable:
    if (executable_name_provided) {
        printf(
            "run_magick() failed to execute magick, executable path was: %s\n", executable_name
        );
    } else {
        printf(
            "run_magick() failed, missing magick executable, place the file '%s' or '%s' and try again\n",
            MAGICK_ImageMagick_CONVERT, MAGICK_ImageMagick_EXECUTABLE
        );
    }
    return false;

L_pack_arguments_failed:
    printf("run_magick() call to pack_arguments() failed\n");
    return false;
}


static inline void apply_scale_factor(double scl_fctr, int32_t wdth, int32_t hght, int32_t* out_wdth, int32_t* out_hght) {
    // the calculated dimmensions must be integer
    double width_f = wdth / scl_fctr;
    double height_f = hght / scl_fctr;

    double integer_width, integer_height;
    modf(width_f, &integer_width);
    modf(height_f, &integer_height);

    int32_t width = (int32_t)integer_width;
    int32_t height = (int32_t)integer_height;

    if (width & 1) {
        width--;
    }
    if (height & 1) {
        height--;
    }

    assert(width > 0);
    assert(height > 0);

    if (width != (int32_t)integer_width || height != (int32_t)integer_height) {
        printf("warning: the encoded image size has odd numbers or can not be downscaled using integers:\n");
        printf("rounding %gx%g to %ix%i.\n", width_f, height_f, width, height);
    }

    *out_wdth = width;
    *out_hght = height;
}

static inline float string_to_float(const char* str) {
    double value1 = atof(str);
    double value2 = strtod(str, NULL);

    double value = value1 > value2 ? value1 : value2;
    return (float)value;
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
    const char* program = argc > 0 ? argv[0] : "kdt_enc";

    size_t idx = 0;
    for (size_t i = 0; program[i] != '\0'; i++) {
        if (program[i] == '/' || program[i] == '\\') {
            idx = i + 1;
        }
    }
    program = program + idx;

    const AVClass* sws_avclass = sws_get_class();
    const AVOption* sws_options = sws_avclass->option;

    const char* magick_exe1 = MAGICK_ImageMagick_CONVERT;
    const char* magick_exe2 = MAGICK_ImageMagick_EXECUTABLE;

    const int sdimmen_min = 2 * MAX_DIMMEN;
    const int sdimmen_max = MAX_SLICE_BLOCKS * MAX_DIMMEN;

    printf("KDT encoder v0.4 by kapodamy\n");
    printf("Encodes image files (like png, jpg, bmp, ico, etc.) into KDT texture format, ");
    printf("also allows features like PVR-VQ, twiddle, and frame wrapping for non power-of-two dimmensions.\n");
    printf("To read ico and dds image files the ImageMagick binary executable must be present with the filename \"%s\" or \"%s\".\n", magick_exe1, magick_exe2);
    printf("Mipmapping is not implemented\n");
    printf("\n");
    printf("Usage: %s [options...] <input image file> <output image file>\n", program);
    printf("\n");
    printf("Common options:\n");
    printf(" -q, --quality <steps>                          Vector quantization steps, higher values allows better quality. Default: 500\n");
    printf(" -d, --dither-algorithm <dither algorithm>      Dither algortihm used for pixel format conversion. Default: auto\n");
    printf(" -s, --scale-algorithm <scale algorithm>        Used only if the image needs downscaling. Default: lanczos\n");
    printf(" -g, --rgb565                                   Use RGB565 pixel format instead of YUV422, this can reduce artifacts but produces color banding.\n");
    printf(" -v, --vq                                       Compress texture using vector quantization, lossy format but saves a lot of VRAM.\n");
    printf(" -t, --no-twiddled                              Disable pixel twiddling, this decreases rendering performance.\n");
    printf(" -z, --lzss                                     Compress the file with LZSS, only improves read and parse times. Does not affect the VRAM usage\n");
    printf("Slice options:\n");
    printf(" -e, --uper-slice                               Store high-resolution images (bigger than %ix%i) as encoded chunks, this reduces the amount of downscaling needed but requires more VRAM.\n", MAX_DIMMEN, MAX_DIMMEN);
    printf(" -b, --uper-slice-max-blocks <integer>          Maximum slice frame dimmension, used with '--uper-slice'. default=%i minimum=%i(%ix%i) maximum=%i(%ix%i)\n", MAX_SLICE_BLOCKS, 2, sdimmen_min, sdimmen_min, MAX_SLICE_BLOCKS, sdimmen_max, sdimmen_max);
    printf(" -r, --sub-slice                                Allow slice right-bottom image borders in order to add less frame padding, this reduces VRAM usage. Can be used with '--uper-slice' option\n");
    printf(" -o, --opacity-slice                            Ignores right and bottom 'transparency padding' if exists, this can reduce VRAM usage on power-of-two spritesheets.\n");
    printf("Other VQ options (not compatible with slice):\n");
    printf(" -m, --force-vq-on-small                        Forces vector quantization on textures smaller than 64x64, waste of space.\n");
    printf(" -u, --force-square-vq                          Always wrap texture with square dimmensions (example 128x128), applies only if vector quantization is needed.\n");
    printf("Advanced options:\n");
    printf(" -p, --downscale-procedure <type>               Specifies how images bigger than %ix%i should be downscaled. Default: CLAMP\n", MAX_DIMMEN, MAX_DIMMEN);
    printf(" -x, --pixel-format <pixel format>              Output pixel format. Default: AUTO\n");
    printf("Resize options:\n");
    printf(" -f, --scale-factor <number>                    Ignores the downscale procedure and resize with the specified amount of times. Default: 0.0\n");
    printf(" -l, --scale-factor-limit <integer>             Do not resize if the image size is less than the specified dimmension, used with '--scale-factor'. Default: 16\n");
    printf(" -a, --max-dimmen <integer>                     Resize images to specific dimension while keeping the aspect ratio, must be an even number. Applies only if the width and/or height exceeds the dimmension, can not be used with '--scale-factor' option. Default: 0 (disabled)\n");
    printf("ICO/DDS options:\n");
    printf(" -e, --image-magick-exec <path>                 Path to ImageMagick executable, only needed for decoding dds and ico files. Default: %s or %s\n", magick_exe1, magick_exe2);
    printf("\n");
    printf("\n");
    printf("Dither algorithms:\n");
    print_avoption_values(sws_options, "sws_dither");
    printf("\n");
    printf("Scale algorithms:\n");
    print_avoption_values(sws_options, "sws_flags");
    printf("\n");
    printf("Downscale procedures:\n");
    print_avoption_values(advanced_options, "downscale_procedures");
    printf("Note: all procedures are relaxed when '--uper-slice' option is used.\n");
    printf("\n");
    printf("Pixel formats (all of them are 16bit):\n");
    print_avoption_values(advanced_options, "pixel_formats");
    printf("\n");
    printf("Notes:\n");
    printf("* For faster VQ compression, use tiny steps like 1, 2, 5.\n");
    printf("* The PVR VQ (vector quantization) can produce files 4 times smaller and saves alot of VRAM (the available amount of VRAM is 8MiB).\n");
    printf("* The encoder automatically chooses the output pixel format by checking the amount of transparency on the image.\n");
    printf("* The piority order (to automatically choose a pixel format) is YU422/RGB565 -> ARGB1555 ->ARGB4444.\n");
    printf("* No power-of-two images are wrapped into power-of-two dimensions, this allow render using the original size.\n");
    printf("* If the image is downscaled, the scale factor is stored in the KDT texture file to allow rendering with the original size.\n");
    printf("* The maximum image size is 8192x8192 otherwise will be rejected.\n");
    printf("* The maximum texture size supported by the PowerVR GPU is %ix%i, thats why downscale procedures are needed.\n", MAX_DIMMEN, MAX_DIMMEN);
    printf("* The VRAM needed is always the output file size minus %zi bytes.\n", sizeof(KDT));
    printf("* For ico files, always the 128x128 icon image is used otherwise the biggest resolution one.\n");
    printf("* There some edge-cases where '--sub-slice' does not take effect, for example an 640x788 resolution.\n");
    printf("* The '--uper-slice-max-blocks' is limited to %i because increases VRAM usage, for example a 3072x3072 VQ-compressed image will require 2.74MiB of VRAM.\n", MAX_SLICE_BLOCKS);
    printf("* Sliced textures (with '--uper-slice' and/or '--sub-slice' option) can introduce line artifacts, a visible gap between texture chunks.\n");
    printf("* LZSS file compression will be disabled if the compression ratio is less than %g%%.\n", MIN_COMPRESSION_RATIO * 100.0);
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
        } else if (STRING_EQUALS(option_name, "q") || STRING_EQUALS(option_name, "quality")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->quality = atoi(option_value);
            if (values->quality < 1) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "d") || STRING_EQUALS(option_name, "dither-algorithm")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            if (has_avoption_value(sws_options, "sws_dither", option_value, &avoption_value))
                values->dither = option_value;
            else
                goto L_invalid_value;
        } else if (STRING_EQUALS(option_name, "s") || STRING_EQUALS(option_name, "scale-algorithm")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            if (has_avoption_value(sws_options, "sws_flags", option_value, &avoption_value))
                values->scale_algorithm = (int32_t)avoption_value;
            else
                goto L_invalid_value;
        } else if (STRING_EQUALS(option_name, "g") || STRING_EQUALS(option_name, "rgb565")) {
            values->rgb565 = true;
        } else if (STRING_EQUALS(option_name, "v") || STRING_EQUALS(option_name, "vq")) {
            values->vq = true;
        } else if (STRING_EQUALS(option_name, "t") || STRING_EQUALS(option_name, "no-twiddled")) {
            values->twiddle = false;
        } else if (STRING_EQUALS(option_name, "z") || STRING_EQUALS(option_name, "lzss")) {
            values->lzss = true;
        } else if (STRING_EQUALS(option_name, "e") || STRING_EQUALS(option_name, "uper-slice")) {
            values->uper_slice = true;
        } else if (STRING_EQUALS(option_name, "b") || STRING_EQUALS(option_name, "uper-slice-max-blocks")) {
            i++;
            values->uper_slice_max_blocks = atoi(option_value);
            if (values->uper_slice_max_blocks < 2 || values->uper_slice_max_blocks > MAX_SLICE_BLOCKS) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "r") || STRING_EQUALS(option_name, "sub-slice")) {
            values->sub_slice = true;
        } else if (STRING_EQUALS(option_name, "o") || STRING_EQUALS(option_name, "opacity-slice")) {
            values->opacity_slice = true;
        } else if (STRING_EQUALS(option_name, "m") || STRING_EQUALS(option_name, "force-vq-on-small")) {
            values->force_vq_on_small = true;
        } else if (STRING_EQUALS(option_name, "u") || STRING_EQUALS(option_name, "force-square-vq")) {
            values->force_square_vq = true;
        } else if (STRING_EQUALS(option_name, "p") || STRING_EQUALS(option_name, "downscale-procedure")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            if (has_avoption_value(advanced_options, "downscale_procedures", option_value, &avoption_value))
                values->downscale_procedure = (int32_t)avoption_value;
            else
                goto L_invalid_value;
        } else if (STRING_EQUALS(option_name, "x") || STRING_EQUALS(option_name, "pixel-format")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            if (has_avoption_value(advanced_options, "pixel_formats", option_value, &avoption_value))
                values->pixel_format = (int32_t)avoption_value;
            else
                goto L_invalid_value;
        } else if (STRING_EQUALS(option_name, "f") || STRING_EQUALS(option_name, "scale-factor")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->scale_factor = string_to_float(option_value);
            if (values->scale_factor < 1.0f || values->scale_factor > 16.0f) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "l") || STRING_EQUALS(option_name, "scale-factor-limit")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->scale_factor_limit = atoi(option_value);
            if (values->scale_factor_limit < 1 || values->scale_factor_limit > MAX_DIMMEN) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "a") || STRING_EQUALS(option_name, "max-dimmen")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->max_dimmen = atoi(option_value);
            if (values->max_dimmen > (MAX_DIMMEN * MAX_SLICE_BLOCKS)) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "e") || STRING_EQUALS(option_name, "image-magick-exec")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->magick_exe = option_value;
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

    if (values->force_square_vq && (values->sub_slice || values->uper_slice)) {
        printf("INFO: the option 'force-square-vq' is not compatible with sliced textures.\n");
        values->force_square_vq = false;
    }
    if (values->force_vq_on_small && (values->sub_slice || values->uper_slice)) {
        printf("INFO: the option 'force-vq-on-small' is not compatible with sliced textures.\n");
        values->force_vq_on_small = false;
    }

    if (values->scale_factor > 0.0f && values->max_dimmen > 0) {
        printf("The option 'max-dimmen' can not be used with 'scale-factor' option.\n");
        exit(1);
        return;
    }

    int32_t max_slice_dimmen = MAX_DIMMEN * (values->uper_slice ? values->uper_slice_max_blocks : 1);
    if (values->max_dimmen > max_slice_dimmen) {
        printf("The option 'max-dimmen' can not bigger than %ix%i.\n", max_slice_dimmen, max_slice_dimmen);
        exit(1);
        return;
    }

    if (values->max_dimmen > 0 && values->max_dimmen != ((values->max_dimmen / 2) * 2)) {
        printf(
            "The 'max-dimmen' option must be an even number, try %i or %i.\n",
            values->max_dimmen - 1, values->max_dimmen + 1
        );
        exit(1);
        return;
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
        out_filename2[dot_idx + 1] = 'k';
        out_filename2[dot_idx + 2] = 'd';
        out_filename2[dot_idx + 3] = 't';
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


static KDT_PixelFormat alpha_detector(RGBA8* pixels, int32_t width, int32_t height, KDT_PixelFormat default_pixfmt) {
    size_t total_pixels = width * height;
    uint8_t alpha_levels[UINT8_MAX + 1];

    // calc alpha levels
    memset(alpha_levels, 0, UINT8_MAX + 1);
    for (size_t i = 0; i < total_pixels; i++) {
        uint8_t alpha = pixels[i].a;
        uint8_t level = alpha_levels[alpha];
        if (level < UINT8_MAX) {
            alpha_levels[alpha] = level + 1;
        }
    }

    // check pixels are opaque
    if (alpha_levels[UINT8_MAX] > 0) {
        bool is_opaque = true;
        for (size_t i = 0; i < UINT8_MAX; i++) {
            if (alpha_levels[i] != 0) {
                is_opaque = false;
                break;
            }
        }
        if (is_opaque) {
            return default_pixfmt;
        }
    }

    // check if ARGB1555 can be used
    if (alpha_levels[0] > 0 && alpha_levels[UINT8_MAX] > 0) {
        bool simple_alpha = true;
        for (size_t i = 1; i < UINT8_MAX; i++) {
            if (alpha_levels[i] != 0) {
                simple_alpha = false;
                break;
            }
        }
        if (simple_alpha) {
            return KDT_PixelFormat_ARGB1555;
        }
    }

    // use ARGB4444, all levels are present
    return KDT_PixelFormat_ARGB4444;
}

static double downscaler_calc(DownScaler_Prc prc, int32_t width, int32_t height, int32_t slice_dimmen, int32_t* out_width, int32_t* out_height) {
    const ScaleLevel* scaling_levels;

    // calc scale from slice dimmen (if applicable)
    int32_t slice_level;
    if (slice_dimmen > 1)
        slice_level = (slice_dimmen / MAX_DIMMEN) - 1;
    else
        slice_level = 0;

    assert(slice_level >= 0 && slice_level < MAX_SLICE_BLOCKS);

    // pick level
    switch (prc) {
        case DownScaler_Prc_CLAMP:
            scaling_levels = SCALING_LEVELS_CLAMP[slice_level];
            break;
        case DownScaler_Prc_MEDIUM:
            scaling_levels = SCALING_LEVELS_MEDIUM[slice_level];
            break;
        case DownScaler_Prc_HARD:
            scaling_levels = SCALING_LEVELS_HARD[slice_level];
            break;
        default:
            assert(false);
            break;
    }

    // pick scale factor and frame dimmen
    int32_t src_dimmen = width > height ? width : height;
    double scale_factor = 1.0;
    for (size_t i = 0; i < SCALING_LEVELS_TOTAL; i++) {
        scale_factor = scaling_levels[i].scale_factor;

        if (scaling_levels[i].max_dimmen >= src_dimmen) {
            break;
        }
    }

    // check if maximum dimmension exceeds the 1024x1024 size
    double dst_dimmen = src_dimmen / scale_factor;
    if (slice_dimmen > 0) {
        if (dst_dimmen > slice_dimmen) {
            scale_factor = src_dimmen / (double)slice_dimmen;
        }
    } else if (dst_dimmen > MAX_DIMMEN) {
        scale_factor = src_dimmen / (double)MAX_DIMMEN;
    }

    if (out_width && out_height) {
        apply_scale_factor(scale_factor, width, height, out_width, out_height);
    }

    return scale_factor;
}

static int32_t pow2_dimmen_calc(int32_t dimmen) {
    assert(dimmen > 0);
    for (int32_t d = 1; d <= MAX_DIMMEN; d *= 2) {
        if (d >= dimmen) {
            return d;
        }
    }
    assert(false);
    return MAX_DIMMEN;
}

static int32_t pow2_dimmen_calc_for_subslice(int32_t dimmen, int32_t* cutoff) {
    assert(dimmen <= MAX_DIMMEN);
    assert(cutoff != NULL);

    int32_t d = pow2_dimmen_calc(dimmen);

    int32_t prev_d = d / 2;
    if (prev_d < MIN_SUBSLICE_DIMMEN) goto L_not_applicable;

    int32_t remaining = dimmen - prev_d;
    if (remaining < MIN_SUBSLICE_DIMMEN) goto L_not_applicable;

    int32_t subslice_d = pow2_dimmen_calc(remaining);

    subslice_d += prev_d;
    if (subslice_d >= d) goto L_not_applicable;

    *cutoff = prev_d;
    return subslice_d;

L_not_applicable:
    *cutoff = -1;
    return d;
}

static int32_t pow2_dimmen_calc_for_upperslice(int32_t dimmen, int32_t slice_dimmen, int32_t* subslice_cutoff) {
    assert(slice_dimmen > MAX_DIMMEN && dimmen <= slice_dimmen);
    int32_t padding = 0;
    while (dimmen > MAX_DIMMEN) {
        padding += MAX_DIMMEN;
        dimmen -= MAX_DIMMEN;
    }
    int32_t d;
    if (subslice_cutoff != NULL) {
        int32_t cutoff;
        d = pow2_dimmen_calc_for_subslice(dimmen, &cutoff);
        if (cutoff > 0) {
            cutoff = padding + cutoff;
        }
        *subslice_cutoff = cutoff;
    } else {
        d = pow2_dimmen_calc(dimmen);
    }
    d += padding;
    return d;
}

static uint16_t* convert_to_argb(RGBA8* pixels, int32_t width, int32_t height, bool single_bit_alpha) {
    // resuse pixels buffer
    uint16_t* converted = (uint16_t*)pixels;
    size_t total = width * height;

    if (single_bit_alpha) {
        for (size_t i = 0; i < total; i++) {
            converted[i] = rgba8_to_argb1555(pixels[i]);
        }
    } else {
        for (size_t i = 0; i < total; i++) {
            converted[i] = rgba8_to_argb4444(pixels[i]);
        }
    }

    return converted;
}

#ifndef USE_FFMPEG_SWS_YUV_CONVERSOR
static uint16_t* convert_to_uyvy(RGBA8* pixels, int32_t width, int32_t height) {
    // resuse pixels buffer
    uint16_t* converted = (uint16_t*)pixels;
    RGBA8* pixels_end = pixels + (width * height);

    while (pixels < pixels_end) {
        RGB24 pixel1 = rgba8_to_rgb24(*pixels++);
        RGB24 pixel2 = rgba8_to_rgb24(*pixels++);

        YUV422Pixels packed = rgb24_to_yuv422(pixel1, pixel2);

        *converted++ = packed.pixel1;
        *converted++ = packed.pixel2;
    }

    return converted;
}
#endif

static inline void wrap_frame(uint16_t* buffer, uint16_t* canvas, int32_t o_w, int32_t o_h, int32_t f_w, int32_t f_h, bool is_yuv422) {
    if (o_w == f_w && o_h == f_h) {
        // nothing to do
        memcpy(canvas, buffer, f_w * f_h * sizeof(uint16_t));
        return;
    }

    size_t orig_scanline = o_w * sizeof(uint16_t);
    uint16_t* canvas_ptr = canvas;
    uint16_t* buffer_ptr = buffer;

    for (int32_t i = 0; i < o_h; i++) {
        memcpy(canvas_ptr, buffer_ptr, orig_scanline);
        buffer_ptr += o_w;
        canvas_ptr += f_w;
    }

    //
    // use last pixel value as horizontal padding (does not work with YUV)
    //

    int32_t padding_height = f_h - o_h;
    if (padding_height > 0 && o_h > 1) {
        buffer_ptr = buffer + (o_w * (o_h - 1));
        canvas_ptr = canvas + (f_w * o_h);

        for (int32_t i = 0; i < padding_height; i++) {
            // copy last stride
            memcpy(canvas_ptr, buffer_ptr, orig_scanline);
            canvas_ptr += f_w;
        }
    }

    int32_t padding_width = f_w - o_w;
    if (padding_width > 0 && o_w > 0) {
        canvas_ptr = canvas + o_w;

        if (is_yuv422) {
            // the width must be an even number
            assert((~padding_width & (int32_t)1) == 1);

            for (int32_t i = 0; i < f_h; i++) {
                // pick last yuv pixel and duplicate it
                RGB24Pixels packed_rgb24 = yuv422_to_rgb24_packed(*(canvas_ptr - 2), *(canvas_ptr - 1));
                YUV422Pixels packed_yuv422 = rgb24_to_yuv422(packed_rgb24.pixel2, packed_rgb24.pixel2);

                for (int32_t j = 0; j < padding_width; j += 2) {
                    *canvas_ptr++ = packed_yuv422.pixel1;
                    *canvas_ptr++ = packed_yuv422.pixel2;
                }
                canvas_ptr += o_w;
            }
        } else {
            for (int32_t i = 0; i < f_h; i++) {
                // pick last pixel
                uint16_t pixel = *(canvas_ptr - 1);

                for (int32_t j = 0; j < padding_width; j++) {
                    *canvas_ptr++ = pixel;
                }
                canvas_ptr += o_w;
            }
        }
    }
}

static inline bool sws_run(KDT_PixelFormat pixfmt, RGBA8* i, void* b, int32_t ow, int32_t oh, int32_t dw, int32_t dh, Arguments* v) {
    enum AVPixelFormat dst_pixfmt;
    switch (pixfmt) {
        case KDT_PixelFormat_YUV422:
#ifdef USE_FFMPEG_SWS_YUV_CONVERSOR
            dst_pixfmt = AV_PIX_FMT_UYVY422;
#else
            dst_pixfmt = AV_PIX_FMT_RGBA;
#endif
            break;
        case KDT_PixelFormat_RGB565:
            dst_pixfmt = AV_PIX_FMT_RGB565;
            break;
        case KDT_PixelFormat_ARGB1555:
        case KDT_PixelFormat_ARGB4444:
            dst_pixfmt = AV_PIX_FMT_RGBA;
            break;
        default:
            assert(false);
            break;
    }

    struct SwsContext* sws_ctx = sws_getContext(
        ow, oh, AV_PIX_FMT_RGBA,
        dw, dh, dst_pixfmt,
        v->scale_algorithm, NULL, NULL, NULL
    );

    if (sws_ctx == NULL) {
        printf("call to sws_getContext() failed\n");
        return false;
    }

    if (v->dither != NULL) {
        int ret = av_opt_set(sws_ctx, "sws_dither", v->dither, 0x00);
        if (ret != 0) {
            printf("failed to set \"sws_dither\" value:%s\n", av_err2str(ret));
            sws_freeContext(sws_ctx);
            return false;
        }
    }

    const uint8_t* const srcSlice[4] = {(const uint8_t*)i, NULL, NULL, NULL};
    int srcStride[4];
    av_image_fill_linesizes(srcStride, AV_PIX_FMT_RGBA, ow);

    uint8_t* const dstSlice[] = {b, NULL, NULL, NULL};
    int dstStride[4];
    av_image_fill_linesizes(dstStride, dst_pixfmt, dw);

    sws_scale(sws_ctx, srcSlice, srcStride, 0, oh, dstSlice, dstStride);

    sws_freeContext(sws_ctx);
    return true;
}

static void encode(Arguments* values, KDT_PixelFormat pixfmt, uint16_t* src, uint16_t* buffer, int32_t width, int32_t height, size_t stride) {
    uint16_t* src_ptr = src;
    size_t canvas_stride = (size_t)width * sizeof(uint16_t);

    if (!values->vq && !values->twiddle) {
        // nothing to encode
        uint16_t* buffer_ptr = buffer;
        for (int32_t i = 0; i < height; i++) {
            memcpy(buffer_ptr, src_ptr, canvas_stride);
            buffer_ptr += width;
            src_ptr += stride;
        }
        // dump_file(buffer, width, height);
        return;
    }

    size_t frame_size = width * height * sizeof(uint16_t);
    uint16_t* canvas = malloc(frame_size);
    uint16_t* canvas_ptr = canvas;

    assert(canvas);

    for (int32_t i = 0; i < height; i++) {
        memcpy(canvas_ptr, src_ptr, canvas_stride);
        canvas_ptr += width;
        src_ptr += stride;
    }

    // dump_file(canvas, width, height);

    // twiddle (if required)
    if (values->vq || values->twiddle) {
        MakeTwiddled16(canvas, buffer, width, height);
        memcpy(canvas, buffer, frame_size);
    }

    // vq compression (if required)
    PVR_VQ* buffer_vq = (PVR_VQ*)buffer;
    if (values->vq) {
        uint16_t* codebook;
        uint8_t* indexes;
        VQ* vq = vq_init(width, height, 4, values->quality, 256, pixfmt);

        vq_do(vq, canvas, &codebook, &indexes);

        memcpy(buffer_vq->codebook, codebook, sizeof(buffer_vq->codebook));
        memcpy(buffer_vq->indexes, indexes, frame_size / 8);

        vq_destroy(vq);
    }

    // detwiddle vq (if required)
    if (values->vq && !values->twiddle) {
        //
        // For some reason the "Enhanced LBG Algorithm Based" used for vq compression
        // does not work with un-twiddled frames, generates a double mirroed frame.
        // if twidding is disabled untwiddle the vq codebook and indexes
        //
        void* tmp_buffer = canvas;

        for (size_t i = 0; i < 256; i++) {
            uint16_t* texels = buffer_vq->codebook[i].texels;

#ifdef UNTWIDDLE_CODEBOOK_WITH_TWIDDLER_FUNCTION
            memcpy(tmp_buffer, texels, sizeof(uint16_t) * 4);
            UnMakeTwiddled16(tmp_buffer, texels, 2, 2);
#else
            uint16_t texel = texels[1];
            texels[1] = texels[2];
            texels[2] = texel;
#endif
        }

        memcpy(tmp_buffer, buffer_vq->indexes, frame_size / 8);
        UnMakeTwiddled8(tmp_buffer, buffer_vq->indexes, width / 2, height / 2);
    }

    free(canvas);
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

static int32_t round_up_to(int32_t nro, int32_t multiple) {
    int32_t rmdr = nro % multiple;
    if (rmdr == 0) return nro;
    return nro + (multiple - rmdr);
}

static void find_transparent_padding(RGBA8* image, int32_t o_w, int32_t o_h, int32_t* p_w, int32_t* p_h, DownScaler_Prc prc, float s, int32_t m) {
    int32_t padding_x = 0;
    int32_t padding_y = 0;

    for (int32_t y = o_h - 1; y >= 0; y--) {
        for (int32_t x = o_w - 1; x >= 0; x--) {
            RGBA8 texel = image[x + (y * o_w)];
            if (texel.a > 0) {
                if (x > padding_x) {
                    padding_x = x;
                }
                if (y > padding_y) {
                    padding_y = y;
                }
            }
        }
    }

    if (padding_x == 0 || padding_y == 0) {
        *p_w = padding_x;
        *p_h = padding_y;
        return;
    }

    //
    // if the image must be downscaled the encoded dimmensions should be multiple of 2
    // increase padding until this condition is meet
    //

    double scale;
    if (m > 0 && (padding_x > m || padding_y > m)) {
        scale = padding_x > padding_y ? padding_x : padding_y;
        scale = m / scale;

        int32_t tmp_width, tmp_height;
        if (padding_x > padding_y) {
            tmp_width = m;
            tmp_height = (int32_t)(padding_y * scale);
        } else {
            tmp_width = (int32_t)(padding_x * scale);
            tmp_height = m;
        }

        scale = downscaler_calc(prc, tmp_width, tmp_height, MAX_DIMMEN * MAX_SLICE_BLOCKS, NULL, NULL);
    } else if (s > 0.0f) {
        scale = s;
    } else {
        scale = downscaler_calc(prc, padding_x, padding_y, MAX_DIMMEN * MAX_SLICE_BLOCKS, NULL, NULL);
    }

    if (padding_x < o_w) {
        padding_x = round_up_to(padding_x, (int32_t)(scale * 2.0));
        if (padding_x > o_w) padding_x = o_w;
    }
    if (padding_y < o_h) {
        padding_y = round_up_to(padding_y, (int32_t)(scale * 2.0));
        if (padding_y > o_h) padding_y = o_h;
    }

    if (padding_x < o_w) assert((o_w - padding_x) <= UINT16_MAX);
    if (padding_y < o_h) assert((o_h - padding_y) <= UINT16_MAX);

    *p_w = padding_x;
    *p_h = padding_y;
}


int main(int argc, char** argv) {
    bool success = true;
    bool unsupported = false;
    bool magick_used = false;
    uint16_t* canvas = NULL;
    void* buffer = NULL;
    SourceHandle* srchnd = NULL;
    FILE* kdt_file = NULL;
    FFGraph* ffgraph = NULL;

    Arguments values = (Arguments){
        .input_filename = NULL,
        .output_filename = NULL,
        .rgb565 = false,
        .vq = false,
        .twiddle = true,
        .force_vq_on_small = false,
        .force_square_vq = false,
        .uper_slice = false,
        .sub_slice = false,
        .opacity_slice = false,
        .lzss = false,
        .scale_factor = 0,
        .scale_factor_limit = 16,
        .max_dimmen = 0,
        .quality = 500,
        .scale_algorithm = SWS_LANCZOS,
        .uper_slice_max_blocks = MAX_SLICE_BLOCKS,
        .dither = NULL,
        .downscale_procedure = DownScaler_Prc_CLAMP,
        .pixel_format = KDT_PixelFormat_NONE,
        .magick_exe = NULL
    };

    parse_arguments(argc, argv, sws_get_class()->option, &values);
    assert(values.input_filename && values.output_filename);

    bool is_dds = string_ends_lowercase_with(values.input_filename, ".dds");
    bool is_ico = string_ends_lowercase_with(values.input_filename, ".ico");

    if (is_dds || is_ico) {
        int ico_entry_index = -1;

        if (is_ico) {
            // ICO files containts "mipmaps" pick one
            FILE* ico_file = fopen(values.input_filename, "rb");
            assert(ico_file);

            ICO_header ico_header;
            fread(&ico_header, sizeof(ICO_header), 1, ico_file);

            uint16_t ico_max_dimmen = 0;
            int ico_max_dimmen_index = -1;
            ICO_Entry ico_entry;
            for (int i = 0; i < ico_header.images; i++) {
                memset(&ico_entry, 0x00, sizeof(ICO_Entry));
                fread(&ico_entry, sizeof(ICO_Entry), 1, ico_file);

                if (ico_entry.width >= ICO_DIMMEN || ico_entry.height >= ICO_DIMMEN) {
                    ico_entry_index = i;
                    break;
                }
                if (ico_entry.width > ico_max_dimmen || ico_entry.height > ico_max_dimmen) {
                    ico_max_dimmen_index = i;
                }
            }
            fclose(ico_file);

            if (ico_header.images > 1 && ico_entry_index < 0 && ico_max_dimmen_index >= 0) {
                ico_entry_index = ico_max_dimmen_index;
            }
        }

        magick_used = run_magick(values.magick_exe, values.input_filename, values.output_filename, ico_entry_index);
        if (!magick_used) {
            success = false;
            unsupported = true;
            goto L_return;
        }
    }

    // NOTE: "load_in_ram" does not work with FFGraph
    srchnd = filehandle_init1(magick_used ? values.output_filename : values.input_filename, false);
    assert(srchnd != NULL);

    videoconverter_set_custom(true, AV_PIX_FMT_RGBA, 0x00, NULL, NULL);
    ffgraph = ffgraph_init(srchnd);
    if (!ffgraph) {
        printf("Unsupported image file: %s\n", values.input_filename);
        success = false;
        unsupported = true;
        goto L_return;
    }

    FFGraphInfo info;
    ffgraph_get_streams_info(ffgraph, &info);
    if (!info.video_has_stream) {
        printf("Unsupported image contents: %s\n", values.input_filename);
        success = false;
        unsupported = true;
        goto L_return;
    }
    if (info.video_width > 8192 || info.video_height > 8192) {
        printf("Invalid image size %i%i, the maximum is 8192x8192: %s\n", info.video_width, info.video_height, values.input_filename);
        success = false;
        unsupported = true;
        goto L_return;
    }

    RGBA8* image = NULL;
    int32_t image_size = 0;
    if (ffgraph_read_video_frame(ffgraph, (void**)&image, &image_size) < 0.0) {
        printf("Failed to decode the image file: %s\n", values.input_filename);
        success = false;
        unsupported = true;
        goto L_return;
    }
    if (image_size != (info.video_width * info.video_height * (int32_t)sizeof(RGBA8))) {
        printf("Unexpected decoded the image size: %s\n", values.input_filename);
        success = false;
        unsupported = true;
        goto L_return;
    }

    if (info.video_width <= 64 && info.video_height <= 64) {
        if (values.lzss || values.sub_slice || values.uper_slice) {
            printf("INFO: image size is less than 64x64, some options will be disabled.\n");
        }
        if (values.lzss) {
            printf("INFO: dissabling 'lzss' file compression.\n");
            values.lzss = false;
        }
        if (values.sub_slice) {
            printf("INFO: dissabling 'sub-slice'.\n");
            values.sub_slice = false;
        }
        if (values.uper_slice) {
            printf("INFO: dissabling 'uper-slice'.\n");
            values.uper_slice = false;
        }
    }

    // step 1: pick pixel format
    KDT_PixelFormat pixfmt;
    if (values.pixel_format == KDT_PixelFormat_NONE) {
        KDT_PixelFormat default_pixfmt = values.rgb565 ? KDT_PixelFormat_RGB565 : KDT_PixelFormat_YUV422;
        pixfmt = alpha_detector(image, info.video_width, info.video_height, default_pixfmt);
    } else {
        pixfmt = values.pixel_format;
    }

    int32_t source_width = info.video_width;
    int32_t source_height = info.video_height;

    // step 2: slice opacity (if required)
    if (values.opacity_slice) {
        int32_t tmp_width, tmp_height;
        find_transparent_padding(
            image,
            info.video_width, info.video_height,
            &tmp_width, &tmp_height,
            values.downscale_procedure, values.scale_factor, values.max_dimmen
        );

        if (tmp_width == info.video_width && tmp_height == info.video_height) {
            values.opacity_slice = false;
        } else if (tmp_width == 0 || tmp_height == 0) {
            values.opacity_slice = false;
        } else {
            source_width = tmp_width;
            source_height = tmp_height;
        }

        if (values.opacity_slice) {
            printf(
                "Croping image: from %ix%i to %ix%i (padding removed)\n",
                info.video_width, info.video_height, tmp_width, tmp_height
            );

            if (tmp_width != info.video_width) {
                RGBA8* image_src = image + info.video_width;
                RGBA8* image_dst = image + tmp_width;
                for (int32_t y = 1; y < tmp_height; y++) {
                    memcpy(image_dst, image_src, sizeof(RGBA8) * tmp_width);
                    image_src += info.video_width;
                    image_dst += tmp_width;
                }
            }
        }
    }

    // step 3: check if the image needs to be downscaled (NOTE: YUV422 needs even numbers on image size)
    int32_t max_slice_dimmen = values.uper_slice_max_blocks * MAX_DIMMEN;
    int32_t dst_width, dst_height;
    if (values.max_dimmen > 0 && (source_width > values.max_dimmen || source_height > values.max_dimmen)) {
        double scale = source_width > source_height ? source_width : source_height;
        scale = values.max_dimmen / scale;

        int32_t tmp_width, tmp_height;
        if (source_width > source_height) {
            tmp_width = values.max_dimmen;
            tmp_height = (int32_t)(source_height * scale);
        } else {
            tmp_width = (int32_t)(source_width * scale);
            tmp_height = values.max_dimmen;
        }

        int32_t slice_dimmen = values.uper_slice ? max_slice_dimmen : -1;
        downscaler_calc(
            values.downscale_procedure, tmp_width, tmp_height, slice_dimmen, &dst_width, &dst_height
        );
    } else if (values.scale_factor > 0.0f) {
        apply_scale_factor(values.scale_factor, source_width, source_height, &dst_width, &dst_height);

        if (dst_width < values.scale_factor_limit || dst_height < values.scale_factor_limit) {
            printf(
                "info: ignoring scale factor (x%f). calculated: %ix%i  minimum: %ix%i\n",
                values.scale_factor, dst_width, dst_height, source_width, source_height
            );

            dst_width = source_width;
            dst_height = source_height;

            int32_t dst_width2 = (dst_width & 1) ? (dst_width + 1) : dst_width;
            int32_t dst_height2 = (dst_height & 1) ? (dst_height + 1) : dst_height;

            if (dst_width != dst_width2 || dst_height != dst_height2) {
                printf(
                    "warning: ignoring the scale factor uses the original image size which has odd numbers, rounding up from %ix%i to %ix%i.\n",
                    dst_width, dst_height, dst_width2, dst_height2
                );
                dst_width = dst_width2;
                dst_height = dst_height2;
            }
        }

        if (values.uper_slice) {
            if (dst_width > max_slice_dimmen || dst_height > max_slice_dimmen) {
                printf(
                    "error: can not manually resize the image size %ix%i with the following parameters: scale-factor=%f limit=%i\n",
                    source_width, source_height, values.scale_factor, values.scale_factor_limit
                );
                printf(
                    "error: the image size %ix%i exceeds the maximum slice size of %ix%i (uper-slice-max-blocks is %i)\n",
                    source_width, source_height, max_slice_dimmen, max_slice_dimmen, values.uper_slice_max_blocks
                );
                success = false;
                unsupported = true;
                goto L_return;
            }
        } else if (dst_width > MAX_DIMMEN || dst_height > MAX_DIMMEN) {
            printf(
                "error: can not manually resize the image size %ix%i with the following parameters: scale-factor=%f limit=%i\n",
                source_width, source_height, values.scale_factor, values.scale_factor_limit
            );
            success = false;
            unsupported = true;
            goto L_return;
        }
    } else {
        int32_t slice_dimmen = values.uper_slice ? max_slice_dimmen : -1;
        downscaler_calc(
            values.downscale_procedure, source_width, source_height, slice_dimmen, &dst_width, &dst_height
        );
    }

    // step 4: calculate the frame dimmensions
    int32_t frame_width, frame_height;
    int32_t subslice_cutoff_offset_x, subslice_cutoff_offset_y;
    if (values.uper_slice) {
        if (values.sub_slice) {
            frame_width = pow2_dimmen_calc_for_upperslice(dst_width, max_slice_dimmen, &subslice_cutoff_offset_x);
            frame_height = pow2_dimmen_calc_for_upperslice(dst_height, max_slice_dimmen, &subslice_cutoff_offset_y);
        } else {
            frame_width = pow2_dimmen_calc_for_upperslice(dst_width, max_slice_dimmen, NULL);
            frame_height = pow2_dimmen_calc_for_upperslice(dst_height, max_slice_dimmen, NULL);
        }
    } else {
        if (values.sub_slice) {
            frame_width = pow2_dimmen_calc_for_subslice(dst_width, &subslice_cutoff_offset_x);
            frame_height = pow2_dimmen_calc_for_subslice(dst_height, &subslice_cutoff_offset_y);
        } else {
            frame_width = pow2_dimmen_calc(dst_width);
            frame_height = pow2_dimmen_calc(dst_height);
        }
    }
    if (!values.sub_slice) {
        subslice_cutoff_offset_x = -1;
        subslice_cutoff_offset_y = -1;
    }

    int32_t frame_size = frame_width * frame_height * sizeof(uint16_t);
    if (values.vq && values.force_square_vq) {
        int32_t max_vq_dimmen = frame_width > frame_height ? frame_width : frame_height;
        frame_width = frame_height = max_vq_dimmen;
    }
    if (!values.sub_slice && !values.uper_slice) {
        if (values.vq && !values.force_vq_on_small && (frame_width < 64 || frame_height < 64)) {
            printf(
                "VQ compression disabled for small texture (%ix%i): %s\n",
                source_width, source_height, values.input_filename
            );
            values.vq = false;
        }
    }

    // step 5: resize and pixel format conversion
    buffer = av_malloc(frame_width * frame_height * sizeof(RGBA8));
    assert(buffer != NULL);
    if (!sws_run(pixfmt, image, buffer, source_width, source_height, dst_width, dst_height, &values)) {
        success = false;
        goto L_return;
    }

    ffgraph_destroy(ffgraph);
    ffgraph = NULL;
    srchnd->destroy(srchnd);
    srchnd = NULL;

    // step 6: process ARGB1555/ARGB4444 if required
    if (pixfmt == KDT_PixelFormat_ARGB1555 || pixfmt == KDT_PixelFormat_ARGB4444) {
        buffer = convert_to_argb(buffer, dst_width, dst_height, pixfmt == KDT_PixelFormat_ARGB1555);
#ifndef USE_FFMPEG_SWS_YUV_CONVERSOR
    } else if (pixfmt == KDT_PixelFormat_YUV422) {
        buffer = convert_to_uyvy(buffer, dst_width, dst_height);
#endif
    }

    // step 7: wrap image
    canvas = calloc(frame_size + 2048, 1);
    assert(canvas != NULL);
    wrap_frame(buffer, canvas, dst_width, dst_height, frame_width, frame_height, pixfmt == KDT_PixelFormat_YUV422);

    // step 8: create output file
    if (values.uper_slice)
        assert(frame_width <= max_slice_dimmen && frame_height <= max_slice_dimmen);
    else
        assert(frame_width <= MAX_DIMMEN && frame_height <= MAX_DIMMEN);

    kdt_file = fopen(values.output_filename, "wb+");
    if (!kdt_file) {
        success = false;
        printf("failed to create output file: %s\n", values.output_filename);
        goto L_return;
    }

    KDT hdr = (KDT){
        .signature = KDT_SIGNATURE,
        .flags = SETF(values.twiddle, KDT_HEADER_FLAG_TWIDDLE) | SETF(values.vq, KDT_HEADER_FLAG_VQ),
        .pixel_format = (uint8_t)pixfmt,
        .encoded_width = dst_width,
        .encoded_height = dst_height,
        .original_width = info.video_width,
        .original_height = info.video_height,
        .frame_width = frame_width,
        .frame_height = frame_height,
        .subslice_cutoff_x = 0,
        .subslice_cutoff_y = 0,
        .padding_width = 0,
        .padding_height = 0
    };

    if (values.uper_slice && (dst_width > MAX_DIMMEN || dst_height > MAX_DIMMEN)) {
        hdr.flags |= SETF(values.uper_slice, KDT_HEADER_FLAG_UPERSLICE);
    }
    if (values.sub_slice && (subslice_cutoff_offset_x > 0 || subslice_cutoff_offset_y > 0)) {
        hdr.flags |= SETF(values.sub_slice, KDT_HEADER_FLAG_SUBSLICE);
        if (subslice_cutoff_offset_x > 0) hdr.subslice_cutoff_x = (uint16_t)subslice_cutoff_offset_x;
        if (subslice_cutoff_offset_y > 0) hdr.subslice_cutoff_y = (uint16_t)subslice_cutoff_offset_y;
    }
    if (values.opacity_slice) {
        hdr.flags |= SETF(values.opacity_slice, KDT_HEADER_FLAG_OPACITYSLICE);
        hdr.padding_width = info.video_width - source_width;
        hdr.padding_height = info.video_height - source_height;
    }

    fwrite(&hdr, sizeof(KDT), 1, kdt_file);

    // dump_file(canvas, frame_width, frame_height);

    // step 9: save as slice blocks (if applicable)
    for (int32_t offset_x = 0, offset_y = 0; offset_y < frame_height;) {
        int32_t block_width = calc_slice_block_dimmen(offset_x, subslice_cutoff_offset_x, frame_width);
        int32_t block_height = calc_slice_block_dimmen(offset_y, subslice_cutoff_offset_y, frame_height);

        uint16_t* canvas_ptr = canvas + offset_x + (frame_width * offset_y);

        encode(&values, pixfmt, canvas_ptr, buffer, block_width, block_height, frame_width);

        size_t block_size = block_width * block_height * sizeof(uint16_t);
        if (values.vq) block_size = (block_size / 8) + 2048;
        fwrite(buffer, block_size, 1, kdt_file);

        offset_x += block_width;
        if (offset_x >= frame_width) {
            offset_x = 0;
            offset_y += block_height;
        }
    }

    fflush(kdt_file);

    // step 10: LZSS compression (if required)
    free(canvas);
    canvas = NULL;

    av_free(buffer);
    buffer = NULL;

    long raw_texture_size = ftell(kdt_file) - offsetof(KDT, data);
    double lzss_ratio = -1.0;

    if (values.lzss) {
        void* kdt_contents = malloc(raw_texture_size);
        assert(kdt_contents);

        fseek(kdt_file, sizeof(KDT), SEEK_SET);
        fread(kdt_contents, sizeof(uint8_t), (size_t)raw_texture_size, kdt_file);

        size_t lzss_buffer_size;
        uint8_t* lzss_buffer;

        lzss_compress(kdt_contents, (size_t)raw_texture_size, &lzss_buffer, &lzss_buffer_size);
        free(kdt_contents);

        long ratio_min = (long)(raw_texture_size * MIN_COMPRESSION_RATIO);
        long ratio_actual = raw_texture_size - (long)lzss_buffer_size;
        if (lzss_buffer && lzss_buffer_size > 0 && ratio_actual >= ratio_min) {
            // update header flags
            hdr.flags |= KDT_HEADER_FLAG_LZSS;

            // write again
            fseek(kdt_file, 0, SEEK_SET);
            fwrite(&hdr, sizeof(KDT), 1, kdt_file);
            fwrite(lzss_buffer, sizeof(uint8_t), lzss_buffer_size, kdt_file);
            fflush(kdt_file);

            // truncate file
            ftruncate(fileno(kdt_file), ftell(kdt_file));

            lzss_ratio = (double)lzss_buffer_size / (double)raw_texture_size;
        }

        if (lzss_buffer) {
            free(lzss_buffer);
        }
    }

    // step 11: print stats
    const char* pixfmt_name = NULL;
    switch (pixfmt) {
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
            assert(false);
            break;
    }

    float texture_size_in_bytes = raw_texture_size;
    const char* texture_size_in_bytes_unit = NULL;
    if (texture_size_in_bytes < 1024) {
        texture_size_in_bytes_unit = "B";
    } else if (texture_size_in_bytes < (1024 * 1024)) {
        texture_size_in_bytes /= 1024;
        texture_size_in_bytes_unit = "KiB";
    } else {
        texture_size_in_bytes /= 1024 * 1024;
        texture_size_in_bytes_unit = "MiB";
    }

    int32_t slice_width_blocks = (frame_width + MAX_DIMMEN - 1) / MAX_DIMMEN;
    int32_t slice_height_blocks = (frame_height + MAX_DIMMEN - 1) / MAX_DIMMEN;

    if (subslice_cutoff_offset_x > 0) slice_width_blocks++;
    if (subslice_cutoff_offset_y > 0) slice_height_blocks++;

    printf(
        "Metadata:      %s%s%s%s%s%s%s%s %.2f%s\n",
        pixfmt_name,
        values.twiddle ? " TWIDDLE" : "",
        values.vq ? " VQ" : "",
        (values.force_square_vq && frame_width == frame_height) ? " SQUARE" : "",
        hdr.flags & KDT_HEADER_FLAG_SUBSLICE ? " SUBSLICE" : "",
        hdr.flags & KDT_HEADER_FLAG_UPERSLICE ? " UPERSLICE" : "",
        hdr.flags & KDT_HEADER_FLAG_OPACITYSLICE ? " OPACITYSLICE" : "",
        lzss_ratio > 0.0 ? " LZSS" : "",
        texture_size_in_bytes, texture_size_in_bytes_unit
    );
    printf(
        "Dimmensions:   original=%ix%i encoded=%ix%i frame=%ix%i\n",
        info.video_width, info.video_height,
        dst_width, dst_height,
        frame_width, frame_height
    );
    printf(
        "Slices:        blocks=%ix%i (%i texture slices created)\n",
        slice_width_blocks, slice_height_blocks,
        slice_width_blocks * slice_height_blocks
    );
    if (values.opacity_slice) {
        printf(
            "Padding:       %ix%i (due OPACITYSLICE option)\n",
            (int)hdr.padding_width, (int)hdr.padding_height
        );
    }
    if (values.lzss) {
        if (lzss_ratio > 0.0)
            printf("File LZSS:     %.2g%%\n", lzss_ratio * 100.0);
        else
            printf("File LZSS:     not compressible\n");
    }
    if (raw_texture_size > ((8 * 1024 * 1024) / 2)) {
        printf("WARNING:       the created texture will require alot of VRAM\n");
    }
    printf("Succesfully created %s\n", values.output_filename);

L_return:
    if (kdt_file) fclose(kdt_file);
    if (canvas) free(canvas);
    if (buffer) av_free(buffer);
    free(values.output_filename);
    if (srchnd) srchnd->destroy(srchnd);
    if (ffgraph) ffgraph_destroy(ffgraph);

    return success ? 0 : (unsupported ? 2 : 1);
}
