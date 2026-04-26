#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 3 ]; then
  echo "Usage: $0 <input_dir> <window_size> <output_file> [np]"
  exit 1
fi

INPUT_DIR="$1"
WINDOW_SIZE="$2"
OUTPUT_FILE="$3"
NP="${4:-6}"

HOSTFILE="/tmp/mpi-hosts.txt"
cat > "${HOSTFILE}" <<EOF
master slots=2
worker1 slots=2
worker2 slots=2
EOF

echo "Running MPI job on hosts:"
cat "${HOSTFILE}"

mpirun \
  --allow-run-as-root \
  --hostfile "${HOSTFILE}" \
  -np "${NP}" \
  --mca plm_rsh_agent ssh \
  /app/mpi_histogram "${INPUT_DIR}" "${WINDOW_SIZE}" "${OUTPUT_FILE}"