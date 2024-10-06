#ifndef _malloc_chk_h
#define _malloc_chk_h

#include <malloc.h>

#include "logger.h"


#define malloc_chk malloc
#define free_chk free
#define realloc_chk realloc

#define malloc_warn2(type_name) logger_error("Failed to allocate '%s' type, out-of-memory.", type_name)

#define assert_for_array_type(VARIABLE, TYPE)                         \
    if (!(VARIABLE)) {                                                \
        malloc_warn2(#TYPE "[]");                                     \
        assert(!VARIABLE); \
    }

#define malloc_for_array(TYPE, ELEMENTS_COUNT) ({                               \
    void* ___tmp_array_ptr;                                                     \
    if (ELEMENTS_COUNT > 0) {                                                   \
        ___tmp_array_ptr = malloc_chk(sizeof(TYPE) * (size_t)(ELEMENTS_COUNT)); \
        assert_for_array_type(___tmp_array_ptr, TYPE);                          \
    } else {                                                                    \
        ___tmp_array_ptr = NULL;                                                \
    }                                                                           \
    ___tmp_array_ptr;                                                           \
})

#define realloc_for_array(ARRAY_PTR, ELEMENTS_COUNT, TYPE) ({                                   \
    void* ___tmp_array_ptr;                                                                     \
    if (ELEMENTS_COUNT > 0) {                                                                   \
        if (ARRAY_PTR != NULL) {                                                                \
            ___tmp_array_ptr = realloc_chk(ARRAY_PTR, sizeof(TYPE) * (size_t)(ELEMENTS_COUNT)); \
        } else {                                                                                \
            ___tmp_array_ptr = malloc_chk(sizeof(TYPE) * (size_t)(ELEMENTS_COUNT));             \
        }                                                                                       \
        assert_for_array_type(___tmp_array_ptr, TYPE);                                          \
    } else {                                                                                    \
        if (ARRAY_PTR != NULL) {                                                                \
            free_chk(ARRAY_PTR);                                                                \
        }                                                                                       \
        ___tmp_array_ptr = NULL;                                                                \
    }                                                                                           \
    ___tmp_array_ptr;                                                                           \
})

#endif
