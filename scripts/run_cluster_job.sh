#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 3 ]; then
    echo "Usage: \$0 <input_dir> <window_size> <output_file> [np] [slots_per_node] [use_master:yes|no]"
    exit 1
fi

INPUT_DIR=$1
WINDOW_SIZE=$2
OUTPUT_FILE=$3

# Optional arguments
NP=${4:-6}
SLOTS_PER_NODE=${5:-2}
USE_MASTER=${6:-yes}

HOSTFILE="/tmp/mpi-hosts.txt"

# Validate numeric inputs
case "$NP" in
    ''|*[!0-9]*)
        echo "Error: np must be a positive integer."
        exit 1
        ;;
esac

case "$SLOTS_PER_NODE" in
    ''|*[!0-9]*)
        echo "Error: slots_per_node must be a positive integer."
        exit 1
        ;;
esac

if [ "$NP" -le 0 ]; then
    echo "Error: np must be >= 1."
    exit 1
fi

if [ "$SLOTS_PER_NODE" -le 0 ]; then
    echo "Error: slots_per_node must be >= 1."
    exit 1
fi

if [ "$USE_MASTER" != "yes" ] && [ "$USE_MASTER" != "no" ]; then
    echo "Error: use_master must be 'yes' or 'no'."
    exit 1
fi

# Build hostfile dynamically
if [ "$USE_MASTER" = "yes" ]; then
    cat > "${HOSTFILE}" <<EOF
master slots=${SLOTS_PER_NODE}
worker1 slots=${SLOTS_PER_NODE}
worker2 slots=${SLOTS_PER_NODE}
EOF
    TOTAL_SLOTS=$((3 * SLOTS_PER_NODE))
else
    cat > "${HOSTFILE}" <<EOF
worker1 slots=${SLOTS_PER_NODE}
worker2 slots=${SLOTS_PER_NODE}
EOF
    TOTAL_SLOTS=$((2 * SLOTS_PER_NODE))
fi

# Safety check: do not allow oversubscription
if [ "$NP" -gt "$TOTAL_SLOTS" ]; then
    echo "Error: requested np=${NP}, but only ${TOTAL_SLOTS} total slots are available."
    echo "       Reduce np or increase slots_per_node."
    exit 1
fi

echo "Running MPI job on hosts:"
cat "${HOSTFILE}"
echo "Total available slots: ${TOTAL_SLOTS}"
echo "Requested MPI processes: ${NP}"
echo "Use master for compute: ${USE_MASTER}"

mpirun \
    --allow-run-as-root \
    --hostfile "${HOSTFILE}" \
    --nooversubscribe \
    -np "${NP}" \
    --mca plm_rsh_agent ssh \
    /app/mpi_histogram "${INPUT_DIR}" "${WINDOW_SIZE}" "${OUTPUT_FILE}"