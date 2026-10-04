#!/usr/bin/env python3
"""Sequential Metal tuning, correctness checks and repeatable end-to-end validation."""
import argparse
import csv
import json
import math
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def run(args, log, env=None):
    with log.open('w') as output:
        subprocess.run([str(x) for x in args], cwd=ROOT, stdout=output,
                       stderr=subprocess.STDOUT, check=True, env=env)


def rows(path):
    return list(csv.DictReader(line for line in path.read_text().splitlines() if not line.startswith('#')))


def configure(build, block, items, threshold, logs):
    run(['cmake', '-S', ROOT, '-B', build, '-DCMAKE_BUILD_TYPE=Release',
         '-DBUILD_TESTING=ON', '-DFASTSORT_SANITIZERS=OFF',
         '-DFASTSORT4_ENABLE_METAL=ON', f'-DFASTSORT4_BLOCK_SIZE={block}',
         f'-DFASTSORT4_MERGE_ITEMS={items}', f'-DFASTSORT4_GPU_THRESHOLD={threshold}'], logs / 'configure.log')
    run(['cmake', '--build', build, '-j', '4'], logs / 'build.log')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=Path, default=Path('/tmp/toysort-fast4-tuning'))
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.build_root = args.build_root.resolve()
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    candidates = []
    for block in (128, 256, 512):
        for items in (4, 8, 16):
            label = f'{block}-{items}'
            build = args.build_root / label
            logs = args.output / label
            logs.mkdir(parents=True, exist_ok=True)
            print(f'Tuning {label}: build, GPU correctness, benchmark', flush=True)
            configure(build, block, items, 100000, logs)
            # A missing GPU returns 77, which is a failure here, not a pass.
            run([build / 'fastsort4_gpu_test'], logs / 'gpu-test.log')
            run([build / 'sort_benchmark', '--tune', '--fast4-mode', 'metal'], logs / 'benchmark.csv')
            data = rows(logs / 'benchmark.csv')
            focus = [r for r in data if r['algorithm'] == 'FastSort4'
                     and r['distribution'] in ('random', 'permutation')]
            assert len(focus) == 4
            score = math.exp(sum(math.log(float(r['median_ms'])) for r in focus) / 4)
            candidates.append((score, block, items, data))
            print(f'{label}: geometric mean {score:.6f} ms', flush=True)
    with (args.output / 'tuning.csv').open('w') as output:
        writer = csv.writer(output)
        writer.writerow(['block_size', 'merge_items', 'geomean_ms'])
        for score, block, items, _ in candidates:
            writer.writerow([block, items, f'{score:.6f}'])
    score, block, items, data = min(candidates, key=lambda item: item[0])
    small = [r for r in data if r['algorithm'] == 'FastSort4' and r['size'] == '100000'
             and r['distribution'] in ('random', 'permutation')]
    threshold = 100000 if all(float(r['median_ms']) < 0.95 * float(next(x['median_ms'] for x in data
                            if x['algorithm'] == 'FastSort3' and x['size'] == r['size']
                            and x['distribution'] == r['distribution'])) for r in small) else 1000000
    selected = dict(block_size=block, merge_items=items, threshold=threshold)
    (args.output / 'selected.json').write_text(json.dumps(selected, indent=2) + '\n')
    print(f'Selected: {selected}', flush=True)
    build = args.build_root / 'selected'
    configure(build, block, items, threshold, args.output)
    run(['ctest', '--test-dir', build, '--output-on-failure'], args.output / 'ctest.log')
    env = os.environ.copy()
    env.update(MTL_DEBUG_LAYER='1', MTL_SHADER_VALIDATION='1',
               MTL_SHADER_VALIDATION_REPORT_TO_STDERR='1', MTL_SHADER_VALIDATION_ABORT_ON_FAULT='1')
    run([build / 'fastsort4_gpu_test'], args.output / 'metal-validation.log', env)
    run([build / 'toysort'], args.output / 'toysort.log')
    correctness = [line for line in (args.output / 'toysort.log').read_text().splitlines() if 'isCorrect:' in line]
    assert correctness and all(line.endswith('isCorrect: 1') for line in correctness)
    comparisons = []
    for seed in (20261004, 314159):
        for repeat in (1, 2):
            print(f'Full benchmark seed={seed}, repeat={repeat}', flush=True)
            path = args.output / f'benchmark-{seed}-{repeat}.csv'
            run([build / 'sort_benchmark', '--seed', seed], path)
            if any(line.split('fallback=', 1)[1].strip() for line in path.read_text().splitlines()
                   if line.startswith('# FastSort4') and 'fallback=' in line):
                raise RuntimeError(f'unexpected GPU fallback in {path}')
            data = rows(path)
            for row in data:
                if row['algorithm'] != 'FastSort4' or row['distribution'] not in ('random', 'permutation'):
                    continue
                ref = next(r for r in data if r['algorithm'] == 'FastSort3'
                           and r['distribution'] == row['distribution'] and r['size'] == row['size'])
                ratio = float(row['median_ms']) / float(ref['median_ms'])
                comparisons.append([seed, repeat, row['distribution'], row['size'], row['median_ms'], ref['median_ms'], ratio])
    with (args.output / 'comparison.csv').open('w') as output:
        writer = csv.writer(output)
        writer.writerow(['seed', 'repeat', 'distribution', 'size', 'fast4_ms', 'fast3_ms', 'ratio_to_fast3'])
        writer.writerows(comparisons)
    failures = [r for r in comparisons if int(r[3]) >= 1000000 and r[-1] >= 1]
    print(f'Large random acceptance: {"FAIL" if failures else "PASS"}; results: {args.output}', flush=True)
    if failures:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
