#include <errno.h>
#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "fs_utils.h"
#include "matrix_hist.h"
#include "output_utils.h"
#include "string_list.h"

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv); // This starts the MPI environment
    int rank, size;
    double start_time = 0.0;
    double end_time = 0.0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the process rank (rank = id)
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get the total number of processes (size = total number of processes)

    if (argc < 4) {
        if (rank == 0) {
            // ./mpi_histogram input_data 16 results/output.jsonl
            // look in input_data, histogram bin width = 16, save results to results/output.jsonl
            fprintf(stderr, "Usage: %s <input_dir> <window_size> <output_file>\n", argv[0]);
            fprintf(stderr, "Input files are .bin matrices with header {u32 rows, u32 cols, u8 channels} then uint8 payload.\n");
        }
        MPI_Finalize();
        return 1;
    }

    const char *input_dir = argv[1];
    int window_size = atoi(argv[2]);
    const char *output_file = argv[3];

    if (window_size <= 0 || window_size > 256) {
        if (rank == 0) fprintf(stderr, "window_size must be in [1..256]\n");
        MPI_Finalize();
        return 1;
    }

    int bins = (256 + window_size - 1) / window_size;
    int hist_len = bins * bins * bins; // Total number of pins

    StringList files;
    list_init(&files);
    scan_dir_recursive(input_dir, &files);
    qsort(files.items, (size_t)files.count, sizeof(char *), cmp_str_ptr);

    char parts_dir[PATH_MAX];
    snprintf(parts_dir, sizeof(parts_dir), "%s.parts", output_file);
    if (rank == 0) {
        if (make_parent_dirs(output_file) != 0) {
            fprintf(stderr, "Failed to create parent directories for output file.\n");
            list_free(&files);
            MPI_Finalize();
            return 1;
        }
        if (ensure_dir(parts_dir) != 0 && errno != EEXIST) {
            fprintf(stderr, "Failed to create parts directory: %s\n", parts_dir);
            list_free(&files);
            MPI_Finalize();
            return 1;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD); // Makes all ranks wait until setup is done
    start_time = MPI_Wtime();

    char part_file[PATH_MAX];
    snprintf(part_file, sizeof(part_file), "%s/rank_%05d.jsonl", parts_dir, rank);
    FILE *out = fopen(part_file, "w");
    if (!out) {
        fprintf(stderr, "Rank %d failed to open part file %s\n", rank, part_file);
        list_free(&files);
        MPI_Finalize();
        return 1;
    }

    int local_count = 0;
    for (int i = rank; i < files.count; i += size) {
        uint64_t *hist = (uint64_t *)calloc((size_t)hist_len, sizeof(uint64_t)); // Allocate histogram memory and initialize it to zero
        if (!hist) {
            fprintf(stderr, "Rank %d out of memory for histogram.\n", rank);
            fclose(out);
            list_free(&files);
            MPI_Finalize();
            return 1;
        }
        uint64_t pixels = 0;
        int rc = process_file_hist(files.items[i], window_size, hist, bins, &pixels); // Reads the file and fills the histogram
        if (rc != 0) {
            fprintf(stderr, "Rank %d failed to process file: %s\n", rank, files.items[i]);
            free(hist);
            continue;
        }

        write_histogram_record(out, base_name(files.items[i]), pixels, bins, hist, hist_len);
        free(hist);
        local_count++;
    }
    fclose(out);

    int *all_counts = NULL;
    if (rank == 0) {
        all_counts = (int *)malloc((size_t)size * sizeof(int));
    }
    MPI_Gather(&local_count, 1, MPI_INT, all_counts, 1, MPI_INT, 0, MPI_COMM_WORLD); // Sends each output to rank0 to collect
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        if (merge_rank_part_files(parts_dir, output_file, size) != 0) {
            fprintf(stderr, "Failed to open final output file: %s\n", output_file);
            free(all_counts);
            list_free(&files);
            MPI_Finalize();
            return 1;
        }
        end_time = MPI_Wtime();
        print_summary_json(files.count, all_counts, size, output_file, end_time - start_time);
        free(all_counts);
    }

    list_free(&files);
    MPI_Finalize();
    return 0;
}
