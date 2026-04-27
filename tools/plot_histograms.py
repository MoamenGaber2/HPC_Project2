import argparse
import json
from pathlib import Path
from typing import Iterable, List, Tuple

import matplotlib.pyplot as plt


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert histogram JSONL output to histogram graph images."
    )
    parser.add_argument("--input-jsonl", required=True, help="Path to histograms.jsonl")
    parser.add_argument("--output-dir", required=True, help="Directory for generated PNG graphs")
    parser.add_argument(
        "--max-files",
        type=int,
        default=20,
        help="Maximum number of per-file graphs to generate (default: 20)",
    )
    parser.add_argument(
        "--aggregate",
        action="store_true",
        help="Also generate an aggregate histogram graph across all records",
    )
    return parser.parse_args()


def read_records(path: Path) -> Iterable[dict]:
    with path.open("r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            yield json.loads(line)


def sanitize_name(name: str) -> str:
    keep = []
    for ch in name:
        if ch.isalnum() or ch in ("-", "_", "."):
            keep.append(ch)
        else:
            keep.append("_")
    return "".join(keep)


def plot_single(hist: List[int], title: str, output_path: Path) -> None:
    x = list(range(len(hist)))
    plt.figure(figsize=(12, 4))
    plt.bar(x, hist, width=1.0)
    plt.title(title)
    plt.xlabel("Bin index")
    plt.ylabel("Count")
    plt.tight_layout()
    plt.savefig(output_path, dpi=120)
    plt.close()


def add_hist(a: List[int], b: List[int]) -> List[int]:
    if not a:
        return b[:]
    if len(a) != len(b):
        raise ValueError("Histogram lengths are inconsistent in input JSONL.")
    return [x + y for x, y in zip(a, b)]


def main() -> None:
    args = parse_args()
    input_path = Path(args.input_jsonl).resolve()
    output_dir = Path(args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    per_file_count = 0
    aggregate_hist: List[int] = []
    total_records = 0

    for rec in read_records(input_path):
        total_records += 1
        hist = rec.get("histogram")
        file_name = str(rec.get("file", f"record_{total_records}"))
        if not isinstance(hist, list):
            continue
        aggregate_hist = add_hist(aggregate_hist, hist)

        if per_file_count < args.max_files:
            graph_name = sanitize_name(file_name) + ".png"
            title = f"Histogram: {file_name}"
            plot_single(hist, title, output_dir / graph_name)
            per_file_count += 1

    if args.aggregate and aggregate_hist:
        plot_single(aggregate_hist, "Aggregate Histogram", output_dir / "aggregate_histogram.png")

    print(
        json.dumps(
            {
                "input_jsonl": str(input_path),
                "output_dir": str(output_dir),
                "records_found": total_records,
                "per_file_graphs_written": per_file_count,
                "aggregate_written": bool(args.aggregate and aggregate_hist),
            }
        )
    )


if __name__ == "__main__":
    main()
