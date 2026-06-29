#!/usr/bin/env python3

import argparse
import csv
import statistics
from pathlib import Path


def read_times(path: Path):
    times = []

    with path.open("r") as f:
        for line_no, line in enumerate(f, start=1):
            line = line.strip()

            if not line:
                continue

            # Allows either plain lines like:
            #   0.12345
            # or CSV-ish lines where the first field is the time:
            #   0.12345,...
            first_field = line.split(",")[0].strip()

            try:
                times.append(float(first_field))
            except ValueError:
                raise ValueError(f"Could not parse time in {path}:{line_no}: {line!r}")

    return times


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("fast_file", help="File with fast-version times, one per line")
    parser.add_argument("slow_file", help="File with slow-version times, one per line")
    parser.add_argument(
        "-o", "--output",
        default="speedups.csv",
        help="Output CSV file"
    )

    args = parser.parse_args()

    fast_times = read_times(Path(args.fast_file))
    slow_times = read_times(Path(args.slow_file))

    if len(fast_times) != len(slow_times):
        raise RuntimeError(
            f"Different number of times: "
            f"{args.fast_file} has {len(fast_times)}, "
            f"{args.slow_file} has {len(slow_times)}"
        )

    rows = []
    speedups = []

    for i, (fast, slow) in enumerate(zip(fast_times, slow_times), start=1):
        if fast == 0:
            speedup = float("inf")
        else:
            speedup = slow / fast

        rows.append({
            "dataset": i,
            "fast_time": fast,
            "slow_time": slow,
            "speedup": speedup,
        })

        if speedup != float("inf"):
            speedups.append(speedup)

    output_path = Path(args.output)

    with output_path.open("w", newline="") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=["dataset", "fast_time", "slow_time", "speedup"]
        )
        writer.writeheader()
        writer.writerows(rows)

    print(f"Wrote {output_path}")

    if speedups:
        print(f"Average speedup: {statistics.mean(speedups):.6f}x")
        print(f"Median speedup:  {statistics.median(speedups):.6f}x")

        # Geometric mean is usually better for speedup ratios.
        geom = statistics.geometric_mean(speedups)
        print(f"Geomean speedup: {geom:.6f}x")


if __name__ == "__main__":
    main()