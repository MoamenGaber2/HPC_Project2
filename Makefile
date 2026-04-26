CC=mpicc
CFLAGS=-O3 -std=c11 -Wall -Wextra -I include

all: mpi_histogram generate_matrices

MPI_HIST_SRCS=src/mpi_histogram.c src/string_list.c src/fs_utils.c src/matrix_hist.c src/output_utils.c

mpi_histogram: $(MPI_HIST_SRCS)
	$(CC) $(CFLAGS) -o mpi_histogram $(MPI_HIST_SRCS)

generate_matrices: src/generate_matrices.c
	$(CC) $(CFLAGS) -o generate_matrices src/generate_matrices.c

clean:
	rm -f mpi_histogram generate_matrices