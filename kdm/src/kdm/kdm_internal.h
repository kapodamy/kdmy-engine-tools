#ifndef _kdm_internal_h
#define _kdm_internal_h

#include <stdbool.h>

#include <libavutil/frame.h>

typedef struct _KDM_Audio KDM_Audio;
typedef struct _KDM_Video KDM_Video;
typedef struct _TwoPassLogBuffer TwoPassLogBuffer;

typedef struct {
    size_t written_bytes;
    bool write_success;
    bool is_keyframe;
    bool needs_more_data;
    double pts;
} KDM_PacketResult;

typedef enum {
    KDM_EncodePass_PASS1,
    KDM_EncodePass_PASS2,
    KDM_EncodePass_PASS3
} KDM_EncodePass;


KDM_Audio* kdm_audioenc_init(int32_t rate, bool is_stereo, int32_t fps);
void kdm_audioenc_destroy(KDM_Audio* audioenc);
ssize_t kdm_audioenc_encode_samples(KDM_Audio* audioenc, int16_t* pcm16sle, size_t samples, FILE* mediafile);
bool kdm_audioenc_has_buffered_samples(KDM_Audio* audioenc);
void kdm_audioenc_enable_prebuffering(KDM_Audio* audioenc, size_t sample_amount);

KDM_Video* kdm_videoenc_init(int32_t width, int32_t height, int bitrate_kbps, int gop, double target_fps, TwoPassLogBuffer* pass2_log, bool pass2);
void kdm_videoenc_destroy(KDM_Video* videoenc);
KDM_PacketResult kdm_videoenc_encode_frame(KDM_Video* videoenc, FILE* mediafile, AVFrame* image);
KDM_PacketResult kdm_videoenc_flush(KDM_Video* videoenc, FILE* mediafile);

struct _KDM_Encoder;
typedef struct _KDM_Encoder KDM_Encoder;

/**
 * Callback used to fetch pcm16sle (16bit signed little-endian) samples
 * @param userdata passed to the callback
 * @param buffer buffer where the samples will be readed
 * @param max_samples_per_channel the requested samples count
 * @returns the amount of samples readed or <1 to indicate EOF
 */
typedef int32_t (*KDM_CB_ReadAudioSamples)(void* userdata, int16_t* buffer, int32_t max_samples_per_channel);

/**
 * Callback used to fetch a single video frame (uncompressed), the pixel format must match
 * the format passed to kdm_enc_set_video_params2() function. It should be RGB565 or UYVY422 (YUV422 little-endian)
 * @param userdata passed to the callback
 * @param out_frame pointer to a buffer containing the frame data
 * @returns The frame PTS (presentation time) in seconds or -1 to indicate EOF. First frame PTS is always 0
 */
typedef double (*KDM_CB_ReadVideoFrame)(void* userdata, AVFrame** out_frame);

KDM_Encoder* kdm_enc_init(bool has_video, bool has_audio, const char* mediafile, double estimated_duration, int32_t cue_interval);
void kdm_enc_destroy(KDM_Encoder* kdmenc);
bool kdm_enc_set_video_params(KDM_Encoder* kdmenc, int32_t small_width, int32_t small_height, int32_t orig_width, int32_t orig_height, double fps, int32_t bitrate_kbps, int gop);
bool kdm_enc_set_audio_params(KDM_Encoder* kdmnc, int32_t reference_fps, int32_t frequency, bool is_stereo);
bool kdm_enc_write_streams(KDM_Encoder* kdmenc, KDM_CB_ReadAudioSamples cb_audio, void* userdata_audio, KDM_CB_ReadVideoFrame cb_video, void* userdata_video, volatile bool* stop_signal);
bool kdm_enc_flush(KDM_Encoder* kdmenc);
bool kdm_enc_set_video_pass2(KDM_Encoder* kdmenc);
bool kdm_enc_write_streams_pass1(KDM_Encoder* kdmenc, KDM_CB_ReadAudioSamples cb_audio, void* userdata_audio, KDM_CB_ReadVideoFrame cb_video, void* userdata_video, volatile bool* stop_signal);
bool kdm_enc_flush_pass1(KDM_Encoder* kdmenc);

#endif