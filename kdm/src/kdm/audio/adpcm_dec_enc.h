#ifndef _wav2adpcm_h
#define _wav2adpcm_h

#include <stdint.h>

struct _ADPCM {
    float signals[2];
    int32_t steps[2];
};

typedef struct _ADPCM ADPCM_Decoder;
typedef struct _ADPCM ADPCM_Encoder;

uint8_t adpcm_encode_sample(int16_t sample, ADPCM_Encoder* enc, size_t channel);
int16_t adpcm_decode_sample(uint8_t adpcm_sample, ADPCM_Decoder* dec, size_t channel);
void adpcm_decoder_init(ADPCM_Decoder* dec);
void adpcm_encoder_init(ADPCM_Encoder* dec);

#endif
