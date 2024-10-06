# KDY-e Dreamcast Texture

kdt encoder and test decoder sourcecode.

---

## About

The KDT file is a 16bit texture format, can hold non power-of-two images and textures up to 8192x8192. The maximum texture size is 1024x1024, any image or texture that exceeds this dimension will be downscaled.
__Note:__ spritesheets bigger than 1024x1024 should have a decent spacing between sprites to avoid precission loss in the UV coordinates.

### Encoder usage

```text
KDT encoder v0.4 by kapodamy
Encodes image files (like png, jpg, bmp, ico, etc.) into KDT texture format, also allows features like PVR-VQ, twiddle, and frame wrapping for non power-of-two dimmensions.
To read ico and dds image files the ImageMagick binary executable must be present with the filename ".\convert.exe" or ".\magick.exe".
Mipmapping is not implemented

Usage: kdt_enc [options...] <input image file> <output image file>

Common options:
 -q, --quality <steps>                          Vector quantization steps, higher values allows better quality. Default: 500
 -d, --dither-algorithm <dither algorithm>      Dither algortihm used for pixel format conversion. Default: auto
 -s, --scale-algorithm <scale algorithm>        Used only if the image needs downscaling. Default: lanczos
 -g, --rgb565                                   Use RGB565 pixel format instead of YUV422, this can reduce artifacts but produces color banding.
 -v, --vq                                       Compress texture using vector quantization, lossy format but saves a lot of VRAM.
 -t, --no-twiddled                              Disable pixel twiddling, this decreases rendering performance.
 -z, --lzss                                     Compress the file with LZSS, only improves read and parse times. Does not affect the VRAM usage
Slice options:
 -e, --uper-slice                               Store high-resolution images (bigger than 1024x1024) as encoded chunks, this reduces the amount of downscaling needed but requires more VRAM.
 -b, --uper-slice-max-blocks <integer>          Maximum slice frame dimmension, used with '--uper-slice'. default=3 minimum=2(2048x2048) maximum=3(3072x3072)
 -r, --sub-slice                                Allow slice right-bottom image borders in order to add less frame padding, this reduces VRAM usage. Can be used with '--uper-slice' option
 -o, --opacity-slice                            Ignores right and bottom 'transparency padding' if exists, this can reduce VRAM usage on power-of-two spritesheets.
Other VQ options (not compatible with slice):
 -m, --force-vq-on-small                        Forces vector quantization on textures smaller than 64x64, waste of space.
 -u, --force-square-vq                          Always wrap texture with square dimmensions (example 128x128), applies only if vector quantization is needed.
Advanced options:
 -p, --downscale-procedure <type>               Specifies how images bigger than 1024x1024 should be downscaled. Default: CLAMP
 -x, --pixel-format <pixel format>              Output pixel format. Default: AUTO
Resize options:
 -f, --scale-factor <number>                    Ignores the downscale procedure and resize with the specified amount of times. Default: 0.0
 -l, --scale-factor-limit <integer>             Do not resize if the image size is less than the specified dimmension, used with '--scale-factor'. Default: 16
 -a, --max-dimmen <integer>                     Resize images to specific dimension while keeping the aspect ratio, must be an even number. Applies only if the width and/or height exceeds the dimmension, can not be used with '--scale-factor' option. Default: 0 (disabled)
ICO/DDS options:
 -e, --image-magick-exec <path>                 Path to ImageMagick executable, only needed for decoding dds and ico files. Default: .\convert.exe or .\magick.exe


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

Downscale procedures:
    CLAMP              Resizes textures with dimensions 2048x2048, 4096x4096 and 8192x8192 to 1024x1024.
    MEDIUM             Resizes specific texture sizes: 1024x1024->256x256, 2048x2048->512x512, 4096x4096->1024x1024 and 8192x8192->1024x1024
    HARD               Resizes specific texture sizes: 256x256->128x128, 512x512->256x256, 1024x1024->256x256, 2048x2048->512x512, 4096x4096->512x512 and 8192x8192->1024x1024
Note: all procedures are relaxed when '--uper-slice' option is used.

Pixel formats (all of them are 16bit):
    AUTO               The encoder automatically chooses a pixel format
    YUV422             Offers higher percieved quality than RGB formats, no transparency
    RGB565             Classic RGB format, no transparency
    ARGB1555           RGB with simple transparency, each pixel can be transparent or opaque
    ARGB4444           Full ARGB

Notes:
* For faster VQ compression, use tiny steps like 1, 2, 5.
* The PVR VQ (vector quantization) can produce files 4 times smaller and saves alot of VRAM (the available amount of VRAM is 8MiB).
* The encoder automatically chooses the output pixel format by checking the amount of transparency on the image.
* The piority order (to automatically choose a pixel format) is YU422/RGB565 -> ARGB1555 ->ARGB4444.
* No power-of-two images are wrapped into power-of-two dimensions, this allow render using the original size.
* If the image is downscaled, the scale factor is stored in the KDT texture file to allow rendering with the original size.
* The maximum image size is 8192x8192 otherwise will be rejected.
* The maximum texture size supported by the PowerVR GPU is 1024x1024, thats why downscale procedures are needed.
* The VRAM needed is always the output file size minus 28 bytes.
* For ico files, always the 128x128 icon image is used otherwise the biggest resolution one.
* There some edge-cases where '--sub-slice' does not take effect, for example an 640x788 resolution.
* The '--uper-slice-max-blocks' is limited to 3 because increases VRAM usage, for example a 3072x3072 VQ-compressed image will require 2.74MiB of VRAM.
* Sliced textures (with '--uper-slice' and/or '--sub-slice' option) can introduce line artifacts, a visible gap between texture chunks.
* LZSS file compression will be disabled if the compression ratio is less than 20%.
```

---

### Decoder usage

```text
KDT decoder v0.20 by kapodamy
Proof-of-Concept decoder, decode KDT texture files into PNG images. Mipmapping is not implemented

Usage: kdt_dec [options...] <input kdt file> <output png file>

Common options:
 -s, --scale-algorithm <scale algorithm>        Used only if the image needs upscaling. Default: lanczos
 -f, --ignore-scale-factor                      Do not upscale to the original image size, decode as-is.
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
```

---

## Building

1. clone FFmpeg [sourcecode](https://ffmpeg.org/download.html "FFmpeg") into `include` folder, the required folders are: libavcodec, libavformat, libavutil, libswresample and libswscale.
2. run `./include/configure` (the __configure__ script of FFmpeg), this will generate the missing header files.
3. place the following DLLs files into `lib`: avcodec-60.dll, avformat-60.dll, avutil-58.dll and swscale-7.dll.
4. run make:
    a. encoder `make build-release-windows`
    b. decoder `make build-decoder-release-windows`

### Notes

- both encoder and decoder can be built for x86 or x64 architectures as long FFmpeg is correctly configured.
- DLLs can be downloaded from [https://www.gyan.dev/ffmpeg/builds/](https://www.gyan.dev/ffmpeg/builds/ "Builds - CODEX FFMPEG @ gyan.dev") (full shared builds) or from [https://github.com/BtbN/FFmpeg-Builds/releases](https://github.com/BtbN/FFmpeg-Builds/releases "BtbN - FFmpeg Static Auto-Builds") (*-shared.zip builds)
- only the FFmpeg headers are required, so any non *.h file can be deleted to save disk space.

## Special Thanks

- __TapamN__ from [https://dcemulation.org/phpBB/viewtopic.php?t=106138](https://dcemulation.org/phpBB/viewtopic.php?t=106138 "pvrtex - PVR Texture Encoder - DCEmulation") for the discovery of the "Enhanced LBG Algorithm Based".
