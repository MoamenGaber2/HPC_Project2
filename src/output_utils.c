#include "output_utils.h"

#include <stdio.h>

#include "common.h"

int write_histogram_record(FILE *out, const char *file_name, uint64_t pixels, int bins, const uint64_t *hist, int hist_len) {
    if (!out || !file_name || !hist || hist_len <= 0) {
        return -1;
    }

    fprintf(out, "{\"file\":\"%s\",\"pixels\":%llu,\"bins_per_channel\":%d,\"histogram\":[",
            file_name, (unsigned long long)pixels, bins);
    for (int h = 0; h < hist_len; h++) {
        if (h) {
            fputc(',', out);
        }
        fprintf(out, "%llu", (unsigned long long)hist[h]);
    }
    fprintf(out, "]}\n");
    return 0;
}

int merge_rank_part_files(const char *parts_dir, const char *output_file, int size) {
    FILE *final = fopen(output_file, "w");
    if (!final) {
        return -1;
    }

    for (int r = 0; r < size; r++) {
        char pf[PATH_MAX];
        snprintf(pf, sizeof(pf), "%s/rank_%05d.jsonl", parts_dir, r);
        FILE *src = fopen(pf, "r");
        if (!src) {
            continue;
        }
        char line[8192];
        while (fgets(line, sizeof(line), src)) {
            fputs(line, final);
        }
        fclose(src);
    }
    fclose(final);
    return 0;
}

void print_summary_json(int total_files_found,
                        const int *processed_per_rank,
                        int size,
                        const char *output_file,
                        double elapsed_seconds) {
    fprintf(stdout, "{\"total_files_found\":%d,\"processed_per_rank\":[", total_files_found);
    for (int r = 0; r < size; r++) {
        if (r) {
            fputc(',', stdout);
        }
        fprintf(stdout, "%d", processed_per_rank[r]);
    }
    fprintf(stdout, "],\"output_file\":\"%s\",\"elapsed_seconds\":%.6f}\n",
            output_file, elapsed_seconds);
}
