# KDY-e Dreamcast Media

kdm encoder and test decoder sourcecode.

---

## About

The KDM media file uses the _MPEG-1_ as video codec and _Yamaha ADPCM_ as audio codec.

### Usuage

```text
KDM encoder v0.20 by kapodamy
Encodes media files (like mp4, mkv, avi, etc.) to kdm media format which uses MPEG-1 for video and YAMAHA 4-bit ADPCM for audio.

Usage: kdm_enc [options...] <input video file> <output video file>

Video options:
 -q, --hq                                       Uses a high bitrate, improves quality alot but can produce suttering (see notes)
 -d, --dither-algorithm <dither algorithm>      Dither algortihm used for video rescale. Default: auto
 -s, --scale-algorithm <scale algorithm>        Used only for downscaling. Default: lanczos
 -f, --fps <framerate>                          Frames per second, the maximum is 60. By default the original framerate is used (see notes)
 -c, --cue-interval <seconds>                   Used for seeking, lower values allow fast seeking. Default: 3
 -l, --small-resolution                         Downscale the video to 320x240 (4:3) or 480x270 (16:9), resolution choosen by aspect ratio
 -g, --default-gop                              Uses the default value 12 for GOP which defines the keyframe interval, instead of 15 or 18 for hq
 -p, --two-pass                                 Two-pass video encoding, can improve the video quality.
 -u, --height-hack                              Removes the bottom frame padding, making the file smaller. Can produce graphical glitches

Audio options:
 -n, --silence                                  Ignore the audio stream.
 -r, --sample-rate <frequency>                  Audio sample rate in hertz, recommended value is 32000. By default the original sample rate is used.
 -m, --mono                                     Downmix the audio stream to mono.

Dither algorithms:
    auto               leave choice to sws
    bayer              bayer dither
    ed                 error diffusion
    a_dither           arithmetic addition dither
    x_dither           arithmetic xor dither

Scale algorithms:
    fast_bilinear      fast bilinear
    bilinear           bilinear
    bicubic            bicubic
    experimental       experimental
    neighbor           nearest neighbor
    area               averaging area
    bicublin           luma bicubic, chroma bilinear
    gauss              Gaussian
    sinc               sinc
    lanczos            Lanczos
    spline             natural bicubic spline
    print_info         print info
    accurate_rnd       accurate rounding
    full_chroma_int    full chroma interpolation
    full_chroma_inp    full chroma input
    bitexact           
    error_diffusion    error diffusion dither

Notes:
* The GD-ROM read speed is slow, is the video resolution or fps are high can produce suttering.
* If the video resolution exceeds the 1024x1024 it will downscaled to 640x480 (letterbox) or 864x486 (widescreen), the output resolution is choosen depending on the video aspect ratio.
* For cue interval, low values allows fast seeking but the result file is slightly bigger and viceversa.
* Using low framerates, sample rate or resolution can improve the I/O performance.
* Audio channels are by default mono or stereo but multichannel audio is downmixed to stereo.


```

### Decoder

```text
KDM decoder (Proof-of-concept) v0.1 by kapodamy
Usage:
       kdm_dec <input kdm file> <output media filename>

Suggested output filename extensions are mkv and avi, the output codecs are HUFFYUV and FLAC.
The output file size can be up to 1GiB
The decoding does not have hardware acceleration in order to test the correct usage of pl_mpeg library

```

---

## Building

1. clone FFmpeg [sourcecode](https://ffmpeg.org/download.html "FFmpeg") into `include` folder, the required folders are: libavcodec, libavformat, libavutil, libswresample and libswscale.
2. run `./include/configure` (the **configure** script of FFmpeg), this will generate the missing header files.
3. place the following DLLs files into `lib`: avcodec-61.dll, avformat-61.dll, avutil-59.dll, swresample-5.dll and swscale-8.dll.
4. run make:
    a. encoder `make build-release-windows`
    b. decoder `make build-decoder-release-windows`

### Notes

- both encoder and decoder can be built for x86 or x64 architectures as long FFmpeg is correctly configured.
- DLLs can be downloaded from [https://www.gyan.dev/ffmpeg/builds/](https://www.gyan.dev/ffmpeg/builds/ "Builds - CODEX FFMPEG @ gyan.dev") (full shared builds) or from [https://github.com/BtbN/FFmpeg-Builds/releases](https://github.com/BtbN/FFmpeg-Builds/releases "BtbN - FFmpeg Static Auto-Builds") (*-shared.zip builds)
- only the FFmpeg headers are required, so any non *.h file can be deleted to save disk space.

## MPEG-1 decoder

**PL_MPEG** is used to decode the video stream [PL_MPEG - MPEG1 Video decoder, MP2 Audio decoder, MPEG-PS demuxer](https://github.com/phoboslab/pl_mpeg "PL_MPEG - MPEG1 Video decoder, MP2 Audio decoder, MPEG-PS demuxer). This is the same library used in the kdy-e as well.
