#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct {
    uint32_t rows;
    uint32_t cols;
    uint8_t channels;
} MatrixHeader;

typedef struct {
    char **items;
    int count;
    int cap;
} StringList;

#endif
