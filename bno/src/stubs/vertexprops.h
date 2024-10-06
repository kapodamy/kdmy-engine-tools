#ifndef _vertexprops_h
#define _vertexprops_h

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "float64.h"

//
//  all functions here are stub
//

static inline void trim_string(const char* str, size_t* out_start, size_t* out_end) {
    size_t index = 0;

    while (true) {
        char c = *str++;
        switch (c) {
            case '\0':
                *out_start = *out_end = 0;
                return;
            case '\r':
            case ' ':
            case '\n':
            case '\t':
            case '\v':
                index++;
                continue;
        }
        break;
    }

    *out_start = index;

    size_t str_length = strlen(str);
    size_t index_end = index + str_length;

    str += str_length - 1;
    while (index_end > index) {
        char c = *str--;
        switch (c) {
            case '\r':
            case '\x20':
            case '\n':
            case '\t':
            case '\v':
                index_end--;
                continue;
        }
        break;
    }

    *out_end = index_end + 1;
}

static inline bool vertexprops_parse_hex(const char* string, uint32_t* output_value, bool only_if_prefixed) {
    size_t start_index, end_index;
    trim_string(string, &start_index, &end_index);

    if (start_index == end_index) {
        return false;
    }

    if (string[start_index + 0] == '#') {
        start_index++;
    } else if (string[start_index + 0] == '0' && tolower(string[start_index + 1]) == 'x') {
        start_index += 2;
    } else if (only_if_prefixed) {
        return false;
    }

    if (start_index == end_index) {
        return false;
    }

    const char* expected_str_end = string + end_index;
    char* end_str;
    unsigned long value = strtoul(string + start_index, &end_str, 16);

    if (end_str != expected_str_end) {
        return false;
    }

    *output_value = value;
    return true;
}

#endif