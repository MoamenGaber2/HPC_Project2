#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Stores metadata for the matrix generator
typedef struct {
    uint32_t rows;
    uint32_t cols;
    uint8_t channels;
} MatrixHeader;

// Dynamic list of strings used to scan .bin files
typedef struct {
    char **items; // Array of strings
    int count; // How many strings are currently stored
    int cap; // Current capacity
} StringList;

#endif
