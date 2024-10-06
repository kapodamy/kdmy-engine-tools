#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adpcm_dec_enc.h"

#include "../../logger.h"
#include "../kdm_internal.h"


#define ADDITIONAL_PACKET_DURATION 0.001 // 1ms
#define BUFFER_LENGTH (16 * 1024 * 1024) // 16MiB

struct _KDM_Audio {
    size_t buffer_used;
    size_t samples_per_packet;
    size_t audio_prebuffering;
    int32_t rate;
    bool is_stereo;
    ADPCM_Encoder adpcm_enc;
    uint8_t buffer[BUFFER_LENGTH];
};


static inline void encode_samples(KDM_Audio* audioenc, uint8_t* buffer_ptr, int16_t* pcm16sle, size_t count) {
    if (audioenc->is_stereo) {
        for (size_t i = 0; i < count; i++) {
            uint8_t sample1 = adpcm_encode_sample(*pcm16sle++, &audioenc->adpcm_enc, 0);
            uint8_t sample2 = adpcm_encode_sample(*pcm16sle++, &audioenc->adpcm_enc, 1);

            *buffer_ptr++ = (sample1 << 4) | sample2;
        }
    } else {
        for (size_t i = 0; i < count; i++) {
            *buffer_ptr++ = adpcm_encode_sample(*pcm16sle++, &audioenc->adpcm_enc, 0);
        }
    }
}

static ssize_t flush_buffer(uint8_t* buffer, size_t sample_bytes_to_write, bool is_stereo, FILE* mediafile) {
    if (!is_stereo) {
        // check if the buffer has odd number of samples
        if (sample_bytes_to_write & 1) {
            buffer[sample_bytes_to_write] = 0x00;
            sample_bytes_to_write++;
        }

        // concat audio samples
        uint8_t* buf_dst = buffer;
        uint8_t* buf_src = buffer;
        for (size_t i = 0; i < sample_bytes_to_write; i += 2) {
            *buf_dst = (buf_src[1] << 4) | buf_src[0];
            buf_src += 2;
            buf_dst++;
        }

        sample_bytes_to_write /= 2;
    }

    size_t ret = fwrite(buffer, 1, sample_bytes_to_write, mediafile);
    if (ret != sample_bytes_to_write) {
        return -2;
    }

    return (ssize_t)sample_bytes_to_write;
}


KDM_Audio* kdm_audioenc_init(int32_t rate, bool is_stereo, int32_t fps) {
    if (rate < 1 || rate > 48000) {
        logger("kdm_audioenc_init() failed, invalid sample rate\n");
        return NULL;
    }

    KDM_Audio* audioenc = malloc(sizeof(KDM_Audio));
    assert(audioenc);

    audioenc->samples_per_packet = (size_t)ceil(((1.0 / fps) + ADDITIONAL_PACKET_DURATION) * rate);
    audioenc->buffer_used = 0;
    audioenc->rate = rate;
    audioenc->is_stereo = is_stereo;
    audioenc->audio_prebuffering = 0;

    if (!is_stereo && (audioenc->samples_per_packet & 1)) {
        // for mono stream the "samples_per_packet" must be an even number
        audioenc->samples_per_packet++;
    }

    adpcm_encoder_init(&audioenc->adpcm_enc);

    return audioenc;
}

void kdm_audioenc_destroy(KDM_Audio* audioenc) {
    free(audioenc);
}

ssize_t kdm_audioenc_encode_samples(KDM_Audio* audioenc, int16_t* pcm16sle, size_t samples, FILE* mediafile) {
    const bool is_stereo = audioenc->is_stereo;
    uint8_t* buffer = audioenc->buffer;
    size_t buffer_used = audioenc->buffer_used;

    if (!pcm16sle) {
        if (audioenc->buffer_used < 1) {
            return -1;
        }

        ssize_t ret = flush_buffer(buffer, buffer_used, is_stereo, mediafile);
        audioenc->buffer_used = 0;
        return ret;
    }

    const size_t buffer_available = BUFFER_LENGTH - buffer_used;
    if (samples > buffer_available) {
        logger(
            "kdm_audioenc_encode_samples() failed, not enough space in the internal buffer."
            " in_samples=%zu used=%zu length=%zu\n",
            samples, buffer_used, BUFFER_LENGTH
        );
        return -2;
    }

    encode_samples(audioenc, buffer + buffer_used, pcm16sle, samples);
    buffer_used += samples;

    size_t samples_per_packet = audioenc->samples_per_packet;
    if (buffer_used < samples_per_packet) {
        // not enough samples to complete the packet
        audioenc->buffer_used = buffer_used;
        return 0;
    }

    const size_t audio_prebuffering = audioenc->audio_prebuffering;
    if (audio_prebuffering > 0) {
        if (buffer_used < audio_prebuffering) {
            // not enough samples to complete the prebuffering
            audioenc->buffer_used = buffer_used;
            return 0;
        } else {
            // done, disable it
            audioenc->audio_prebuffering = 0;
            samples_per_packet = audio_prebuffering;
        }
    }

    ssize_t bytes_written = flush_buffer(buffer, samples_per_packet, is_stereo, mediafile);
    buffer_used -= samples_per_packet;

    memmove(buffer, buffer + samples_per_packet, buffer_used);
    audioenc->buffer_used = buffer_used;

    return bytes_written;
}

bool kdm_audioenc_has_buffered_samples(KDM_Audio* audioenc) {
    return audioenc->buffer_used > 0;
}

void kdm_audioenc_enable_prebuffering(KDM_Audio* audioenc, size_t sample_amount) {
    audioenc->audio_prebuffering = sample_amount;
}
