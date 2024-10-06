#ifndef __kdm_decoder_h
#define __kdm_decoder_h

#include <stdbool.h>
#include <stdint.h>

#include "../src/kdm/kdm.h"

bool kdmdec_init(const char* filename);
void kdmdec_destroy();
bool kdmdec_has_audio();
void kdmdec_get_audio_params(int* channels, int* sample_rate);
bool kdmdec_has_video();
void kdmdec_get_video_params(int* width, int* height, double* fps);
int32_t kdmdec_parse_cue_table(KDMCue** output_cue_table);
bool kdmdec_parse_next_packet();
bool kdmdec_read_video_frame(void* buffer);
int32_t kdmdec_read_audio_samples(void* buffer, int max_samples, bool flush);
long kdmdec_get_progress();

#endif