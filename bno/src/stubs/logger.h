#ifndef _logger_h
#define _logger_h

#include <stdarg.h>
#include <stdio.h>


static inline void logger_error(const char* fmt, ...) {
    va_list argv;
    va_start(argv, fmt);
    fputs("[e] ", stdout);
    vfprintf(stdout, fmt, argv);
    fputc('\n', stdout);
    va_end(argv);
}

static inline void logger_warn(const char* fmt, ...) {
    va_list argv;
    va_start(argv, fmt);
    fputs("[!] ", stdout);
    vfprintf(stdout, fmt, argv);
    fputc('\n', stdout);
    va_end(argv);
}

static inline void logger_info(const char* fmt, ...) {
    va_list argv;
    va_start(argv, fmt);
    fputs("[i] ", stdout);
    vfprintf(stdout, fmt, argv);
    fputc('\n', stdout);
    va_end(argv);
}

static inline void logger_log(const char* fmt, ...) {
    va_list argv;
    va_start(argv, fmt);
    fputs("[~] ", stdout);
    vfprintf(stdout, fmt, argv);
    fputc('\n', stdout);
    va_end(argv);
}

#endif
