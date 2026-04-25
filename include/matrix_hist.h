#ifndef MATRIX_HIST_H
#define MATRIX_HIST_H

#include <stdint.h>

int process_file_hist(const char *path, int window_size, uint64_t *hist, int bins, uint64_t *pixels_out);

#endif
