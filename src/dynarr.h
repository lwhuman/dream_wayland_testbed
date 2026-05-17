#ifndef D_DYNAMIC_ARRAY
#define D_DYNAMIC_ARRAY

#include <stddef.h>

typedef struct _dyn_arr_header {
    size_t count;
    size_t capacity;
} _dyn_arr_header;

#define dyn_arr_len(arr) ((_dyn_arr_header *)(arr) - 1)->count

#define DYN_ARR_INIT_CAPACITY 256

#define dyn_arr_push(arr, n)                                                  \
    {                                                                         \
        do {                                                                  \
            if ((arr) == nullptr) {                                           \
                _dyn_arr_header *header = malloc(                             \
                    sizeof(*arr) * DYN_ARR_INIT_CAPACITY +                    \
                    sizeof(_dyn_arr_header)                                   \
                );                                                            \
                header->count    = 0;                                         \
                header->capacity = DYN_ARR_INIT_CAPACITY;                     \
                arr              = (void *)(header + 1);                      \
            }                                                                 \
            _dyn_arr_header *header = (_dyn_arr_header *)(arr) - 1;           \
            if (header->count >= header->capacity) {                          \
                header->capacity *= 2;                                        \
                header = realloc(                                             \
                    header,                                                   \
                    sizeof(*arr) * header->capacity + sizeof(_dyn_arr_header) \
                );                                                            \
                arr = (void *)(header + 1);                                   \
            }                                                                 \
            (arr)[header->count++] = (n);                                     \
        } while (0);                                                          \
    }

#define dyn_arr_pop(arr)                                              \
    {                                                                 \
        do {                                                          \
            _dyn_arr_header *header = ((_dyn_arr_header *)(arr) - 1); \
            if (header->count > 0) {                                  \
                header->count--;                                      \
            }                                                         \
        } while (0);                                                  \
    }

#define dyn_arr_swap_remove(arr, value)                   \
    {                                                     \
        size_t last_index = dyn_arr_len((arr)) - 1;       \
        for (size_t i = 0; i < dyn_arr_len((arr)); ++i) { \
            if ((arr)[i] == (value)) {                    \
                (arr)[i] = (arr)[last_index];             \
                dyn_arr_pop((arr));                       \
            }                                             \
        }                                                 \
    }

#define dyn_arr_last_index(arr) (dyn_arr_len((arr)) - 1)

#define dyn_arr_free(arr) free((_dyn_arr_header *)(arr) - 1);

#endif // !D_DYNAMIC_ARRAY
