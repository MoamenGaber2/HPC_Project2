# Task 2 - MPI Histogram in C (Docker)

This project computes per-file color histograms using **C + MPI** on simulated binary matrix files.

The workflow is simplified:

- You only use CLI for **building and starting containers**
- All execution (running containers, terminals, etc.) is done via **Docker Desktop UI**

---

## What it does

- Scans a directory for `.bin` matrix files
- Distributes files across MPI processes
- Each process computes histograms independently
- Final results are merged into a single JSONL file

Designed to handle **very large datasets (~10GB)** safely using chunk-based processing.

---

## Binary Matrix Format (`.bin`)

Each file contains:

- Header:
  - `rows` (uint32)
  - `cols` (uint32)
  - `channels` (uint8) → `1` (grayscale) or `3` (RGB)

- Data:
  - Raw pixel values (`uint8`)
  - Size = `rows × cols × channels`

---

## Histogram Model

- `window_size` defines bin width over `[0..255]`
- `bins = ceil(256 / window_size)`
- Total bins per file = `bins³`

---

## Project Structure

```
src/
  mpi_histogram.c
  generate_matrices.c

tools/
  plot_histograms.py

Dockerfile
docker-compose.yml
Makefile
```

---

## Step 1 — Build Docker Image (CLI)

Run this once:

```bash
docker compose build
```

---

## Step 2 — Start the Cluster (CLI)

```bash
docker compose up -d
```

This creates:

- `master`
- `worker1`
- `worker2`

---

## Step 3 — Use Docker Desktop (No More CLI)

From here, **open Docker Desktop** and:

### 1. Open Containers

- Go to **Containers tab**
- You will see: `master`, `worker1`, `worker2`

---

### 2. Generate Input Data

- Click on **master container**
- Open **Terminal**

Run:

```bash
/app/generate_matrices /workspace/data 200 2048 2048 1 42
```

---

### 3. Run MPI Job

Still inside **master terminal**:

```bash
/app/run_cluster_job.sh /workspace/data 16 /workspace/out/histograms.jsonl 6
```

---

### 4. View Output

- Open the **Files tab** in Docker Desktop
- Navigate to:

```
/workspace/out/
```

You will find:

- `histograms.jsonl`

---

## Step 4 — Stop the Cluster (CLI)

```bash
docker compose down
```

---

## Output Format

Each line in `histograms.jsonl`:

```json
{
  "file": "matrix_0000000.bin",
  "pixels": 4194304,
  "bins_per_channel": 16,
  "histogram": [...]
}
```

---

## Generate Graphs (Optional - Local Python)

Install dependencies:

```bash
python -m pip install -r tools/requirements-plot.txt
```

Run:

```bash
python tools/plot_histograms.py \
  --input-jsonl out/histograms.jsonl \
  --output-dir out/graphs \
  --max-files 20 \
  --aggregate
```

---

## Memory Efficiency

- Files processed one-by-one
- Chunk-based reading (~1M pixels per chunk)
- Fixed histogram memory (`bins³`)
- Immediate disk writes per process

---

## Notes

- Docker Compose is used only to simulate a cluster locally
- For real clusters, replace Compose with actual machines + shared storage
- No need for long CLI commands — everything else is handled via Docker Desktop UI

---

## Summary (Very Short Workflow)

1. Build:

```bash
docker compose build
```

2. Start:

```bash
docker compose up -d
```

3. Use Docker Desktop:

- Open `master` container
- Run generator
- Run MPI job
- Check output

4. Stop:

```bash
docker compose down
```

---

This keeps the workflow **clean, visual, and much easier for demos or teammates**.
