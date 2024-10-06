#ifndef _magick_h

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        #define MAGICK_ImageMagick_EXECUTABLE ".\\magick.exe"
        #define MAGICK_ImageMagick_CONVERT ".\\convert.exe"
#elif defined(unix) || defined(__unix) || defined(__unix__)
        #define MAGICK_ImageMagick_EXECUTABLE "./magick"
        #define MAGICK_ImageMagick_CONVERT "./convert"
#elif defined(__APPLE__) || defined(__MACH__)
        #error "not supported, place here the ImageMagick executable names"
#elif defined(__linux__)
        #define MAGICK_ImageMagick_EXECUTABLE "magick"
        #define MAGICK_ImageMagick_CONVERT "convert"
#elif defined(__FreeBSD__)
        #error "not supported, place here the ImageMagick executable names"
#elif (__ANDROID__)
        #error "not supported, place here the ImageMagick executable names"
#else
        #error "not supported, place here the ImageMagick executable names"
#endif

#endif
