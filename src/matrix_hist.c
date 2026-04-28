#include "matrix_hist.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

static int read_matrix_header(FILE *f, MatrixHeader *h) {
    return fread(h, sizeof(MatrixHeader), 1, f) == 1 ? 0 : -1;
}

// Main worker function
int process_file_hist(const char *path, int window_size, uint64_t *hist, int bins, uint64_t *pixels_out) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return -1;
    }
    MatrixHeader h;
    if (read_matrix_header(f, &h) != 0) {
        fclose(f);
        return -1;
    }
    if (h.channels != 1 && h.channels != 3) {
        fclose(f);
        return -1;
    }

    uint64_t total_pixels = (uint64_t)h.rows * (uint64_t)h.cols;
    uint64_t chunk_pixels = 1ULL << 20; // 2^20 = 1,048,576 Instead of loading the whole matrix into memory, this code reads it piece by piece.
    uint64_t chunk_bytes = chunk_pixels * h.channels;
    uint8_t *buf = (uint8_t *)malloc((size_t)chunk_bytes);
    if (!buf) {
        fclose(f);
        return -1;
    }

    uint64_t processed = 0;
    while (processed < total_pixels) {
        uint64_t remaining = total_pixels - processed;
        uint64_t this_pixels = remaining < chunk_pixels ? remaining : chunk_pixels; // For the last chunk if the remaining is less than a chunk buffer
        uint64_t this_bytes = this_pixels * h.channels;
        size_t got = fread(buf, 1, (size_t)this_bytes, f);
        if (got != (size_t)this_bytes) {
            free(buf);
            fclose(f);
            return -1;
        }

        for (uint64_t i = 0; i < this_pixels; i++) {
            uint8_t r, g, b;
            if (h.channels == 1) {
                r = g = b = buf[i];
            } else {
                uint64_t j = i * 3; // Becuase every pixel is r, g, b. That is why we multiply by 3
                r = buf[j];
                g = buf[j + 1];
                b = buf[j + 2];
            }
            
            // Defining which pixels belong to which bins in the histogram
            int rb = r / window_size;
            int gb = g / window_size;
            int bb = b / window_size;
            
            // Safety check
            if (rb >= bins) rb = bins - 1;
            if (gb >= bins) gb = bins - 1;
            if (bb >= bins) bb = bins - 1;

            // Flatten the 3D histogram into a 1D array
            int idx = (rb * bins + gb) * bins + bb;
            hist[idx]++;
        }
        processed += this_pixels;
    }

    free(buf);
    fclose(f);
    *pixels_out = total_pixels;
    return 0;
}
