#include <assert.h>
#include <float.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libavutil/opt.h>

#include "ffgraph/ffgraph.h"
#include "fp_freq_table.h"
#include "mvqa/audio/adpcm_dec_enc.h"
#include "sndbridge/wavutil.h"

#define OGG_LOOPSTART "LOOPSTART"
#define OGG_LOOPLENGTH "LOOPLENGTH"

#define STRING_EQUALS(str1, str2) (strcmp(str1, str2) == 0)
#define CHECK_OPTION_VALUE(value_string, jump_label) \
    if (value_string == NULL || value_string[0] == '\0') goto jump_label;

typedef struct {
    int32_t max_duration_milliseconds;
    int32_t sample_rate;
    bool auto_sample_rate;
    bool pcm8u;
    bool mono;
    bool copy_rejected;
    bool test_only;
    const char* input_filename;
    const char* output_filename;
} Arguments;


typedef struct __attribute__((__packed__)) {
    uint32_t cue_point_id;
    uint32_t play_order_position;
    uint32_t data_chunk_id;
    uint32_t chunk_start;
    uint32_t block_start;
    uint32_t frame_offset;
} WavCuePoint;

typedef struct __attribute__((__packed__)) {
    // only two cue are required
    uint32_t count;
    WavCuePoint points[2];
} WavCue;

typedef struct __attribute__((__packed__)) {
    RIFFChunk riff;

    WavCue cue;
} WavCueChunk;


static int64_t parse_number(const char* string) {
    size_t string_length = strlen(string);
    const char* end = string + string_length;
    char* ptr = (char*)string;

    int64_t number = strtol(string, &ptr, 10);

    if (ptr >= end)
        return number;
    else
        return -1;
}

static void read_loop_points(FFGraph* ffgraph, const char* filename, int64_t* start, int64_t* end) {
    *start = -1;
    *end = -1;

    const AVFormatContext* fmt_ctx = ffgraph->audio->fmt_ctx;
    const AVStream* avstream = ffgraph->audio->ffgraphconv.av_stream;
    const char* codec_tag = fmt_ctx->iformat->name;
    const AVDictionaryEntry* tag = NULL;

    if (STRING_EQUALS(codec_tag, "ogg")) {
        long lps = -1, lpl = -1;

        while ((tag = av_dict_iterate(avstream->metadata, tag))) {
            if (STRING_EQUALS(tag->key, OGG_LOOPSTART)) {
                lps = parse_number(tag->value);
            } else if (STRING_EQUALS(tag->key, OGG_LOOPLENGTH)) {
                lpl = parse_number(tag->value);
            }
        }

        if (lps >= 0 && lpl > 0) {
            *start = lps;
            *end = lps + lpl;
        }

        return;
    }

    if (STRING_EQUALS(codec_tag, "wav") /* && !STRING_EQUALS(codec_tag, "w64")*/) {
        // FFmpeg does not have an API to parse SPML or CUE chunks, parse again the wav file
        SourceHandle* hnd = filehandle_init1(filename, false);

        int64_t data_offset, data_length, loop_start = -1, loop_length = -1;
        WavFormat fmt_info;
        wav_read_header(hnd, &fmt_info, &data_offset, &data_length, &loop_start, &loop_length);
        hnd->destroy(hnd);

        if (loop_start >= 0 && loop_length > 0) {
            *start = loop_start;
            *end = loop_start + loop_length;
        }

        return;
    }
}

static uint32_t read_samples(uint32_t total_samples, int32_t channels, bool pcm8u, FFGraph* ffgraph, void** dst) {
    size_t bytes_per_sample = pcm8u ? 1 : 2;
    size_t expected_bytes = total_samples * channels * bytes_per_sample;
    uint8_t* write_buffer = malloc(expected_bytes);
    assert(write_buffer);

    uint8_t read_buffer[8 * 1024];
    uint32_t readed_samples = 0;

    uint32_t bytes_per_sample_per_channel = bytes_per_sample * channels;
    uint8_t* buffer_offset = write_buffer;

    int32_t read_buffer_samples_length = sizeof(read_buffer) / bytes_per_sample_per_channel;

    while (true) {
        int32_t readed = ffgraph_read_audio_samples(ffgraph, read_buffer, read_buffer_samples_length);
        if (readed < 1) {
            break;
        }

        readed_samples += (uint32_t)readed;

        if (readed_samples > total_samples) {
            // increase buffer size
            expected_bytes += 1024 * 1024;
            total_samples = expected_bytes / bytes_per_sample_per_channel;
            size_t offset = (size_t)(buffer_offset - write_buffer);
            write_buffer = realloc(write_buffer, expected_bytes);
            assert(write_buffer);
            buffer_offset = write_buffer + offset;
        }

        size_t readed_bytes = readed * bytes_per_sample_per_channel;

        memcpy(buffer_offset, read_buffer, readed_bytes);
        buffer_offset += readed_bytes;
    }

    *dst = write_buffer;
    return readed_samples;
}


static size_t find_parent(const char* filename) {
    size_t idx = 0;
    for (size_t i = 0; filename[i] != '\0'; i++) {
        if (filename[i] == '/' || filename[i] == '\\') {
            idx = i + 1;
        }
    }
    return idx;
}

static bool copy_file(const char* in_filename, const char* out_filename) {
    size_t in_filename_length = strlen(in_filename);
    size_t in_parent = find_parent(in_filename);
    size_t out_parent = find_parent(out_filename);

    if (in_parent < 1 && out_parent < 1) {
        printf("ignoring '--copy-if-rejected' becuase the input and output uses the same folder.\n");
        return true;
    }

    size_t dst_length = (in_filename_length - in_parent) + out_parent;
    char* dst = malloc(dst_length + 1);
    assert(dst);
    dst[dst_length] = '\0';

    if (out_parent > 0) {
        memcpy(dst, out_filename, out_parent);
        memcpy(dst + out_parent, in_filename + in_parent, in_filename_length - in_parent);
    } else {
        memcpy(dst, in_filename + in_parent, in_filename_length - in_parent);
    }

    FILE* file = fopen(dst, "wb");
    if (!file) {
        printf("failed to create '%s' output file.\n", dst);
        free(dst);
        return false;
    }

    FILE* in_file = fopen(in_filename, "rb");
    if (!in_file) {
        printf("failed to open '%s' input file.\n", in_filename);
        return false;
    }

    size_t read;
    uint8_t buffer[8 * 1024];
    while ((read = fread(buffer, sizeof(uint8_t), sizeof(buffer), in_file)) > 0) {
        fwrite(buffer, sizeof(uint8_t), read, file);
    }


    printf("copied '%s' to '%s'.\n", in_filename, dst);

    free(dst);
    fclose(in_file);
    fclose(file);

    return true;
}


static int32_t calc_fp_freq(uint32_t freq) {
    assert(freq <= 48000);

    int32_t last_fp_freq = FPFREQ_TABLE[0].fp_freq;

    for (size_t i = 0; i < FPFREQ_TABLE_SIZE; i++) {
        const FPFreq* entry = FPFREQ_TABLE + i;

        if (entry->freq == freq)
            return entry->fp_freq;
        else if (entry->freq > freq)
            return last_fp_freq;
        else
            last_fp_freq = entry->fp_freq;
    }

    // never reached
    return last_fp_freq;
}

static uint32_t calc_correct_sampleRatePitch(uint32_t sample_rate) {
    if (sample_rate < 4) return sample_rate;

    uint32_t tmp_freq = sample_rate - 4;
    int32_t last_fp_freq = calc_fp_freq(tmp_freq);
    size_t first_repeat_index = 0;

    tmp_freq++;

    for (size_t i = 1; i < 9; i++) {
        int32_t fp_freq = calc_fp_freq(tmp_freq);

        if (fp_freq != last_fp_freq) {
            last_fp_freq = fp_freq;
            first_repeat_index = i;
        }

        if (tmp_freq >= sample_rate) {
            size_t distance = i - first_repeat_index;
            if (distance < 1) {
#if DEBUG
                if (tmp_freq != sample_rate) {
                    printf("calc_correct_sampleRatePitch() round-up the sample rate from %u to %u\n", sample_rate, tmp_freq);
                }
#endif
                return tmp_freq;
            }
        }

        tmp_freq++;
    }

    // this never should happen
    return sample_rate;
}


static void print_usage(int argc, char** argv) {
    const char* program = argc > 0 ? argv[0] : "sfx";

    size_t idx = 0;
    for (size_t i = 0; program[i] != '\0'; i++) {
        if (program[i] == '/' || program[i] == '\\') {
            idx = i + 1;
        }
    }
    program = program + idx;

    printf("SFX v0.1 by kapodamy\n");
    printf("Encodes short sound files (mostly ogg vorbis) into Wav media format and ");
    printf("uses YAMAHA 4-bit ADPCM or PCM-U8 as audio codec.\n");
    printf("The encoder checks if the supplied sound is eligible using heuristics (see notes).\n");
    printf("Loop points are supported for Ogg (from comments) and Wav (from smpl/cue chunks).\n");
    printf("\n");
    printf("Usage: %s [options...] <input audio file> <output wav file>\n", program);
    printf("\n");
    printf("Options:\n");
    printf(" -d, --max-duration    [milliseconds]       Maximum sound duration, use 0 to guess automatically. Default: 0\n");
    printf(" -r, --sample-rate     [hertz]              Sample rate frequency, use 0 to keep the original sample rate. Default: original sample rate ");
    printf("or 16000 if '--auto-samplerate' is present\n");
    printf(" -a, --auto-sample-rate                     Reduce sample rate until all encoding criterias are met, but not below '--sample-rate' value.\n");
    printf(" -p, --pcm-u8                               Use 'unsigned 8-bit PCM', better audio quality but increases x2 the result file size.\n");
    printf(" -m, --force-mono                           Downmix to mono, by default always is downmixed to stereo.\n");
    printf(" -c, --copy-if-rejected                     Copies the file in the output folder as-is if the sound is not eligible.\n");
    printf(" -t, --test-only                            Display the output metadata (channels, samples, format and duration) without writing any file.\n");
    printf("\n");
    printf("Maximum duration for the most commonly used sample rates:\n");
    printf("*  48000Hz      1.365ms     1 seconds\n");
    printf("*  44100Hz      1.486ms         \"\" \n");
    printf("*  32000hz      2.047ms     2 seconds\n");
    printf("*  24000hz      2.730ms         \"\" \n");
    printf("*  16000Hz      4.095ms     4 seconds\n");
    printf("*  12000Hz      5.461ms     5 seconds\n");
    printf("*  8000Hz       8.191ms     8 seconds\n");
    printf("*  4000Hz       16.383ms    16 seconds\n");
    printf("\n");
    printf("Notes:\n");
    printf("* The AICA only contains 64 channels available, but in the practice only nearly 26 can be used for sound effects.\n");
    printf("* The maximum amount of audio samples can be stored on AICA RAM is 65535, thats it.\n");
    printf("* The reason why are limited to 65535 is because AICA uses a 16-bit register to store the samples count.\n");
    printf("* The amount of samples is calculated as:    samples = duration_in_seconds / frecuency_in_hertz.\n");
    printf("* On sounds with more than 65535 samples the sample rate is reduced, this decrease audio quality.\n");
    printf("* The sample rate is always reduced in steps/multiple of 4 because the AICA stores the frequency in floating-point format (citation needed), ");
    printf("arbitrary values can lead to frequency missmatch (not enough precission).\n");
    printf("* Audio channels are by default mono or stereo but multichannel audio is always downmixed to stereo.\n");
    printf("* Loop points are updated (if present) when the sample rate is changed.\n");
    printf("\n");
}

static void parse_arguments(int argc, char** argv, Arguments* values) {
    if (argc < 2 || STRING_EQUALS(argv[1], "-h") || STRING_EQUALS(argv[1], "--help")) {
        print_usage(argc, argv);
        exit(0);
    }

    const char* in_filename = NULL;
    const char* out_filename = NULL;
    int option_index = 0, value_index = 0;
    int limit = argc - 2;

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
        } else if (STRING_EQUALS(option_name, "d") || STRING_EQUALS(option_name, "max-duration")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->max_duration_milliseconds = atoi(option_value);
            if (values->max_duration_milliseconds < 0) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "r") || STRING_EQUALS(option_name, "sample-rate")) {
            CHECK_OPTION_VALUE(option_value, L_missing_value);
            i++;
            values->sample_rate = atoi(option_value);
            if (values->sample_rate < 0 || values->sample_rate > 48000) {
                goto L_invalid_value;
            }
        } else if (STRING_EQUALS(option_name, "a") || STRING_EQUALS(option_name, "auto-sample-rate")) {
            values->auto_sample_rate = true;
        } else if (STRING_EQUALS(option_name, "p") || STRING_EQUALS(option_name, "pcm-u8")) {
            values->pcm8u = true;
        } else if (STRING_EQUALS(option_name, "m") || STRING_EQUALS(option_name, "force-mono")) {
            values->mono = true;
        } else if (STRING_EQUALS(option_name, "c") || STRING_EQUALS(option_name, "copy-if-rejected")) {
            values->copy_rejected = true;
        } else if (STRING_EQUALS(option_name, "t") || STRING_EQUALS(option_name, "test-only")) {
            values->test_only = true;
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

    if (!in_filename || in_filename[0] == '\0') {
        printf("missing input filename.\n");
        exit(1);
        return;
    }

    if (!out_filename || out_filename[0] == '\0') {
        printf("missing output filename.\n");
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

    values->input_filename = in_filename;
    values->output_filename = out_filename;
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
    Arguments values = (Arguments){
        .max_duration_milliseconds = 0,
        .sample_rate = 0,
        .auto_sample_rate = false,
        .pcm8u = false,
        .mono = false,
        .copy_rejected = false,
        .test_only = false,
        .input_filename = NULL,
        .output_filename = NULL
    };

    parse_arguments(argc, argv, &values);
    assert(values.input_filename && values.output_filename);

    // NOTE: "load_in_ram" does not work with FFGraph
    SourceHandle* srchnd_audio = filehandle_init1(values.input_filename, false);
    assert(srchnd_audio);

    FFGraph* ffgraph = ffgraph_init(srchnd_audio);
    if (!ffgraph) {
        printf("Failed to open the file %s\n", values.input_filename);
        srchnd_audio->destroy(srchnd_audio);
        return 1;
    }

    bool success = false;

    FFGraphInfo info;
    ffgraph_get_original_stream_info(ffgraph, &info);

    bool second_pass = false;
    uint32_t total_samples = (uint32_t)ceil(info.audio_seconds_duration * info.audio_sample_rate);
    uint32_t sample_rate = info.audio_sample_rate;
    uint32_t channels_count = (values.mono || info.audio_channels < 2) ? 1 : 2;

    if (values.auto_sample_rate) {
        if (values.sample_rate < 1) {
            values.sample_rate = 16000;
        }
    } else {
        if (values.sample_rate > 0) {
            // override the sample rate
            sample_rate = (uint32_t)values.sample_rate;
        }
    }

L_decode:
    // initialize audio decoder
    audioconverter_enable_custom_output(true, (int32_t)sample_rate, values.mono, values.pcm8u);
    ffgraph_init_audioconverter(ffgraph);
    if (second_pass) ffgraph_seek(ffgraph, 0.0);

    // read the whole sound samples in ram
    void* buffer;
    total_samples = read_samples(total_samples, channels_count, values.pcm8u, ffgraph, &buffer);

    if (!second_pass) {
        int32_t duration_ms = (int32_t)ceil((total_samples / (double)info.audio_sample_rate) * 1000.0);
        printf(
            "File:    duration=%ims  %s\n",
            duration_ms, values.input_filename
        );
        printf(
            "Input:   sampleRate=%ihz channels=%i samples=%i\n",
            info.audio_sample_rate, info.audio_channels, total_samples
        );

        if (values.max_duration_milliseconds > 0) {
            printf(
                "Output:  rejected (exceeds the maximum duration of %ums)\n",
                values.max_duration_milliseconds
            );

            if (values.copy_rejected && !values.test_only) {
                success = copy_file(values.input_filename, values.output_filename);
            }

            goto L_return;
        }
    }

    // check if the amount of samples are valid
    if (total_samples > UINT16_MAX) {
        if (!second_pass && values.auto_sample_rate) {
            // calculate the sample rate
            uint32_t new_sample_rate = (uint32_t)trunc((UINT16_MAX * sample_rate) / (double)total_samples);

            // calculate the correct frequency need for "SampleRatePitch" AICA register
            sample_rate = calc_correct_sampleRatePitch(new_sample_rate);

            if (new_sample_rate >= values.sample_rate) {
                if (ffgraph->audio->ffgraphconv.destroy_cb) {
                    // dispose previous audio decoder
                    ffgraph->audio->ffgraphconv.destroy_cb(&ffgraph->audio->ffgraphconv);
                }

                // decode the audio again with a different sample rate
                second_pass = true;
                free(buffer);
                goto L_decode;
            }

            // calculate the minimum amount
            total_samples = (total_samples * values.sample_rate) / sample_rate;
        }

        printf(
            "Output:  rejected (can not reduce the amount of samples less than %u with %uhz)\n",
            total_samples, sample_rate
        );

        if (values.copy_rejected && !values.test_only) {
            success = copy_file(values.input_filename, values.output_filename);
        }
        goto L_return;
    }

    // encode to adpcm (if required)
    if (!values.pcm8u) {
        int16_t* pcm16sle_buffer = (int16_t*)buffer;
        uint8_t* adpcm4y_buffer = (uint8_t*)buffer;

        ADPCM_Encoder adpcm_enc;
        adpcm_encoder_init(&adpcm_enc);

        if (channels_count > 1) {
            for (uint32_t i = 0; i < total_samples; i++) {
                uint8_t sample1 = adpcm_encode_sample(*pcm16sle_buffer++, &adpcm_enc, 0);
                uint8_t sample2 = adpcm_encode_sample(*pcm16sle_buffer++, &adpcm_enc, 1);

                *adpcm4y_buffer++ = (sample1 << 4) | sample2;
            }
        } else {
            for (uint32_t i = 0; i < total_samples;) {
                uint8_t sample1 = adpcm_encode_sample(*pcm16sle_buffer++, &adpcm_enc, 0);
                i++;

                uint8_t sample2;
                if (i < total_samples) {
                    sample2 = adpcm_encode_sample(*pcm16sle_buffer++, &adpcm_enc, 0);
                    i++;
                } else {
                    // filler
                    sample2 = 0x00;
                }

                *adpcm4y_buffer++ = (sample2 << 4) | sample1;
            }
        }
    }

    // show output metadata and "goto L_return" (if "--test-only" is present)
    bool is_unchanged = sample_rate == info.audio_sample_rate && channels_count == info.audio_channels && !second_pass;
    printf(
        "Output:  sampleRate=%uhz channels=%u samples=%i %s\n",
        sample_rate, channels_count, total_samples,
        is_unchanged ? "(original)" : ""
    );

    // read embedded loop points
    int64_t loop_start, loop_end;
    read_loop_points(ffgraph, values.input_filename, &loop_start, &loop_end);
    bool has_loop_points = loop_start >= 0 && loop_end > loop_start;

    if (has_loop_points) {
        bool new_sample_rate = sample_rate != info.audio_sample_rate;

        if (new_sample_rate) {
            // adjust loop points to the new sample rate. Note: round-up the loop end point
            loop_start = (sample_rate * loop_start) / info.audio_sample_rate;
            loop_end = (int64_t)ceil((sample_rate * loop_end) / (double)info.audio_sample_rate);
        }

        printf(
            "Loop:    start=%lli  end=%lli (%s)\n",
            loop_start, loop_end, new_sample_rate ? "updated" : "original"
        );
    }

    if (values.test_only) {
        success = true;
        goto L_return;
    }

    // calculate bitrate, samples data size
    uint32_t bitrate = sample_rate;
    if (values.pcm8u) bitrate *= channels_count;

    size_t data_size = total_samples;
    if (values.pcm8u)
        data_size *= channels_count;
    else if (channels_count < 2)
        data_size /= 2;

    // create wav file
    WAV wav = {
        .riff = {
            .name = WAV_HDR_RIFF,
            .chunk_size = sizeof(WAV) + data_size
        },
        .riff_type = WAV_RIFF_TYPE_WAVE,
        .fmt = {
            .name = WAV_HDR_FMT,
            .chunk_size = sizeof(WavFormat),
        },
        .format = {
            .format = values.pcm8u ? WAV_PCM : WAV_YAMAHA_AICA_ADPCM,
            .channels = channels_count,
            .sample_rate = sample_rate,
            .bitrate = bitrate,
            .block_align = values.pcm8u ? channels_count : 1024,
            .bits_per_sample = values.pcm8u ? 8 : 4,
        }
    };
    WavCueChunk cue_chunk = {
        .riff = {
            .name = WAV_HDR_CUE,
            .chunk_size = sizeof(WavCue),
        },
        .cue = {
            .count = 2,
            .points = {
                {
                    .cue_point_id = 0,
                    .play_order_position = 0,
                    .data_chunk_id = WAV_HDR_DATA,
                    .chunk_start = 0,
                    .block_start = 0,
                    .frame_offset = (uint32_t)loop_start,
                },
                {
                    .cue_point_id = 1,
                    .play_order_position = 0,
                    .data_chunk_id = WAV_HDR_DATA,
                    .chunk_start = 0,
                    .block_start = 0,
                    .frame_offset = (uint32_t)loop_end,
                },
            },
        }
    };
    RIFFChunk data_chunk = {
        .name = WAV_HDR_DATA,
        .chunk_size = data_size
    };

    if (has_loop_points) wav.riff.chunk_size += sizeof(WavCueChunk);

    FILE* wav_file = fopen(values.output_filename, "wb");
    if (!wav_file) {
        printf("failed to create '%s' output file.\n", values.output_filename);
        goto L_return;
    }

    fwrite(&wav, sizeof(WAV), 1, wav_file);
    if (has_loop_points) fwrite(&cue_chunk, sizeof(WavCueChunk), 1, wav_file);
    fwrite(&data_chunk, sizeof(RIFFChunk), 1, wav_file);
    fwrite(buffer, sizeof(uint8_t), data_size, wav_file);
    fclose(wav_file);

    success = true;
    printf("created '%s' successfully\n", values.output_filename);

L_return:
    free(buffer);
    ffgraph_destroy(ffgraph);
    if (srchnd_audio) srchnd_audio->destroy(srchnd_audio);

    return success ? 0 : 1;
}
