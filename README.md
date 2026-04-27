# Task 2 - MPI Histogram in C (Docker)

This project computes per-file color histograms with **C + MPI**.  
Input files are simulated binary matrices (`.bin`), which satisfies the "2D matrix is fine" requirement and works well for very large datasets.

## What it does

- Recursively scans an input directory for `.bin` matrix files.
- Distributes files across MPI ranks using round-robin assignment.
- Each rank computes histogram for assigned files and writes a rank-local part file.
- Rank 0 merges all parts into one final JSONL output.

This design is memory-safe for large input (around 10 GB) because files are streamed in chunks instead of globally loaded.

## Binary matrix format (`.bin`)

Each file contains:

1. Header struct:
   - `uint32_t rows`
   - `uint32_t cols`
   - `uint8_t channels` (`1` for 2D grayscale, `3` for RGB-like)
2. Payload:
   - Raw `uint8_t` matrix data in row-major order
   - Size = `rows * cols * channels`

If `channels=1`, a pixel value is reused for R/G/B when histogramming.

## Histogram model

- `window_size` is bin width over `[0..255]`.
- `bins = ceil(256 / window_size)`
- Total histogram bins per file = `bins^3`.

## Files

- `src/mpi_histogram.c`: MPI histogram processor.
- `src/generate_matrices.c`: generator for simulated binary matrices.
- `Makefile`: builds both executables with `mpicc`.
- `CMakeLists.txt`: CMake build for MPI binaries.
- `Dockerfile`: C/MPI build and runtime image.
- `tools/plot_histograms.py`: Python graph generation from JSONL histogram output.

## Build

```bash
docker build -t mpi-hist-c .
```

Build using CMake (without Docker):

```bash
cmake -S . -B build
cmake --build build
```

Or with Compose (for multi-node simulation):

```bash
docker compose build
```

## Generate simulated matrices (optional)

This command generates 2D matrices (`channels=1`):

```bash
docker run --rm -v "${PWD}:/appdata" --entrypoint /app/generate_matrices mpi-hist-c \
  /appdata/data 200 2048 2048 1 42
```

Arguments are:
`<output_dir> <count> <rows> <cols> <channels(1|3)> <seed>`

## Run histogram with MPI inside Docker

```bash
docker run --rm -v "${PWD}:/appdata" mpi-hist-c \
  /appdata/data 16 /appdata/out/histograms.jsonl
```

To explicitly choose rank count:

```bash
docker run --rm -v "${PWD}:/appdata" --entrypoint mpirun mpi-hist-c \
  -np 4 /app/mpi_histogram /appdata/data 16 /appdata/out/histograms.jsonl
```

## Multi-node simulation with Docker Compose

This simulates a cluster of 3 nodes:

- `master`
- `worker1`
- `worker2`

All nodes run the same image and share your project folder at `/workspace`.

### 1) Start the simulated cluster

```bash
docker compose up -d
```

### 2) Generate input matrices from the master node

```bash
docker compose exec master /app/generate_matrices /workspace/data 200 2048 2048 1 42
```

### 3) Run MPI job across nodes

```bash
docker compose exec master /app/run_cluster_job.sh /workspace/data 16 /workspace/out/histograms.jsonl 6
```

Arguments are:

- `input_dir`
- `window_size`
- `output_file`
- optional `np` (default 6)

### 4) Stop the cluster

```bash
docker compose down
```

## Output format

`histograms.jsonl` contains one JSON object per matrix file:

```json
{"file":"matrix_0000000.bin","pixels":4194304,"bins_per_channel":16,"histogram":[...]}
```

## Convert output to histogram graphs (Python)

Install plotting dependency:

```bash
python -m pip install -r tools/requirements-plot.txt
```

Generate PNG graphs from output JSONL:

```bash
python tools/plot_histograms.py \
  --input-jsonl out/histograms.jsonl \
  --output-dir out/graphs \
  --max-files 20 \
  --aggregate
```

This writes:

- up to `max-files` per-file graphs
- optional `aggregate_histogram.png` when `--aggregate` is used

## Memory strategy for very large input

- Files are processed one by one per rank.
- File payload is streamed in chunks (`~1M pixels/chunk`), not fully loaded.
- Histogram memory is fixed per file (`bins^3` counters).
- Each rank writes incremental disk output immediately.

## Multi-node note

Compose mode is for local simulation. For real multi-machine runs, keep the same MPI program and replace Compose with actual hosts and shared storage.
