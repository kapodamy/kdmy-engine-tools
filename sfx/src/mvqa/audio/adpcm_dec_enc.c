#include <math.h>

#include "adpcm_dec_enc.h"


static const float diff_lookup[] = {
    0.125f,
    0.375f,
    0.625f,
    0.875f,
    1.125f,
    1.375f,
    1.625f,
    1.875f,
    -0.125f,
    -0.375f,
    -0.625f,
    -0.875f,
    -1.125f,
    -1.375f,
    -1.625f,
    -1.875f,
};
static int32_t index_scale[] = {
    0x0e6, 0x0e6, 0x0e6, 0x0e6, 0x133, 0x199, 0x200, 0x266
};


static inline int32_t calc_next_step(int32_t step, int32_t scale_index) {
    step = (step * index_scale[scale_index & 7]) >> 8;
    return step < 0x0007f ? 0x0007f : (step > 0x6000 ? 0x6000 : step);
}

static inline float calc_next_signal(int32_t step, float signal, int32_t diff_index) {
    signal += truncf(step * diff_lookup[diff_index]);
    return signal < INT16_MIN ? INT16_MIN : (signal > INT16_MAX ? INT16_MAX : signal);
}


uint8_t adpcm_encode_sample(int16_t sample, ADPCM_Encoder* enc, size_t channel) {
    float signal = enc->signals[channel];
    int32_t step = enc->steps[channel];

    int diff = (int)(((sample - signal) * 8) / step);
    int half_diff = abs(diff) / 2;

    if (half_diff > 7) half_diff = 7;
    if (diff < 0) half_diff += 8;

    signal = calc_next_signal(step, signal, half_diff);
    step = calc_next_step(step, half_diff);

    enc->signals[channel] = signal;
    enc->steps[channel] = step;

    return half_diff;
}

int16_t adpcm_decode_sample(uint8_t adpcm_sample, ADPCM_Decoder* dec, size_t channel) {
    float signal = dec->signals[channel];
    int32_t step = dec->steps[channel];

    signal = calc_next_signal(step, signal, adpcm_sample & 15);
    step = calc_next_step(step, adpcm_sample);

    dec->signals[channel] = signal;
    dec->steps[channel] = step;

    return (int16_t)signal;
}

void adpcm_decoder_init(ADPCM_Decoder* dec) {
    *dec = (ADPCM_Decoder){
        .signals = {0.0f, 0.0f},
        .steps = {0x007f, 0x007f}
    };
}

void adpcm_encoder_init(ADPCM_Encoder* dec) {
    *dec = (ADPCM_Encoder){
        .signals = {0.0f, 0.0f},
        .steps = {0x007f, 0x007f}
    };
}
