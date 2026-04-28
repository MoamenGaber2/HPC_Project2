#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#define MKDIR(path) mkdir(path, 0755)
#endif

typedef struct {
    uint32_t rows;
    uint32_t cols;
    uint8_t channels;
} MatrixHeader;

static int ensure_dir(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return 0;
    }
    return MKDIR(path);
}

static uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

int main(int argc, char **argv) {
    if (argc < 7) {
        // Runs like this: ./generate_matrices data 100 256 256 3 42
        fprintf(stderr, "Usage: %s <output_dir> <count> <rows> <cols> <channels(1|3)> <seed>\n", argv[0]);
        return 1;
    }
    const char *output_dir = argv[1];
    int count = atoi(argv[2]);
    uint32_t rows = (uint32_t)atoi(argv[3]);
    uint32_t cols = (uint32_t)atoi(argv[4]);
    uint8_t channels = (uint8_t)atoi(argv[5]);
    uint32_t seed = (uint32_t)atoi(argv[6]);

    if (!(channels == 1 || channels == 3) || count <= 0 || rows == 0 || cols == 0) {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }
    if (ensure_dir(output_dir) != 0) {
        fprintf(stderr, "Failed creating output_dir: %s\n", output_dir);
        return 1;
    }

    uint64_t pixels = (uint64_t)rows * (uint64_t)cols;
    uint64_t bytes = pixels * channels;
    uint8_t *buffer = (uint8_t *)malloc((size_t)bytes);
    if (!buffer) {
        fprintf(stderr, "Out of memory allocating matrix buffer.\n");
        return 1;
    }

    uint32_t rng = seed ? seed : 123456789u;
    for (int i = 0; i < count; i++) {
        for (uint64_t j = 0; j < bytes; j++) {
            buffer[j] = (uint8_t)(xorshift32(&rng) & 0xFFu); // Generates random values between 0 and 255
        }
        char path[1024];
        snprintf(path, sizeof(path), "%s/matrix_%07d.bin", output_dir, i);
        FILE *f = fopen(path, "wb");
        if (!f) {
            fprintf(stderr, "Failed opening %s for writing\n", path);
            free(buffer);
            return 1;
        }
        MatrixHeader h;
        h.rows = rows;
        h.cols = cols;
        h.channels = channels;
        if (fwrite(&h, sizeof(h), 1, f) != 1 || fwrite(buffer, 1, (size_t)bytes, f) != (size_t)bytes) { // Write the header h then write the matrix content from buffer
            fprintf(stderr, "Failed writing matrix file %s\n", path);
            fclose(f);
            free(buffer);
            return 1;
        }
        fclose(f);
    }

    free(buffer);
    printf("Generated %d matrix files in %s\n", count, output_dir);
    return 0;
}
// .bin file looks like this in memory: [MatrixHeader][pixel bytes...]