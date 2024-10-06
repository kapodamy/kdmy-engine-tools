#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/kdm/audio/adpcm_dec_enc.h"
#include "../src/kdm/kdm.h"

#include "kdm_decoder.h"
#include "pl_mpeg/pl_mpeg.h"


#define KDM_DECODER_VERSION 3


static FILE* file;
static long file_length;
static KDMFileHeader header;

static uint8_t packet_data_video[3 * 1024 * 1024]; // 3MiB

static void* decoded_image;
static int decoded_image_size;
static bool decoded_image_available;

static int16_t* decoded_samples;
static int decoded_samples_length;
static int decoded_samples_used;

static uint8_t* packet_data_audio;
static int packet_data_audio_length;
static ADPCM_Decoder adpcm_dec;

static KDMCue* cue_table;
static plm_buffer_t* mpeg1_dec_buffer;
static plm_video_t* mpeg1_dec_video;
static bool decode_video_running;


static inline void kdmdec_audio_decode(int readed_bytes) {
    if (header.flags & KDM_FLAGS_HEADER_AUDIO_STEREO) {
        int available = decoded_samples_length - decoded_samples_used;
        int16_t* ptr = decoded_samples + (decoded_samples_used * 2);

        if (readed_bytes > available) {
            printf("kdmdec_audio_decode() failed, ¿did you forget to call kdmdec_read_audio_samples()?\n");
            readed_bytes = available;
        }

        for (int i = 0; i < readed_bytes; i++) {
            uint8_t left_sample = packet_data_audio[i] & 0x0F;
            uint8_t rigth_sample = packet_data_audio[i] >> 4;

            *ptr++ = adpcm_decode_sample(left_sample, &adpcm_dec, 0);
            *ptr++ = adpcm_decode_sample(rigth_sample, &adpcm_dec, 1);
        }

        decoded_samples_used += readed_bytes;
    } else {
        int count = readed_bytes * 2;
        int available = decoded_samples_length - decoded_samples_used;
        int16_t* ptr = decoded_samples + decoded_samples_used;

        if (count > available) {
            printf("kdmdec_audio_decode() not enough space in the packet_data_audio samples\n");
            count = available;
            readed_bytes = available / 2;
        }

        for (int i = 0; i < readed_bytes; i++) {
            uint8_t encoded_sample1 = packet_data_audio[i] & 0x0F;
            uint8_t encoded_sample2 = packet_data_audio[i] >> 4;

            *ptr++ = adpcm_decode_sample(encoded_sample1, &adpcm_dec, 0);
            *ptr++ = adpcm_decode_sample(encoded_sample2, &adpcm_dec, 0);
        }

        decoded_samples_used += count;
    }
}

static inline void kdmdec_video_decode() {
    if (decode_video_running) {
        return;
    }

    decode_video_running = true;
    plm_frame_t* frame = plm_video_decode(mpeg1_dec_video);
    decode_video_running = false;

    if (!frame) return;

    plm_frame_to_rgb(frame, decoded_image, frame->width * sizeof(uint8_t) * 3);
    decoded_image_available = true;
}

static bool kdmdec_read_cue_table() {
    cue_table = calloc(header.cue_table_length, sizeof(KDMCue));
    assert(cue_table);

    if (fread(cue_table, sizeof(KDMCue), header.cue_table_length, file) != header.cue_table_length) {
        return false;
    }

    for (int32_t i = 0; i < header.cue_table_length; i++) {
        if ((long)cue_table[i].offset >= file_length) {
            printf(
                "kdmdec_read_cue_table() invalid packet offset found in cue table. index=%i timestamp=%u offset=%u\n",
                i, cue_table[i].timestamp, cue_table[i].offset
            );

            cue_table[i].offset = cue_table[i].timestamp = UINT32_MAX;
        }
    }

    return true;
}

static void kdmdec_fill_mpeg1_dec_buffer(plm_buffer_t* self, void* user) {
    (void)self;
    (void)user;

    if (cue_table == NULL) {
        return;
    }
    if (!kdmdec_parse_next_packet() && !feof(file)) {
        printf("kdmdec_fill_mpeg1_dec_buffer() call to kdmdec_parse_next_packet() failed\n");
    }
}


bool kdmdec_init(const char* filename) {
    cue_table = NULL;
    mpeg1_dec_buffer = NULL;
    mpeg1_dec_video = NULL;
    decode_video_running = NULL;

    file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    fseek(file, 0, SEEK_END);
    file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (!fread(&header, sizeof(KDMFileHeader), 1, file)) {
        fclose(file);
        return false;
    }

    if (header.signature != KDM_SIGNATURE) {
        printf("kdmdec_init() failed, the file '%s' is not KDM. Invalid header signture\n", filename);
        fclose(file);
        return false;
    }

    if (header.version != KDM_DECODER_VERSION) {
        printf("kdmdec_init() failed, unsupporteed KDM version. expected=%i found=%i\n", 0, header.version);
        fclose(file);
        return false;
    }

    if (kdmdec_has_video()) {
        assert(header.video_encoded_width <= 1024);
        assert(header.video_encoded_height <= 1024);
        assert(header.video_original_width >= header.video_encoded_width);
        assert(header.video_original_height >= header.video_encoded_height);
        assert(header.video_fps > 0.0f && header.video_fps <= 60.0f);

        decoded_image_size = header.video_encoded_width * header.video_encoded_height * sizeof(uint8_t) * 3;
        decoded_image = malloc(decoded_image_size * 10);
        assert(decoded_image);

        mpeg1_dec_buffer = plm_buffer_create_with_capacity(sizeof(packet_data_video) * 2);
        plm_buffer_set_load_callback(mpeg1_dec_buffer, kdmdec_fill_mpeg1_dec_buffer, NULL);
        mpeg1_dec_video = plm_video_create_with_buffer(mpeg1_dec_buffer, TRUE);
    }

    if (kdmdec_has_audio()) {
        assert(header.audio_frequency > 0 && header.audio_frequency <= 48000);
        size_t channels = (header.flags & KDM_FLAGS_HEADER_AUDIO_STEREO) ? 2 : 1;
        double packet_duration = header.video_fps ? (1.0 / header.video_fps) : 2.0;
        packet_duration += 5.0; // just in case

        decoded_samples_length = (int)(packet_duration * header.audio_frequency);
        decoded_samples = malloc(decoded_samples_length * sizeof(int16_t) * channels);
        assert(decoded_samples);

        packet_data_audio_length = ((header.audio_frequency * 4.020) / 2);
        packet_data_audio = malloc(packet_data_audio_length * channels);
        assert(packet_data_audio);

        adpcm_decoder_init(&adpcm_dec);
    }

    decoded_image_available = false;
    decoded_samples_used = 0;

    printf(
        "loaded:\n  %s\n",
        filename
    );
    printf(
        "capabilities:\n  %s%s%s%s%s\n",
        (header.flags & KDM_FLAGS_HEADER_HAS_VIDEO) ? "HAS_VIDEO " : "",
        (header.flags & KDM_FLAGS_HEADER_HAS_AUDIO) ? "HAS_AUDIO " : "",
        (header.flags & KDM_FLAGS_HEADER_AUDIO_STEREO) ? "AUDIO_STEREO " : "",
        (header.flags & KDM_FLAGS_HEADER_RESERVED1) ? "RESERVED1 " : "",
        (header.flags & KDM_FLAGS_HEADER_RESERVED2) ? "RESERVED2 " : ""
    );
    printf(
        "metadata:\n  version %i, %f seconds, %i seek points in the cue table\n",
        header.version, header.estimated_duration_in_milliseconds / 1000.0,
        header.cue_table_length
    );
    printf(
        "audio stream:\n  ADPCM %iHz\n",
        header.audio_frequency
    );
    printf(
        "video stream:\n  %ix%i (original was %ix%i) %ffps\n",
        header.video_encoded_width, header.video_encoded_height,
        header.video_original_width, header.video_original_height,
        header.video_fps
    );

    return true;
}

void kdmdec_destroy() {
    fclose(file);
    if (decoded_image) {
        free(decoded_image);
        decoded_image = NULL;
    }
    if (decoded_samples) {
        free(decoded_samples);
        decoded_samples = NULL;
    }
    if (packet_data_audio) {
        free(packet_data_audio);
        packet_data_audio = NULL;
    }
    if (cue_table) {
        free(cue_table);
        cue_table = NULL;
    }
    if (mpeg1_dec_video) {
        plm_video_destroy(mpeg1_dec_video);
        mpeg1_dec_video = NULL;
    }
}

bool kdmdec_has_audio() {
    return header.flags & KDM_FLAGS_HEADER_HAS_AUDIO;
}

void kdmdec_get_audio_params(int* channels, int* sample_rate) {
    *channels = (header.flags & KDM_FLAGS_HEADER_AUDIO_STEREO) ? 2 : 1;
    *sample_rate = header.audio_frequency;
}

bool kdmdec_has_video() {
    return header.flags & KDM_FLAGS_HEADER_HAS_VIDEO;
}

void kdmdec_get_video_params(int* width, int* height, double* fps) {
    *width = header.video_encoded_width;
    *height = header.video_encoded_height;
    *fps = header.video_fps;
}

int32_t kdmdec_parse_cue_table(KDMCue** output_cue_table) {
    if (cue_table == NULL && header.cue_table_length > 0 && !kdmdec_read_cue_table()) {
        return -1;
    }

    if (output_cue_table) {
        *output_cue_table = cue_table;
    }

    return header.cue_table_length;
}

bool kdmdec_parse_next_packet() {
    KDMPacketHeader packet;

    if (cue_table == NULL && header.cue_table_length > 0 && !kdmdec_read_cue_table()) {
        goto L_failed_truncated;
    }

    if (fread(&packet, sizeof(KDMPacketHeader), 1, file) != 1) {
        return false;
    }

    // bool is_video_keyframe = packet.packet_flags & KDM_FLAGS_PACKET_VIDEO_KEYFRAME;

    if (packet.audio_data_size > 0) {
        if (packet.audio_data_size > packet_data_audio_length) {
            printf(
                "kdmdec_parse_next_packet() invalid 'packet.audio_data_size'. maximum=%i found=%i",
                packet_data_audio_length, packet.audio_data_size
            );
            return false;
        }

        if (fread(packet_data_audio, sizeof(uint8_t), packet.audio_data_size, file) != (size_t)packet.audio_data_size) {
            goto L_failed_truncated;
        }

        kdmdec_audio_decode(packet.audio_data_size);
    }

    if (packet.video_data_size > 0) {
        if (packet.video_data_size > (int32_t)sizeof(packet_data_video)) {
            printf(
                "kdmdec_parse_next_packet() invalid 'packet.video_data_size'. maximum=%zu found=%i",
                sizeof(packet_data_video), packet.video_data_size
            );
            return false;
        }

        if (fread(packet_data_video, sizeof(uint8_t), packet.video_data_size, file) != (size_t)packet.video_data_size) {
            goto L_failed_truncated;
        }

        plm_buffer_write(mpeg1_dec_buffer, packet_data_video, (size_t)packet.video_data_size);
        kdmdec_video_decode();
    }

    return true;

L_failed_truncated:
    printf("kdmdec_parse_next_packet() failed, file tructated at %li", ftell(file));
    return false;
}

bool kdmdec_read_video_frame(void* buffer) {
    if (decoded_image_available) {
        decoded_image_available = false;
        memcpy(buffer, decoded_image, decoded_image_size);
        return true;
    }
    return false;
}

int kdmdec_read_audio_samples(void* buffer, int max_samples, bool flush) {
    if (decoded_samples_used > max_samples || (flush && decoded_samples_used > 0)) {
        int count = decoded_samples_used < max_samples ? decoded_samples_used : max_samples;
        int channels = (header.flags & KDM_FLAGS_HEADER_AUDIO_STEREO) ? 2 : 1;
        int total = count * channels;

        memcpy(buffer, decoded_samples, total * sizeof(int16_t));
        decoded_samples_used -= count;

        memmove(decoded_samples, decoded_samples + total, decoded_samples_used * channels * sizeof(int16_t));

        return count;
    }
    return 0;
}

long kdmdec_get_progress() {
    assert(file);
    return (ftell(file) * 100) / file_length;
}