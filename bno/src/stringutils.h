#ifndef _stringutils_h
#define _stringutils_h

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//
//  all functions here are stub
//

typedef struct {
    uint8_t size;
    uint32_t code;
} Grapheme;


static inline bool string_decode_utf8_character(const char* string, int32_t index, Grapheme* grapheme) {
    size_t string_length = strlen(string);

    if (string_length < 1) return false; // invalid index

    string += index;
    uint8_t code = (uint8_t)*string;

    if ((code & 0x80) == 0) {
        grapheme->code = code; // ASCII character
        grapheme->size = 1;
        return true;
    }

    uint8_t count;
    uint32_t value;

    if ((code & 0xF8) == 0xF0) {
        count = 3;
        value = code & 0x07;
    } else if ((code & 0xF0) == 0xE0) {
        count = 2;
        value = code & 0x0F;
    } else if ((code & 0xE0) == 0xC0) {
        count = 1;
        value = code & 0x1F;
    } else {
        return false; // invalid encoding
    }

    if (count >= string_length) return false; // invalid index or character

    // count the first byte
    string++;

    for (uint8_t i = 0; i < count; i++) {
        uint32_t code = (uint8_t)*string;
        if ((code & 0xC0) != 0x80) return false; // invalid encoding

        value <<= 6;
        value |= (code & 0x3F);
        string++;
    }

    grapheme->size = count + 1;
    grapheme->code = value;

    return true;
}


static inline char* string_duplicate(const char* string) {
    if (!string) return NULL;

    size_t length = strlen(string) + 1;

    char* string_copy = malloc(length);
    assert(string_copy);

    memcpy(string_copy, string, length);
    return string_copy;
}

static inline bool string_equals(const char* str1, const char* str2) {
    if (str1 == str2) return true; // same pointer
    if (!str1 && !str2) return true;
    if (!str1 || !str2) return false;

    return strcmp(str1, str2) == 0;
}


#endif