#include <assert.h>
#include <string.h>

#include "twopass_buffer.h"


#define INITIAL_BUFFER_CAPACITY (64 * 1024 * 1024) // 64MiB
#define BUFFER_INCREMENT (1 * 1024 * 1024)         // 1MiB


TwoPassLogBuffer* twopass_log_init() {
    TwoPassLogBuffer* twopass = malloc(sizeof(TwoPassLogBuffer));
    assert(twopass);

    *twopass = (TwoPassLogBuffer){
        .used = 0,
        .length = INITIAL_BUFFER_CAPACITY,
        .buffer = malloc(INITIAL_BUFFER_CAPACITY)
    };

    assert(twopass->buffer);
    twopass->buffer[0] = '\0';

    return twopass;
}

void twopass_log_destroy(TwoPassLogBuffer* twopass) {
    free(twopass->buffer);
    free(twopass);
}

void twopass_log_append(TwoPassLogBuffer* twopass, const char* stats) {
    if (!stats || stats[0] == '\0') return;

    size_t stats_length = strlen(stats) + 1;
    size_t used = twopass->used + stats_length;

    if (used > twopass->length) {
        twopass->length = (used + (BUFFER_INCREMENT - 1)) / BUFFER_INCREMENT;
        twopass->buffer = realloc(twopass->buffer, twopass->length);
        assert(twopass->buffer);
    }

    memcpy(twopass->buffer + twopass->used, stats, stats_length);
    twopass->used = used - 1;
}
