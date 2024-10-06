#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lzss.h"


typedef struct {
    size_t offset;
    size_t size;
} LZSSP;

typedef struct {
    size_t value;
    bool success;
} LZSSR;


static inline void lzss_emit_token(void* buff, size_t off_rl, size_t psiz, size_t litr) {
    uint16_t* buffer = (uint16_t*)buff;
    uint16_t token;

    // pattern relative offset
    token = (off_rl & 0x3F) << 10;

    // pattern size
    token |= (psiz & 0x1F) << 5;

    // follow literal size
    token |= (litr & 0x1F) << 0;

    *buffer = token;
}

static inline void lzss_set_token_literal(void* buff, size_t litr) {
    uint16_t* buffer = (uint16_t*)buff;
    uint16_t token = *buffer;

    *buffer = (token & ~0x1F) | (litr & 0x1F);
}


static inline LZSSP lzss_find_pattern(uint8_t* data, size_t offset, size_t size) {
    size_t pattern_offset = 0, pattern_size = 0;
    size_t window_offset = offset < 0x5E ? 0 : (offset - 0x5E);

    for (; window_offset < offset; window_offset++) {
        size_t current_pattern_size = 0;

        for (size_t i = window_offset, j = offset; i < offset && j < size; i++, j++) {
            if (data[i] == data[j]) {
                current_pattern_size++;
                if (current_pattern_size >= 0x1F) break;
            } else {
                break;
            }
        }

        if (current_pattern_size > sizeof(uint16_t) && current_pattern_size > pattern_size) {
            // check if the window offset is valid
            size_t end = window_offset + current_pattern_size;

            if (end < offset) {
                // check if the pattern offset can be stored
                size_t test_offset = offset - end;

                if (test_offset <= 0x3F) {
                    // remember pattern
                    pattern_offset = test_offset;
                    pattern_size = current_pattern_size;
                }
            }
        }
    }

    if (pattern_size < 3) {
        // no pattern found
        return (LZSSP){0, 0};
    }

#if DEBUG
    assert(pattern_offset <= 0x3F);
    assert(pattern_size <= 0x1F);
#else
    if (pattern_offset > 0x3F || pattern_size > 0x1F) {
        // this should not happen
        return (LZSSP){0, 0};
    }
#endif

    return (LZSSP){.offset = pattern_offset, .size = pattern_size};
}

static inline LZSSR lzss_find_single_byte_repeats(uint8_t* data, size_t offset, size_t size) {
    uint8_t* data_ptr = data + offset;
    uint8_t* data_end = data + size;

    size_t occurrences = 0;
    uint8_t value = *data_ptr++;

    while (data_ptr < data_end && occurrences < 63) {
        if (*data_ptr != value) {
            break;
        }
        occurrences++;
        data_ptr++;
    }

    if (occurrences < 3) {
        return (LZSSR){.value = 0, .success = false};
    }

    return (LZSSR){.value = occurrences, .success = true};
}


void lzss_compress(uint8_t* data, size_t data_size, uint8_t** compressed_data, size_t* compressed_data_size) {
    size_t output_length = data_size + sizeof(uint32_t);
    uint8_t* output = malloc(output_length + 512);
    size_t output_offset = 0;
    size_t data_offset = 0;
    size_t readed_offset = 0;
    size_t unique_bytes_count = 0;
    uint16_t* last_pattern_ptr = NULL;

    // write data length at the start
    *((uint32_t*)output) = (uint32_t)data_size;
    output_offset += sizeof(uint32_t);

    while (data_offset < data_size) {
        LZSSP pattern = {0, 0};
        LZSSR repeats = {0, false};

        // find repeats of a single byte
        repeats = lzss_find_single_byte_repeats(data, data_offset, data_size);

        if (!repeats.success) {
            // find patterns at the current offset
            pattern = lzss_find_pattern(data, data_offset, data_size);
        }

        // check if necessary emit a literal
        if (repeats.success || pattern.size > 0) {
            unique_bytes_count = data_offset - readed_offset;

            if (readed_offset < 1 && unique_bytes_count > 0) {
                if ((output_offset + sizeof(uint16_t) + data_offset) >= output_length) {
                    // the compressed size exceds the uncompressed size
                    goto L_failed;
                }

                // emit start token with start data
                lzss_emit_token(output + output_offset, 0, 0, data_offset);
                output_offset += sizeof(uint16_t);

                memcpy(output + output_offset, data, data_offset);
                output_offset += data_offset;

                readed_offset = data_offset;
                unique_bytes_count = 0;
            } else if (unique_bytes_count > 0) {
                if ((output_offset + sizeof(uint16_t) + unique_bytes_count) >= output_length) {
                    // the compressed size exceds the uncompressed size
                    goto L_failed;
                }

                if (last_pattern_ptr) {
                    // append literal to the previous token
                    lzss_set_token_literal(last_pattern_ptr, unique_bytes_count);
                } else {
                    // the previous pattern already has a literal, emit a literal
                    lzss_emit_token(output + output_offset, 0, 0, unique_bytes_count);
                    output_offset += sizeof(uint16_t);
                }

                memcpy(output + output_offset, data + readed_offset, unique_bytes_count);

                output_offset += unique_bytes_count;
                readed_offset += unique_bytes_count;
            }
        }

        if (repeats.success) {
            lzss_emit_token(output + output_offset, repeats.value, 0, 0);
            output_offset += sizeof(uint16_t);

            *(output + output_offset) = *(data + data_offset);
            output_offset++;

            last_pattern_ptr = NULL;
            readed_offset += repeats.value;
            data_offset += repeats.value;
            unique_bytes_count = 0;
            continue;
        }

        if (pattern.size > 0) {
            lzss_emit_token(output + output_offset, pattern.offset, pattern.size, 0);

            last_pattern_ptr = (uint16_t*)(output + output_offset);
            output_offset += sizeof(uint16_t);
            data_offset += pattern.size;
            readed_offset += pattern.size;
            unique_bytes_count = 0;
            continue;
        }

        unique_bytes_count++;
        data_offset++;

        if (unique_bytes_count >= 0x1F || (data_offset >= data_size && unique_bytes_count > 0)) {
            if ((output_offset + sizeof(uint16_t) + unique_bytes_count) >= output_length) {
                // the compressed size exceds the uncompressed size
                goto L_failed;
            }

            if (last_pattern_ptr) {
                // append literal to the previous token
                lzss_set_token_literal(last_pattern_ptr, unique_bytes_count);
            } else {
                // since there no pattern or previous pattern, emit a literal
                lzss_emit_token(output + output_offset, 0, 0, unique_bytes_count);
                output_offset += sizeof(uint16_t);
            }

            // write literal
            memcpy(output + output_offset, data + readed_offset, unique_bytes_count);

            last_pattern_ptr = NULL;
            readed_offset += unique_bytes_count;
            output_offset += unique_bytes_count;
            unique_bytes_count = 0;
        }
    }

    if (output_offset <= sizeof(uint32_t)) {
        goto L_failed;
    }

    if (output_offset < output_length) {
        // shrink the buffer
        output = realloc(output, output_offset);
        assert(output);
    }

    *compressed_data = output;
    *compressed_data_size = output_offset;
    return;

L_failed:
    free(output);
    *compressed_data = NULL;
    *compressed_data_size = 0;
    return;
}

void lzss_decompress(uint8_t* compressed_data, size_t compressed_data_size, uint8_t** data, size_t* data_size) {
    uint32_t uncompressed_size = *((uint32_t*)compressed_data);
    uint8_t* uncompressed_data_start = malloc(uncompressed_size);
    size_t written = 0;
    uint8_t* compressed_data_end = compressed_data + compressed_data_size;
    compressed_data += sizeof(uint32_t);

    assert(uncompressed_data_start);
    uint8_t* uncompressed_data = uncompressed_data_start;

    while (compressed_data < compressed_data_end) {
        uint16_t token = *((uint16_t*)compressed_data);

        size_t pattern_offset = (token & 0xFC00) >> 10;
        size_t pattern_size = (token & 0x03E0) >> 5;
        size_t literal_size = (token & 0x001F) >> 0;

        compressed_data += sizeof(uint16_t);

        if (pattern_size == 0 && literal_size == 0) {
            uint8_t byte = *compressed_data;

            written += pattern_offset;
            if (written > uncompressed_size) {
                // something went wrong
                break;
            }

            memset(uncompressed_data, byte, pattern_offset);

            compressed_data++;
            uncompressed_data += pattern_offset;
            continue;
        }

        written += pattern_size + literal_size;
        if (written > uncompressed_size) {
            // something went wrong
            break;
        }

        if (pattern_size > 0) {
            size_t backward_offset = pattern_offset + pattern_size;
            memcpy(uncompressed_data, uncompressed_data - backward_offset, pattern_size);
            uncompressed_data += pattern_size;
        }


        if (literal_size > 0) {
            // copy literal (no compressed bytes)
            memcpy(uncompressed_data, compressed_data, literal_size);
            compressed_data += literal_size;
            uncompressed_data += literal_size;
        }
    }

    if ((written < 1 && uncompressed_size > 0) || written > uncompressed_size) {
        free(uncompressed_data_start);
        uncompressed_size = 0;
        uncompressed_data_start = NULL;
    }

    *data = uncompressed_data_start;
    *data_size = uncompressed_size;
}
