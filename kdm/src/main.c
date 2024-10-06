#include <assert.h>
#include <float.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libavutil/opt.h>

#include "ffgraph/ffgraph.h"
#include "kdm/avi_creator.h"
#include "kdm/kdm_internal.h"
#include "logger.h"

#define RES_L_WIDTH 640
#define RES_L_HEIGHT 480
#define RES_W_WIDTH 640
#define RES_W_HEIGHT 360
#define RES_SMALL_L_WIDTH 320
#define RES_SMALL_L_HEIGHT 240
#define RES_SMALL_W_WIDTH 320
#define RES_SMALL_W_HEIGHT 180
#define ASPECT_RATIO_LETTERBOX (4.0 / 3.0)
#define M1V_BITRATE_KBPS_HQ 800
#define M1V_BITRATE_KBPS_LQ 400

#define STRING_EQUALS(str1, str2) (strcmp(str1, str2) == 0)
#define CHECK_OPTION_VALUE(value_string, jump_label) \
    if (value_string == NULL || value_string[0] == '\0') goto jump_label;

typedef struct {
    const char* dither;
    int32_t fps;
    int32_t scale_algorithm;
    bool silence;
    int32_t sample_rate;
    bool mono;
    const char* input_filename;
    char* output_filename;
    int32_t cuekeyframe_interval;
    bool force_small_resolution;
    bool hq;
    int32_t custom_gop;
    bool no_save_original_size;
    int32_t avi_bitrate_kbps;
    int32_t m1v_bitrate_kbps;
    bool no_progress;
} Arguments;
typedef struct {
    FFGraph* ffgraph;
    double sample_rate;
    double time_video;
    double time_audio;
    double duration;
} FFGraphWrapper;
typedef struct __attribute__((__packed__)) {
    volatile char signature[16];
    volatile double porj;
} InterProcArea;

// encoded "!KDM!@IntrPRC<%>" string
static const char encoded_signature[16] = {0x23, 0x4D, 0x46, 0x4F, 0x23, 0x42, 0x4B, 0x70, 0x76, 0x74, 0x52, 0x54, 0x45, 0x3E, 0x27, 0x40};
static const int8_t encoded_signature_slide = -2;

static double next_porj = 0.0;
static volatile bool end_line = false;
static volatile bool has_int_signal = false;
static InterProcArea interproc_comm = {};
static bool force_small_resolution = false;
static bool no_progress = false;


static void main_logging_hook(void* avcl, int level, const char* fmt, va_list vl) {
    if (end_line) {
        putchar('\n');
        end_line = false;
    }
    av_log_default_callback(avcl, level, fmt, vl);
}

static void dimmen_setter(int src_width, int src_height, int* dst_width, int* dst_height) {
    if (force_small_resolution) {
        double aspect_ratio = (double)src_width / (double)src_height;
        if (fabs(aspect_ratio - ASPECT_RATIO_LETTERBOX) <= DBL_EPSILON) {
            // 4:3
            *dst_width = RES_SMALL_L_WIDTH;
            *dst_height = RES_SMALL_L_HEIGHT;
        } else {
            // 16:9 or any other
            *dst_width = RES_SMALL_W_WIDTH;
            *dst_height = RES_SMALL_W_HEIGHT;
        }
    } else if (src_width < 1024 && src_height < 1024) {
        *dst_width = src_width;
        *dst_height = src_height;
    } else {
        double aspect_ratio = (double)src_width / (double)src_height;
        if (src_width != src_height && fabs(aspect_ratio - ASPECT_RATIO_LETTERBOX) <= DBL_EPSILON) {
            // 4:3
            *dst_width = RES_L_WIDTH;
            *dst_height = RES_L_HEIGHT;
        } else {
            // 16:9 or any other
            *dst_width = RES_W_WIDTH;
            *dst_height = RES_W_HEIGHT;
        }
    }
}

static void print_progress(FFGraphWrapper* ffgraph_wrapper) {
    double time = AV_NOPTS_VALUE;
    if (ffgraph_wrapper->time_audio > ffgraph_wrapper->time_video) {
        if (ffgraph_wrapper->time_audio != AV_NOPTS_VALUE)
            time = ffgraph_wrapper->time_audio;
        else
            time = ffgraph_wrapper->time_video;
    } else {
        if (ffgraph_wrapper->time_video != AV_NOPTS_VALUE)
            time = ffgraph_wrapper->time_video;
        else
            time = ffgraph_wrapper->time_audio;
    }

    if (time == AV_NOPTS_VALUE) time = ffgraph_wrapper->duration;

    double porj = (time / ffgraph_wrapper->duration) * 100.0;


    // truncate two decimals
    porj = trunc(porj * 100.0) / 100.0;

    if (porj < next_porj) return;
    next_porj = trunc(porj) + 1.0;
    interproc_comm.porj = porj;

    if (no_progress) return;

    if (end_line) {
        putchar('\n');
    }
    printf("\r\r                    \r\rencoding... %.0f%%\r", trunc(porj));
    // printf("encoding... %.2f%%\n", porj);
    // fflush(stdout);
}

static void print_avi_progress(void* userdata, double time) {
    FFGraphWrapper* ffgraph_wrapper = userdata;

    if (time < 0.0) {
        ffgraph_wrapper->time_video = 0.0;
        next_porj = 0.0;
    } else {
        ffgraph_wrapper->time_video = time;
    }

    print_progress(ffgraph_wrapper);
}

static int32_t read_audio(void* userdata, int16_t* buffer, int32_t max_samples_per_channel) {
    FFGraphWrapper* ffgraph_wrapper = userdata;

    int32_t ret = ffgraph_read_audio_samples(ffgraph_wrapper->ffgraph, buffer, max_samples_per_channel);

    if (ret > 0) {
        ffgraph_wrapper->time_audio += ret / (double)ffgraph_wrapper->sample_rate;
        print_progress(ffgraph_wrapper);
    }

    return ret;
}

static double read_video(void* userdata, AVFrame** out_frame) {
    FFGraphWrapper* ffgraph_wrapper = userdata;

    double pts = ffgraph_read_video_frame(ffgraph_wrapper->ffgraph, out_frame);

    if (pts > 0) {
        ffgraph_wrapper->time_video = pts;
        print_progress(ffgraph_wrapper);
    }

    return pts;
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
    const char* program = argc > 0 ? argv[0] : "kdm_enc";

    size_t idx = 0;
    for (size_t i = 0; program[i] != '\0'; i++) {
        if (program[i] == '/' || program[i] == '\\') {
            idx = i + 1;
        }
    }
    program = program + idx;

    const AVClass* sws_avclass = sws_get_class();
    const AVOption* sws_options = sws_avclass->option;

    printf("KDM encoder v0.20 by kapodamy\n");
    printf("Encodes media files (like mp4, mkv, avi, etc.) to kdm media format ");
    printf("which uses MPEG-1 for video and YAMAHA 4-bit ADPCM for audio.\n");
    printf("\n");
    printf("Usage: %s [options...] <input video file> <output video file>\n", program);
    printf("\n");
    printf("Video options:\n");
    printf(" -q, --hq                                       Uses a high bitrate, improves quality alot but can produce suttering (see notes)\n");
    printf(" -d, --dither-algorithm <dither algorithm>      Dither algortihm used for video rescale. Default: auto\n");
    printf(" -s, --scale-algorithm <scale algorithm>        Used only for downscaling. Default: lanczos\n");
    printf(" -f, --fps <framerate>                          Frames per second, the maximum is 60. By default is 24 FPS (see notes)\n");
    printf(" -c, --cue-interval <seconds>                   Used for seeking, lower values allow fast seeking. Default: 3\n");
    printf(" -l, --small-resolution                         Force downscale the video to 320x240 (4:3) or 320x180 (16:9) if necessary, resolution choosen by aspect ratio\n");
    printf(" -g, --gop  <integer>                           Changes the GOP size. Default: 15, 18 (with '--hq' option) or 12 (with '--mpeg-bitrate' option)\n");
    printf(" -z, --no-save-original-resolution              Do not store the original video resolution in the output file if was downscaled.\n");
    printf(" -p, --two-pass-bitrate                         Two-pass video encoding bitrate expressed in Kbits/s, can improve the video quality. Default: 5000\n");
    printf(" -u, --mpeg-bitrate                             Final video bitrate expressed in Kbits/s, this determines the video quality. Default: 800 if '--hq' is used, otherwise, 400\n");
    printf("\n");
    printf("Audio options:\n");
    printf(" -n, --silence                                  Ignore the audio stream.\n");
    printf(" -r, --sample-rate <frequency>                  Audio sample rate in hertz, recommended value is 32000. By default the original sample rate is used.\n");
    printf(" -m, --mono                                     Downmix the audio stream to mono.\n");
    printf("\n");
    printf("Other options:\n");
    printf(" -x, --no-progress                              No display encoding progress\n");
    printf("\n");
    printf("Dither algorithms:\n");
    print_avoption_values(sws_options, "sws_dither");
    printf("\n");
    printf("Scale algorithms:\n");
    print_avoption_values(sws_options, "sws_flags");
    printf("\n");
    printf("Notes:\n");
    printf("* The GD-ROM read speed is slow, if the video resolution or fps are high can produce suttering.\n");
    printf("* Use a lower video bitrate to avoid suttering.\n");
    printf("* If the video resolution exceeds the 1024x1024 it will downscaled to 640x480 (letterbox)");
    printf(" or 640x360 (widescreen), the output resolution is choosen depending on the video aspect ratio.\n");
    printf("* For cue interval, low values allows fast seeking but the result file is slightly bigger ");
    printf("and viceversa.\n");
    printf("* Using low framerates, sample rate or resolution can improve the I/O performance.\n");
    printf("* Audio channels are by default mono or stereo but multichannel audio is downmixed to stereo.\n");
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
    int64_t sws_value = 0;

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
        } else if (STRING_EQUALS(option_name, "q") || STRING_EQUALS(option_name, "hq")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            values->hq = true;
        } else if (STRING_EQUALS(option_name, "d") || STRING_EQUALS(option_name, "dither-algorithm")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            if (has_avoption_value(sws_options, "sws_dither", option_value, &sws_value))
                values->dither = option_value;
            else
                goto L_invalid_value;
        } else if (STRING_EQUALS(option_name, "f") || STRING_EQUALS(option_name, "fps")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->fps = atoi(option_value);
            if (values->fps < 1 || values->fps > 60) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "c") || STRING_EQUALS(option_name, "cue-interval")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->cuekeyframe_interval = atoi(option_value);
            if (values->cuekeyframe_interval < 1) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "s") || STRING_EQUALS(option_name, "scale-algorithm")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            if (has_avoption_value(sws_options, "sws_flags", option_value, &sws_value))
                values->scale_algorithm = (int32_t)sws_value;
            else
                goto L_invalid_value;
        } else if (STRING_EQUALS(option_name, "n") || STRING_EQUALS(option_name, "silence")) {
            values->silence = true;
        } else if (STRING_EQUALS(option_name, "r") || STRING_EQUALS(option_name, "sample-rate")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->sample_rate = atoi(option_value);
            if (values->sample_rate < 1 || values->sample_rate > 48000) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "m") || STRING_EQUALS(option_name, "mono")) {
            values->mono = true;
        } else if (STRING_EQUALS(option_name, "l") || STRING_EQUALS(option_name, "small-resolution")) {
            values->force_small_resolution = true;
        } else if (STRING_EQUALS(option_name, "g") || STRING_EQUALS(option_name, "gop")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->custom_gop = atoi(option_value);
            if (values->custom_gop < 1 || values->custom_gop > 127) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "z") || STRING_EQUALS(option_name, "no-save-original-resolution")) {
            values->no_save_original_size = true;
        } else if (STRING_EQUALS(option_name, "p") || STRING_EQUALS(option_name, "two-pass-bitrate")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->avi_bitrate_kbps = atoi(option_value);
            if (values->avi_bitrate_kbps < 1 || values->avi_bitrate_kbps > 800000) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "u") || STRING_EQUALS(option_name, "mpeg-bitrate")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->m1v_bitrate_kbps = atoi(option_value);
            if (values->m1v_bitrate_kbps < 1 || values->m1v_bitrate_kbps > 800000) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "x") || STRING_EQUALS(option_name, "no-progress")) {
            values->no_progress = true;
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

        out_filename2 = malloc(dot_idx + sizeof(".kdm"));
        assert(out_filename2);

        memcpy(out_filename2, in_filename, dot_idx);
        out_filename2[dot_idx + 0] = '.';
        out_filename2[dot_idx + 1] = 'm';
        out_filename2[dot_idx + 2] = 'v';
        out_filename2[dot_idx + 3] = 'q';
        out_filename2[dot_idx + 4] = 'a';
        out_filename2[dot_idx + 5] = '\0';
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

static void handle_INT(int sig) {
    // restore INT handler
    signal(sig, SIG_IGN);
    has_int_signal = true;
    logger("Stopping...");
}

int main(int argc, char** argv) {
    bool success = true;
    KDM_Encoder* kdm = NULL;
    FFGraph* ffgraph = NULL;
    char* temp_avi_file = NULL;

    Arguments values = (Arguments){
        .dither = NULL,
        .fps = 24,
        .scale_algorithm = SWS_LANCZOS,
        .silence = false,
        .sample_rate = 0,
        .mono = false,
        .input_filename = NULL,
        .output_filename = NULL,
        .cuekeyframe_interval = -1,
        .force_small_resolution = false,
        .hq = false,
        .custom_gop = -1,
        .no_save_original_size = false,
        .avi_bitrate_kbps = 5000,
        .m1v_bitrate_kbps = -1,
        .no_progress = false
    };

    interproc_comm.porj = 0.0;
    memset((void*)interproc_comm.signature, 0x00, sizeof(encoded_signature));
    for (size_t i = 0; i < sizeof(encoded_signature); i++) {
        interproc_comm.signature[i] = encoded_signature[i] + encoded_signature_slide;
    }

    av_log_set_callback(main_logging_hook);

    parse_arguments(argc, argv, sws_get_class()->option, &values);
    assert(values.input_filename && values.output_filename);

    force_small_resolution = values.force_small_resolution;
    no_progress = values.no_progress;

    videoconverter_set_custom(
        true, values.scale_algorithm, dimmen_setter, values.dither
    );
    audioconverter_enable_custom_output(
        true, values.sample_rate > 0 ? values.sample_rate : -1, values.mono ? 1 : -1, true
    );

    // NOTE: "load_in_ram" does not work with FFGraph (there a bug to fix)
    SourceHandle* srchnd_video = filehandle_init1(values.input_filename, false);
    SourceHandle* srchnd_audio = values.silence ? NULL : filehandle_init1(values.input_filename, false);
    // assert(srchnd_video);

    ffgraph = ffgraph_init(srchnd_video, srchnd_audio);
    if (!ffgraph) {
        success = false;
        goto L_return;
    }

    FFGraphInfo info;
    ffgraph_get_streams_info(ffgraph, &info);

    int32_t orig_width = values.no_save_original_size ? info.video_width : info.video_original_width;
    int32_t orig_height = values.no_save_original_size ? info.video_height : info.video_original_height;

    FFGraphWrapper ffgraph_wrapper = (FFGraphWrapper){
        .ffgraph = ffgraph,
        .time_video = 0.0,
        .duration = info.video_seconds_duration,
    };

    signal(SIGINT, handle_INT);
    printf("\n");
    printf("Press Ctrl-C to stop\n");

    if (info.video_has_stream) {
        //
        // pre-process video stream for better quality
        //

        temp_avi_file = avicreator_process_video(
            ffgraph,
            print_avi_progress, &ffgraph_wrapper,
            values.output_filename, values.avi_bitrate_kbps,
            &has_int_signal
        );
        if (has_int_signal) {
            goto L_return;
        }
        if (!temp_avi_file) {
            success = false;
            goto L_return;
        }

        // custom video specs are not longer required
        videoconverter_set_custom(false, 0, NULL, NULL);

        // reopen ffgraph
        srchnd_video->destroy(srchnd_video);
        srchnd_video = filehandle_init1(temp_avi_file, false);
        assert(srchnd_video);

        ffgraph_destroy(ffgraph);
        if (srchnd_audio) srchnd_audio->seek(srchnd_audio, 0, SEEK_SET);

        ffgraph = ffgraph_init(srchnd_video, srchnd_audio);
        if (!ffgraph) {
            success = false;
            goto L_return;
        }

        ffgraph_get_streams_info(ffgraph, &info);
    }

    if (values.fps < 1) {
        if (!info.audio_has_stream || info.video_fps < 1)
            values.fps = 30;
        else if (info.video_fps > 60)
            values.fps = 60;
        else
            values.fps = info.video_fps;
    }
    if (values.sample_rate < 1) {
        if (!info.audio_has_stream || info.audio_sample_rate < 1)
            values.sample_rate = 0;
        else
            values.sample_rate = info.audio_sample_rate;
    }

    double duration = info.audio_seconds_duration > info.video_seconds_duration ? info.audio_seconds_duration : info.video_seconds_duration;

    kdm = kdm_enc_init(
        info.video_has_stream, info.audio_has_stream,
        values.output_filename,
        duration,
        values.cuekeyframe_interval
    );
    if (!kdm) {
        success = false;
        goto L_return;
    }

    if (info.video_has_stream) {
        int32_t bitrate_kbps;

        if (values.m1v_bitrate_kbps > 0)
            bitrate_kbps = values.m1v_bitrate_kbps;
        else if (values.hq)
            bitrate_kbps = M1V_BITRATE_KBPS_HQ;
        else
            bitrate_kbps = M1V_BITRATE_KBPS_LQ;

        assert(bitrate_kbps > 0);

        if (bitrate_kbps > 4800) {
            av_log(
                NULL, AV_LOG_WARNING,
                "WARNING: the MPEG-1 bitrate is bigger than 4800Kbps, this can lead to suttering. found : %iKbps.\n",
                bitrate_kbps
            );
        }

        if (!kdm_enc_set_video_params(kdm, info.video_width, info.video_height, orig_width, orig_height, values.fps, bitrate_kbps, values.custom_gop)) {
            success = false;
            goto L_return;
        }
    }
    if (info.audio_has_stream) {
        if (!kdm_enc_set_audio_params(kdm, values.fps, values.sample_rate, info.audio_channels > 1)) {
            success = false;
            goto L_return;
        }
    }

    ffgraph_wrapper = (FFGraphWrapper){
        .ffgraph = ffgraph,
        .sample_rate = info.audio_sample_rate,
        .time_audio = 0.0,
        .time_video = 0.0,
        .duration = duration,
    };
    next_porj = 0.0;

    printf("Preparing MPEG-1 two-pass encoding...\n");

    success = kdm_enc_write_streams_pass1(
        kdm,
        info.audio_has_stream ? read_audio : NULL, &ffgraph_wrapper,
        info.video_has_stream ? read_video : NULL, &ffgraph_wrapper,
        &has_int_signal
    );
    if (!success) {
        printf("pass 1 encoding failed\n");
        goto L_return;
    }

    success = kdm_enc_flush_pass1(kdm);
    if (!success) {
        printf("pass 1 flush failed\n");
        goto L_return;
    }

    success = kdm_enc_set_video_pass2(kdm);
    if (!success) {
        printf("pass 2 preparation failed\n");
        goto L_return;
    }

    ffgraph_destroy(ffgraph);
    if (srchnd_video) srchnd_video->seek(srchnd_video, 0, SEEK_SET);
    if (srchnd_audio) srchnd_audio->seek(srchnd_audio, 0, SEEK_SET);

    ffgraph = ffgraph_init(srchnd_video, srchnd_audio);
    if (!ffgraph) {
        success = false;
        printf("pass 2 ffgraph preparation failed\n");
        goto L_return;
    }

    ffgraph_wrapper = (FFGraphWrapper){
        .ffgraph = ffgraph,
        .sample_rate = info.audio_sample_rate,
        .time_audio = 0.0,
        .time_video = 0.0,
        .duration = duration,
    };
    next_porj = 0.0;

    printf("Creating %s media file...\n", values.output_filename);

    success = kdm_enc_write_streams(
        kdm,
        info.audio_has_stream ? read_audio : NULL, &ffgraph_wrapper,
        info.video_has_stream ? read_video : NULL, &ffgraph_wrapper,
        &has_int_signal
    );
    if (!success) {
        printf("pass 2 encoding failed\n");
        goto L_return;
    }

    interproc_comm.porj = 100.0;

    // whatever kdm_enc_write_streams() function failed or not flush (writes the cue table)
    success = kdm_enc_flush(kdm);

L_return:
    if (kdm) kdm_enc_destroy(kdm);
    if (ffgraph) ffgraph_destroy(ffgraph);
    if (srchnd_audio) srchnd_audio->destroy(srchnd_audio);
    if (srchnd_video) srchnd_video->destroy(srchnd_video);

    if (has_int_signal) {
        unlink(values.output_filename);
    } else {
        if (success) {
            print_progress(&ffgraph_wrapper);
            printf("\nEncoding completed\n");
        } else {
            printf("\nEncoding failed\n");
        }
    }

    if (temp_avi_file) {
        unlink(temp_avi_file);
        free(temp_avi_file);
    }

    free(values.output_filename);

    return success ? 0 : 2;
}

void logger(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (end_line) {
        putchar('\n');
        end_line = false;
    }

    vprintf(fmt, args);
    va_end(args);
}
