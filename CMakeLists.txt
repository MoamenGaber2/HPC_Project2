cmake_minimum_required(VERSION 3.16)
project(hpc_histogram C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(MPI REQUIRED)

add_executable(mpi_histogram
    src/mpi_histogram.c
    src/string_list.c
    src/fs_utils.c
    src/matrix_hist.c
    src/output_utils.c
)
target_include_directories(mpi_histogram PRIVATE src)
target_link_libraries(mpi_histogram PRIVATE MPI::MPI_C)

add_executable(generate_matrices
    src/generate_matrices.c
)
target_include_directories(generate_matrices PRIVATE src)
