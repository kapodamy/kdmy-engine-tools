#include <stdio.h>

#include "../ffgraph/sourcehandle.h"
#include "wavutil.h"


typedef struct __attribute__((__packed__)) {
    uint32_t id;
    uint32_t type;
    uint32_t start;
    uint32_t end;
    uint32_t fraction;
    uint32_t play_counts;
} WavSamplePoint;

typedef struct __attribute__((__packed__)) {
    uint32_t manufacturer;
    uint32_t product;
    uint32_t sample_period;
    uint32_t midi_unity_note;
    uint32_t midi_pitch_fraction;
    uint32_t smpte_format;
    uint32_t smpte_offset;
    uint32_t sample_loops_count;
    uint32_t sample_data;
} WavSample;

typedef struct __attribute__((__packed__)) {
    uint32_t cue_point_id;        // 0|1
    uint32_t play_order_position; // 0
    uint32_t data_chunk_id;       // data
    uint32_t chunk_start;         // 0
    uint32_t block_start;         // 0
    uint32_t frame_offset;        // in samples
} WavCuePoint;


static inline void read_wav_cue(SourceHandle* hnd, int64_t* start, int64_t* length) {
    uint32_t cues_count;
    if (hnd->read(hnd, &cues_count, sizeof(uint32_t)) != sizeof(uint32_t)) {
        // truncated file
        return;
    }

    int next_cue_point_id = 0;
    WavCuePoint cue;
    uint32_t cue_start = 0, cue_end = 0;

    for (uint32_t i = 0; i < cues_count; i++) {
        if (hnd->read(hnd, &cue, sizeof(WavCuePoint)) != sizeof(WavCuePoint)) {
            // truncated file
            return;
        }

        if (cue.data_chunk_id != WAV_HDR_DATA) {
            continue;
        }

        if (cue.cue_point_id == next_cue_point_id) {
            if (next_cue_point_id == 0)
                cue_start = cue.frame_offset;
            else
                cue_end = cue.frame_offset;

            next_cue_point_id++;
        }

        if (next_cue_point_id > 1) {
            if (cue_start > cue_end) {
                *start = -2;
                *length = -2;
            } else {
                *start = cue_start;
                *length = cue_end - cue_start;
            }
            return;
        }
    }
}

static inline void read_wav_spml(SourceHandle* hnd, int64_t* start, int64_t* length) {
    WavSample spml;
    if (hnd->read(hnd, &spml, sizeof(WavSample)) != sizeof(WavSample)) {
        // truncated file
        return;
    }

    if (spml.midi_unity_note != 0 || spml.midi_pitch_fraction != 0) {
        goto L_invalid_fields;
    }
    if (spml.smpte_format != 0x00 || spml.smpte_offset != 0) {
        goto L_invalid_fields;
    }

    WavSamplePoint smpl_point;
    for (uint32_t i = 0; i < spml.sample_loops_count; i++) {
        if (hnd->read(hnd, &smpl_point, sizeof(WavSamplePoint)) != sizeof(WavSamplePoint)) {
            // truncated file
            return;
        }

        if (smpl_point.id == 0) {
            if (smpl_point.fraction != 0) goto L_invalid_fields;
            if (smpl_point.end < smpl_point.start) goto L_invalid_fields;

            // note 1: the "smpl_point.play_counts" field is ignored
            // note 2: the "smpl_point.end" does not exclude the last sample
            *start = smpl_point.start;
            *length = smpl_point.end - smpl_point.start;
            return;
        }
    }

    // empty smpl
    return;

L_invalid_fields:
    *start = -2;
    *length = -2;
}


bool wav_read_header(SourceHandle* hnd, WavFormat* fmt, int64_t* df, int64_t* dl, int64_t* lps, int64_t* lpl) {
    int64_t offset_end;
    int64_t wav_end_offset = hnd->tell(hnd);
    WAV wav;

    if (hnd->read(hnd, &wav, sizeof(WAV)) != sizeof(WAV)) {
        goto L_error_truncated;
    }

    if (wav.riff.name != WAV_HDR_RIFF || wav.riff_type != WAV_RIFF_TYPE_WAVE || wav.fmt.name != WAV_HDR_FMT) {
        printf("[x] sndbridge: read_wav_header() invalid WAV file");
        return false;
    }

    if (wav.fmt.chunk_size != sizeof(WavFormat)) {
        // seek the extra blob
        offset_end = hnd->tell(hnd) + (int64_t)(wav.fmt.chunk_size - sizeof(WavFormat));

        if (hnd->seek(hnd, offset_end, SEEK_SET)) {
            goto L_error_truncated;
        }
    }

    wav_end_offset += wav.riff.chunk_size;

    int64_t loop_start = -1, loop_length = -1;

    RIFFChunk chunk;
    while (true) {
        if (hnd->read(hnd, &chunk, sizeof(RIFFChunk)) != sizeof(RIFFChunk)) {
            goto L_error_truncated;
        }

        int64_t offset = hnd->tell(hnd);
        offset_end = offset + chunk.chunk_size;

        if (chunk.name == WAV_HDR_DATA) {
            *df = offset;
            *dl = chunk.chunk_size;
        } else if (chunk.name == WAV_HDR_SMPL) {
            read_wav_spml(hnd, &loop_start, &loop_length);
        } else if (chunk.name == WAV_HDR_CUE) {
            read_wav_cue(hnd, &loop_start, &loop_length);
        }

        if (hnd->seek(hnd, offset_end, SEEK_SET)) {
            goto L_error_truncated;
        }

        if (offset_end >= wav_end_offset) {
            // EOF
            break;
        }
    }

    if (loop_start == -2 || loop_length == -2) {
        printf("[x] sndbridge: wav_read_header() invalid loop points or cue/smpl found");
        *lps = *lpl = 0;
    } else if (loop_start < 0 || loop_length < 1) {
        *lps = *lpl = 0;
    } else {
        *lps = loop_start;
        *lpl = loop_length;
    }

    *fmt = wav.format;
    return true;

L_error_truncated:
    printf("[x] sndbridge: read_wav_header() truncated WAV file");
    return false;
}