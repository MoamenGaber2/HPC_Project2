#ifndef OUTPUT_UTILS_H
#define OUTPUT_UTILS_H

#include <stdio.h>
#include <stdint.h>

int write_histogram_record(FILE *out, const char *file_name, uint64_t pixels, int bins, const uint64_t *hist, int hist_len);
int merge_rank_part_files(const char *parts_dir, const char *output_file, int size);
void print_summary_json(int total_files_found,
                        const int *processed_per_rank,
                        int size,
                        const char *output_file,
                        double elapsed_seconds);

#endif
