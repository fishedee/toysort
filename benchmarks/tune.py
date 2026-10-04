#!/usr/bin/env python3
"""Sequential Release builds and a reproducible FastSort parameter sweep."""
import argparse
import csv
import math
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-root", type=Path,
                        default=Path(tempfile.gettempdir()) / "toysort-tuning")
    parser.add_argument("--seed", type=int, default=20261004)
    args = parser.parse_args()
    if not 0 <= args.seed <= 2**32 - 1:
        parser.error("seed must fit in an unsigned 32-bit integer")
    source = Path(__file__).resolve().parents[1]
    args.build_root.mkdir(parents=True, exist_ok=True)
    scores = []
    for insertion in (16, 24, 32):
        for block in (32, 64, 128):
            name = f"insertion-{insertion}-block-{block}"
            build = args.build_root / name
            with (args.build_root / f"{name}.log").open("w") as log:
                commands = [
                    ["cmake", "-S", str(source), "-B", str(build),
                     "-DCMAKE_BUILD_TYPE=Release", "-DBUILD_TESTING=ON",
                     "-DFASTSORT_SANITIZERS=OFF",
                     f"-DFASTSORT_INSERTION_THRESHOLD={insertion}",
                     f"-DFASTSORT_BLOCK_SIZE={block}"],
                    ["cmake", "--build", str(build), "--target",
                     "sort_benchmark", "fastsort_test", "-j", "4"],
                    ["ctest", "--test-dir", str(build), "--output-on-failure",
                     "-R", "^fastsort_correctness$"],
                ]
                for command in commands:
                    subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
            output = args.build_root / f"{name}.csv"
            with output.open("w") as destination:
                subprocess.run([str(build / "sort_benchmark"), "--tune", "--seed", str(args.seed)],
                               stdout=destination, check=True)
            with output.open() as data:
                rows = csv.DictReader(line for line in data if not line.startswith("#"))
                times = [float(row["median_ms"]) for row in rows if row["algorithm"] == "FastSort"]
            if len(times) != 18 or any(time <= 0 for time in times):
                raise RuntimeError(f"incomplete or invalid measurements in {output}")
            score = math.exp(sum(math.log(time) for time in times) / len(times))
            scores.append((score, insertion, block))
            print(f"{name}: geometric mean {score:.6f} ms (18 equally weighted cases)", flush=True)
    with (args.build_root / "summary.csv").open("w") as destination:
        writer = csv.writer(destination)
        writer.writerow(("geomean_ms", "insertion_threshold", "block_size"))
        writer.writerows(sorted(scores))
    score, insertion, block = min(scores)
    print(f"Selected: insertion={insertion}, block={block}, geomean={score:.6f} ms")


if __name__ == "__main__":
    main()
