# SFX Tool

This tool encodes short OGG sounds into WAV with the YAMAHA 4-bit ADPCM (or PCM-U8) audio codec. This offloads the vorbis decoding from the CPU to the AICA which has "hardware acceleration" for adpcm/pcm formats.
The amount of samples can be stored in the audio RAM is limited to 65535 samples, however if the sound is a little longer, the sampling rate can be reduced until it reaches 65535 samples.
It is recommended to convert all short sounds to WAV because there are only 4 stream slots available and 2 of them are typically used for instrumental and vocal tracks. This is a limitation of [KallistiOS](https://github.com/KallistiOS/KallistiOS/blob/master/kernel/arch/dreamcast/include/dc/sound/stream.h#L40 "stream.h file · KallistiOS")
__Note:__ reducing the sample rate frequency decreases audio quality.

## Usage

```text
SFX v0.1 by kapodamy
Encodes short sound files (mostly ogg vorbis) into Wav media format and uses YAMAHA 4-bit ADPCM or PCM-U8 as audio codec.
The encoder checks if the supplied sound is eligible using heuristics (see notes).
Loop points are supported for Ogg (from comments) and Wav (from smpl/cue chunks).

Usage: sfx [options...] <input audio file> <output wav file>

Options:
 -d, --max-duration    [milliseconds]       Maximum sound duration, use 0 to guess automatically. Default: 0
 -r, --sample-rate     [hertz]              Sample rate frequency, use 0 to keep the original sample rate. Default: original sample rate or 16000 if '--auto-samplerate' is present
 -a, --auto-sample-rate                     Reduce sample rate until all encoding criterias are met, but not below '--sample-rate' value.
 -p, --pcm-u8                               Use 'unsigned 8-bit PCM', better audio quality but increases x2 the result file size.
 -m, --force-mono                           Downmix to mono, by default always is downmixed to stereo.
 -c, --copy-if-rejected                     Copies the file in the output folder as-is if the sound is not eligible.
 -t, --test-only                            Display the output metadata (channels, samples, format and duration) without writing any file.

Maximum duration for the most commonly used sample rates:
*  48000Hz      1.365ms     1 seconds
*  44100Hz      1.486ms         "" 
*  32000hz      2.047ms     2 seconds
*  24000hz      2.730ms         "" 
*  16000Hz      4.095ms     4 seconds
*  12000Hz      5.461ms     5 seconds
*  8000Hz       8.191ms     8 seconds
*  4000Hz       16.383ms    16 seconds

Notes:
* The AICA only contains 64 channels available, but in the practice only nearly 26 can be used for sound effects.
* The maximum amount of audio samples can be stored on AICA RAM is 65535, thats it.
* The reason why are limited to 65535 is because AICA uses a 16-bit register to store the samples count.
* The amount of samples is calculated as:    samples = duration_in_seconds / frecuency_in_hertz.
* On sounds with more than 65535 samples the sample rate is reduced, this decrease audio quality.
* The sample rate is always reduced in steps/multiple of 4 because the AICA stores the frequency in floating-point format (citation needed), arbitrary values can lead to frequency missmatch (not enough precission).
* Audio channels are by default mono or stereo but multichannel audio is always downmixed to stereo.
* Loop points are updated (if present) when the sample rate is changed.
```

---

## Building

1. clone FFmpeg [sourcecode](https://ffmpeg.org/download.html "FFmpeg") into `include` folder, the required folders are: libavcodec, libavformat, libavutil, libswresample and libswscale.
2. run `./include/configure` (the __configure__ script of FFmpeg), this will generate the missing header files.
3. place the following DLLs files into `lib`: avcodec-60.dll, avformat-60.dll, avutil-58.dll and swresample-4.dll.
4. run `make build-release-windows`

### Notes

- the encoder can be built for x86 or x64 architectures as long FFmpeg is correctly configured.
- DLLs can be downloaded from [https://www.gyan.dev/ffmpeg/builds/](https://www.gyan.dev/ffmpeg/builds/ "Builds - CODEX FFMPEG @ gyan.dev") (full shared builds) or from [https://github.com/BtbN/FFmpeg-Builds/releases](https://github.com/BtbN/FFmpeg-Builds/releases "BtbN - FFmpeg Static Auto-Builds") (*-shared.zip builds)
- only the FFmpeg headers are required, so any non *.h file can be deleted to save disk space.
