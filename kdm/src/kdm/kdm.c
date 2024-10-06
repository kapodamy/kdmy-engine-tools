#include <assert.h>
#include <float.h>
#include <malloc.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "kdm.h"

#include "../logger.h"
#include "huffman.h"
#include "kdm_internal.h"
#include "lzss.h"
#include "twopass_buffer.h"


#define KDM_ENCODER_VERSION 3

#define BUFFER_LENGTH 1024
#define AICA_BUFFER_SAMPLES_MONO_SIZE 65534   // must be an even number
#define AICA_BUFFER_SAMPLES_STEREO_SIZE 32767 // can not exceed the 64KiB

#define DBL_NEAR_EQUALS(d1, d2) (fabs(d1 - d2) <= DBL_EPSILON)

#define MIN_COMPRESSED_RATIO 1.12


typedef struct {
    KDM_CB_ReadVideoFrame callback;
    void* userdata;

    double next_pts;
    double next_pts_duration;

    AVFrame* old_frame;
    double old_pts;

    AVFrame* new_frame;
    double new_pts;
} FrameStepper;

typedef struct _KDM_Encoder {
    KDMFileHeader hdr;
    FrameStepper stepper;
    bool header_written;
    KDMCue cue_table[UINT16_MAX];
    uint16_t cue_table_used;
    uint16_t cue_table_length;
    FILE* mediafile;
    size_t audio_reference_fps;
    KDM_Audio* audioenc;
    KDM_Video* videoenc;
    size_t samples_written;
    int64_t estimated_cue_interval;
    int64_t next_cue_timestamp;
    TwoPassLogBuffer* pass_log;
    int video_enc_param_gop;
    double video_enc_param_fps;
    int32_t video_enc_param_bitrate_kbps;
} KDM_Encoder;


static void frame_stepper_init(FrameStepper* stepper, size_t width, size_t height, float fps) {
    AVFrame* frame = av_frame_alloc();
    assert(frame);

    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = width;
    frame->height = height;
    assert(av_frame_get_buffer(frame, 0) >= 0);

    *stepper = (FrameStepper){
        .callback = NULL,
        .userdata = NULL,
        .next_pts = 0,
        .next_pts_duration = 1.0 / fps,
        .old_frame = frame,
        .old_pts = -1.0,
        .new_frame = NULL,
        .new_pts = -1.0
    };
}

static void frame_stepper_destroy(FrameStepper* stepper) {
    av_frame_free(&stepper->old_frame);
}

static void frame_stepper_reset(FrameStepper* stepper, KDM_CB_ReadVideoFrame cb, void* ud) {
    stepper->callback = cb;
    stepper->userdata = ud;

    AVFrame* tmp_frame;
    stepper->old_pts = stepper->callback(stepper->userdata, &tmp_frame);
    av_frame_copy(stepper->old_frame, tmp_frame);

    if (stepper->old_pts < 0.0) {
        // no frames available
        stepper->next_pts = stepper->old_pts = stepper->new_pts = -1.0;
        return;
    }
    stepper->new_pts = stepper->callback(stepper->userdata, &stepper->new_frame);

    av_frame_copy(stepper->old_frame, tmp_frame);
    stepper->next_pts = stepper->old_pts;
}

static AVFrame* frame_stepper_fetch(FrameStepper* stepper) {
    assert(stepper->callback);

    /*if (stepper->next_pts != -1.0 && DBL_NEAR_EQUALS(stepper->old_pts, stepper->next_pts)) {
        goto L_return;
    }*/

    if (stepper->new_pts < 0.0) {
        // EOF reached
        return NULL;
    }

    if (DBL_NEAR_EQUALS(stepper->new_pts, stepper->next_pts) || (stepper->new_pts < stepper->next_pts)) {
        // move new_frame --> old_frame

        while (stepper->new_pts < stepper->next_pts) {
            stepper->old_pts = stepper->new_pts;
            av_frame_copy(stepper->old_frame, stepper->new_frame);

            stepper->new_pts = stepper->callback(stepper->userdata, &stepper->new_frame);
            if (stepper->new_pts < 0.0) {
                goto L_return;
            }
        }
    }

L_return:
    stepper->next_pts += stepper->next_pts_duration;
    return stepper->old_frame;
}


KDM_Encoder* kdm_enc_init(bool has_video, bool has_audio, const char* mediafile, double estimated_duration, int32_t estimated_cue_interval) {
    // assert(estimated_duration > 0.0);
    assert(mediafile != NULL && mediafile[0] != '\0');

    if (estimated_cue_interval < 0) estimated_cue_interval = 3;

    KDM_Encoder* kdmenc = calloc(1, sizeof(KDM_Encoder));
    assert(kdmenc);

    kdmenc->hdr.signature = KDM_SIGNATURE;
    kdmenc->hdr.version = KDM_ENCODER_VERSION;
    kdmenc->hdr.estimated_duration_in_milliseconds = (uint32_t)ceil(estimated_duration * 1000.0);
    kdmenc->hdr.cue_table_length = 0;

    KDM__CONFIGURE_FLAG(kdmenc->hdr.flags, KDM_FLAGS_HEADER_HAS_VIDEO, has_video);
    KDM__CONFIGURE_FLAG(kdmenc->hdr.flags, KDM_FLAGS_HEADER_HAS_AUDIO, has_audio);

    kdmenc->mediafile = fopen(mediafile, "w+b");
    assert(kdmenc->mediafile);

    //
    // calculate cue table length
    //      is limited to 65535 entries
    //      if the media is to long, the table is truncated
    //      entry timestamps are expressed in microseconds
    //      first offset entry is always: sizeof(KDMFileHeader) + (cue_table_length * sizeof(uint32_t))
    //      first timestamp entry is always: 0
    kdmenc->cue_table_used = 0;
    kdmenc->cue_table_length = UINT16_MAX;

    kdmenc->samples_written = 0;
    kdmenc->estimated_cue_interval = estimated_cue_interval * 1000 * 1000;
    kdmenc->next_cue_timestamp = 0;

    kdmenc->pass_log = twopass_log_init();

    return kdmenc;
}

void kdm_enc_destroy(KDM_Encoder* kdmenc) {
    if (kdmenc->videoenc) kdm_videoenc_destroy(kdmenc->videoenc);
    if (kdmenc->audioenc) kdm_audioenc_destroy(kdmenc->audioenc);
    frame_stepper_destroy(&kdmenc->stepper);
    fflush(kdmenc->mediafile);
    ftruncate(fileno(kdmenc->mediafile), ftell(kdmenc->mediafile));
    fclose(kdmenc->mediafile);
    twopass_log_destroy(kdmenc->pass_log);
    free(kdmenc);
}


bool kdm_enc_set_video_params(KDM_Encoder* kdmenc, int32_t small_width, int32_t small_height, int32_t orig_width, int32_t orig_height, double fps, int32_t bitrate_kbps, int gop) {
    if (!(kdmenc->hdr.flags & KDM_FLAGS_HEADER_HAS_VIDEO)) {
        logger("kdm_set_video_params() not allowed, the kdm does not have a video stream\n");
        return false;
    }
    if (kdmenc->videoenc) {
        logger("kdm_set_video_params() error, already was called\n");
        return false;
    }

    kdmenc->hdr.video_original_width = orig_width;
    kdmenc->hdr.video_original_height = orig_height;
    kdmenc->hdr.video_encoded_width = small_width;
    kdmenc->hdr.video_encoded_height = small_height;

    kdmenc->hdr.video_fps = (float)fps;

    kdmenc->video_enc_param_gop = gop;
    kdmenc->video_enc_param_fps = fps;
    kdmenc->video_enc_param_bitrate_kbps = bitrate_kbps;

    kdmenc->videoenc = kdm_videoenc_init(small_width, small_height, bitrate_kbps, gop, fps, kdmenc->pass_log, false);
    if (!kdmenc->videoenc) {
        return false;
    }

    frame_stepper_init(&kdmenc->stepper, small_width, small_height, fps);

    return true;
}

bool kdm_enc_set_audio_params(KDM_Encoder* kdmenc, int32_t reference_fps, int32_t frequency, bool is_stereo) {
    if (!(kdmenc->hdr.flags & KDM_FLAGS_HEADER_HAS_AUDIO)) {
        logger("kdm_set_audio_params() not allowed, the kdm does not have a audio stream\n");
        return false;
    }
    if (kdmenc->audioenc) {
        logger("kdm_set_audio_params() error, already called\n");
        return false;
    }

    kdmenc->hdr.audio_frequency = (uint16_t)frequency;
    KDM__CONFIGURE_FLAG(kdmenc->hdr.flags, KDM_FLAGS_HEADER_AUDIO_STEREO, is_stereo);

    kdmenc->audioenc = kdm_audioenc_init(frequency, is_stereo, reference_fps);
    kdmenc->audio_reference_fps = reference_fps;

    /*if (prebuffering) {
        size_t count = is_stereo ? AICA_BUFFER_SAMPLES_STEREO_SIZE : AICA_BUFFER_SAMPLES_MONO_SIZE;
        kdm_audioenc_enable_prebuffering(kdmenc->audioenc, count);
    }*/

    return kdmenc->audioenc != NULL;
}

bool kdm_enc_write_streams(KDM_Encoder* kdmenc, KDM_CB_ReadAudioSamples cb_audio, void* userdata_audio, KDM_CB_ReadVideoFrame cb_video, void* userdata_video, volatile bool* stop_signal) {
    if (!cb_audio && !cb_video) {
        logger("kdm_enc_write_streams() invalid operation, missing callbacks\n");
        return false;
    }
    if ((!cb_audio) != (!kdmenc->audioenc)) {
        logger("kdm_enc_write_streams() invalid operation, cb_audio is NULL or must be null\n");
        return false;
    }
    if ((!cb_video) != (!kdmenc->videoenc)) {
        logger("kdm_enc_write_streams() invalid operation, cb_video is NULL or must be null\n");
        return false;
    }

    if (!kdmenc->header_written) {
        if (fwrite(&kdmenc->hdr, sizeof(KDMFileHeader), 1, kdmenc->mediafile) != 1) {
            return false;
        }

        // reserve space for the cue table in the output
        int table_write_result = fwrite(
            kdmenc->cue_table, sizeof(KDMCue), kdmenc->cue_table_length, kdmenc->mediafile
        );
        if (table_write_result != kdmenc->cue_table_length) {
            return false;
        }

        kdmenc->header_written = true;
    }

    int16_t* audio_buffer = malloc(BUFFER_LENGTH * sizeof(int16_t) * 2);

    KDM_Audio* audioenc = kdmenc->audioenc;
    KDM_Video* videoenc = kdmenc->videoenc;
    FILE* mediafile = kdmenc->mediafile;
    FrameStepper* stepper = &kdmenc->stepper;
    bool is_stereo = (kdmenc->hdr.flags & KDM_FLAGS_HEADER_AUDIO_STEREO) != 0;

    KDMCue* cue_table = kdmenc->cue_table;
    uint16_t cue_table_length = kdmenc->cue_table_length;
    uint16_t cue_table_used = kdmenc->cue_table_used;

    bool read_audio = audioenc != NULL;
    bool read_video = videoenc != NULL;
    uint32_t last_audio_timestamp = 0;

    AVFrame* frame;
    int32_t samples;

    KDM_PacketResult v_ret;
    int32_t a_ret;

    KDMPacketHeader packet;
    long packet_offset;

    if (read_video) {
        frame_stepper_reset(stepper, cb_video, userdata_video);
    }

    while (read_audio || read_video) {
        a_ret = 0;
        v_ret = (KDM_PacketResult){.is_keyframe = false, .pts = 0.0};
        packet = (KDMPacketHeader){.packet_flags = 0x00, .video_data_size = 0, .audio_data_size = 0};

        // reserve space for packet header in the output
        packet_offset = ftell(mediafile);
        fwrite(&packet, sizeof(KDMPacketHeader), 1, mediafile);

        // read, encode and write audio samples
        while (read_audio) {
            samples = cb_audio(userdata_audio, audio_buffer, BUFFER_LENGTH);

            if (samples < 1) {
                // eof reached or error, flush encoder
                if (kdm_audioenc_has_buffered_samples(audioenc))
                    a_ret = kdm_audioenc_encode_samples(audioenc, NULL, 0, mediafile);
                else
                    a_ret = 0;

                read_audio = false;
            } else {
                a_ret = kdm_audioenc_encode_samples(audioenc, audio_buffer, samples, mediafile);

                if (a_ret != -2 && a_ret < 1) {
                    // wait until there enough encoded samples
                    continue;
                }
            }

            if (a_ret == -2) {
                logger("kdm_enc_write_streams() failed to audio samples\n");
                goto L_failed;
            }

            last_audio_timestamp = (kdmenc->samples_written * 1000.0 * 1000.0) / kdmenc->hdr.audio_frequency;
            packet.audio_data_size = a_ret;
            kdmenc->samples_written += is_stereo ? a_ret : (a_ret / 2);
            break;
        }

        // read, encode, compress and write video frame
        while (read_video) {
            frame = frame_stepper_fetch(stepper);

            if (!frame) {
                // EOF reached
                read_video = false;
                break;
            }

            v_ret = kdm_videoenc_encode_frame(videoenc, mediafile, frame);

            if (v_ret.needs_more_data) {
                continue;
            }

            if (!v_ret.write_success) {
                logger("kdm_enc_write_streams() failed to write video frame\n");
                goto L_failed;
            }

            KDM__CONFIGURE_FLAG(packet.packet_flags, KDM_FLAGS_PACKET_VIDEO_KEYFRAME, v_ret.is_keyframe);
            packet.video_data_size = v_ret.written_bytes;
            break;
        }

        // update packet header
        size_t packet_data_size = packet.video_data_size + packet.audio_data_size;

        fseek(mediafile, packet_offset, SEEK_SET);
        if (packet_data_size > 0) {
            fwrite(&packet, sizeof(KDMPacketHeader), 1, kdmenc->mediafile);
            fseek(mediafile, packet_data_size, SEEK_CUR);
        }

        // update cue table (if needed)
        if ((uint32_t)packet_offset < UINT32_MAX && cue_table_used < cue_table_length) {
            int64_t timestamp = -1.0;

            if (read_video) {
                if (v_ret.is_keyframe) {
                    timestamp = v_ret.pts * 1000.0 * 1000.0;
                }
            } else if (read_audio) {
                // the audio stream is longer, still add cue entry
                timestamp = last_audio_timestamp;
            }

            if (timestamp > kdmenc->next_cue_timestamp) {
                cue_table[cue_table_used++] = (KDMCue){
                    .timestamp = (uint32_t)timestamp,
                    .offset = (uint32_t)packet_offset
                };
                kdmenc->next_cue_timestamp += kdmenc->estimated_cue_interval;
            }
        }

        if (*stop_signal) {
            break;
        }
    }

    kdmenc->cue_table_used = cue_table_used;

    free(audio_buffer);
    return true;

L_failed:
    free(audio_buffer);
    return false;
}

bool kdm_enc_flush(KDM_Encoder* kdmenc) {
    long last_packet_header_offset = ftell(kdmenc->mediafile);
    if (last_packet_header_offset < 0) {
        return false;
    }

    KDMPacketHeader packet_header = {
        .packet_flags = 0x00,
        .video_data_size = 0,
        .audio_data_size = 0
    };
    if (fwrite(&packet_header, sizeof(KDMPacketHeader), 1, kdmenc->mediafile) != 1) {
        return false;
    }

    if (kdmenc->videoenc) {
        KDM_PacketResult ret = kdm_videoenc_flush(kdmenc->videoenc, kdmenc->mediafile);
        if (!ret.write_success) {
            return false;
        }
        if (ret.written_bytes > 0) {
            packet_header.video_data_size = ret.written_bytes;
        }
    }
    if (kdmenc->audioenc && kdm_audioenc_has_buffered_samples(kdmenc->audioenc)) {
        ssize_t written_audio_bytes = kdm_audioenc_encode_samples(kdmenc->audioenc, NULL, 0, kdmenc->mediafile);
        if (written_audio_bytes < 0) {
            return false;
        }
        packet_header.audio_data_size = written_audio_bytes;
    }

    long end_offset = ftell(kdmenc->mediafile);
    if (fseek(kdmenc->mediafile, last_packet_header_offset, SEEK_SET)) {
        return false;
    }

    if ((packet_header.video_data_size + packet_header.audio_data_size) > 0) {
        if (fwrite(&packet_header, sizeof(KDMPacketHeader), 1, kdmenc->mediafile) != 1) {
            return false;
        }
        if (fseek(kdmenc->mediafile, end_offset, SEEK_SET)) {
            return false;
        }
    } else {
        end_offset = last_packet_header_offset;
    }

    // trim cue table
    size_t used_entries = kdmenc->cue_table_used;

    // reoder packet offsets in the cue table
    if (kdmenc->cue_table_length != used_entries) {
        size_t offset_diff = (kdmenc->cue_table_length - used_entries) * sizeof(KDMCue);
        for (size_t i = 0; i < used_entries; i++) {
            kdmenc->cue_table[i].offset -= offset_diff;
        }
    }

    // seek and write cue_table
    long start_offset = sizeof(KDMFileHeader) + (kdmenc->cue_table_length * sizeof(KDMCue));
    if (fseek(kdmenc->mediafile, 0, SEEK_SET)) {
        return false;
    }

    kdmenc->hdr.cue_table_length = kdmenc->cue_table_used;
    if (fwrite(&kdmenc->hdr, sizeof(KDMFileHeader), 1, kdmenc->mediafile) != 1) {
        return false;
    }

    if (fwrite(kdmenc->cue_table, sizeof(KDMCue), used_entries, kdmenc->mediafile) != used_entries) {
        return false;
    }
    if (kdmenc->cue_table_used == kdmenc->cue_table_length) {
        if (fseek(kdmenc->mediafile, end_offset, SEEK_SET)) {
            return false;
        }
        return true;
    }

    // trim cue table and move packets
    kdmenc->cue_table_length = kdmenc->cue_table_used;

    size_t new_start_offset = sizeof(KDMFileHeader) + (used_entries * sizeof(KDMCue));
    uint8_t buffer[1024 * 1024];
    while (start_offset < end_offset) {
        if (fseek(kdmenc->mediafile, start_offset, SEEK_SET)) {
            return false;
        }

        size_t to_read = end_offset - start_offset;
        if (to_read > sizeof(buffer)) to_read = sizeof(buffer);

        size_t readed = fread(buffer, sizeof(uint8_t), to_read, kdmenc->mediafile);
        if (readed < 1) {
            return false;
        }

        if (fseek(kdmenc->mediafile, new_start_offset, SEEK_SET)) {
            return false;
        }
        if (fwrite(buffer, sizeof(uint8_t), readed, kdmenc->mediafile) != readed) {
            return false;
        }

        start_offset += readed;
        new_start_offset += readed;
    }

    return true;
}


bool kdm_enc_set_video_pass2(KDM_Encoder* kdmenc) {
    if (!kdmenc->videoenc) return true;

    frame_stepper_destroy(&kdmenc->stepper);
    frame_stepper_init(
        &kdmenc->stepper,
        kdmenc->hdr.video_encoded_width, kdmenc->hdr.video_encoded_height,
        kdmenc->video_enc_param_fps
    );

    kdm_videoenc_destroy(kdmenc->videoenc);

    kdmenc->videoenc = kdm_videoenc_init(
        kdmenc->hdr.video_encoded_width, kdmenc->hdr.video_encoded_height,
        kdmenc->video_enc_param_bitrate_kbps, kdmenc->video_enc_param_gop, kdmenc->video_enc_param_fps,
        kdmenc->pass_log, true
    );

    return !!kdmenc->videoenc;
}

bool kdm_enc_write_streams_pass1(KDM_Encoder* kdmenc, KDM_CB_ReadAudioSamples cb_audio, void* userdata_audio, KDM_CB_ReadVideoFrame cb_video, void* userdata_video, volatile bool* stop_signal) {
    if (!cb_audio && !cb_video) {
        logger("kdm_enc_write_streams() invalid operation, missing callbacks\n");
        return false;
    }
    if ((!cb_audio) != (!kdmenc->audioenc)) {
        logger("kdm_enc_write_streams() invalid operation, cb_audio is NULL or must be null\n");
        return false;
    }
    if ((!cb_video) != (!kdmenc->videoenc)) {
        logger("kdm_enc_write_streams() invalid operation, cb_video is NULL or must be null\n");
        return false;
    }

    int16_t* audio_buffer = malloc(BUFFER_LENGTH * sizeof(int16_t) * 2);

    KDM_Audio* audioenc = kdmenc->audioenc;
    KDM_Video* videoenc = kdmenc->videoenc;
    FrameStepper* stepper = &kdmenc->stepper;
    
    bool read_audio = audioenc != NULL;
    bool read_video = videoenc != NULL;

    AVFrame* frame;
    int32_t samples;

    KDM_PacketResult v_ret;

    if (read_video) {
        frame_stepper_reset(stepper, cb_video, userdata_video);
    }

    while (read_audio || read_video) {
        v_ret = (KDM_PacketResult){.is_keyframe = false, .pts = 0.0};

        // read audio samples
        while (read_audio) {
            samples = cb_audio(userdata_audio, audio_buffer, BUFFER_LENGTH);
            if (samples < 1) {
                read_audio = false;
            }
            break;
        }

        // read, encode, compress and write video frame
        while (read_video) {
            frame = frame_stepper_fetch(stepper);

            if (!frame) {
                // EOF reached
                read_video = false;
                break;
            }

            v_ret = kdm_videoenc_encode_frame(videoenc, NULL, frame);

            if (v_ret.needs_more_data) {
                continue;
            }

            if (!v_ret.write_success) {
                logger("kdm_enc_write_streams() failed to write video frame\n");
                goto L_failed;
            }

            break;
        }

        if (*stop_signal) {
            break;
        }
    }

    free(audio_buffer);
    return true;

L_failed:
    free(audio_buffer);
    return false;
}

bool kdm_enc_flush_pass1(KDM_Encoder* kdmenc) {
    if (kdmenc->videoenc) {
        KDM_PacketResult ret = kdm_videoenc_flush(kdmenc->videoenc, NULL);
        if (!ret.write_success) {
            return false;
        }
    }
    return true;
}
