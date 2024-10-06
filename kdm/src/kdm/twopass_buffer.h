#ifndef _twopass_buffer_h
#define _twopass_buffer_h

#include <stdlib.h>

typedef struct _TwoPassLogBuffer {
    char* buffer;
    size_t used;
    size_t length;
} TwoPassLogBuffer;

TwoPassLogBuffer* twopass_log_init();
void twopass_log_destroy(TwoPassLogBuffer* twopass);
void twopass_log_append(TwoPassLogBuffer* twopass, const char* stats);


#endif
