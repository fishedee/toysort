# Repository Guidelines

## Project Structure & Module Organization

This repository compares C++ sorting implementations in one `toysort` executable.
- `src/`: algorithm implementations, usually paired lowercase `.h` and `.cpp` files (for example, `quicksort.h` and `quicksort.cpp`).
- `src/sort.h`: shared `Sort` interface with `GetName()` and `Run()`.
- `src/main.cpp`: algorithm registration, correctness checks, and timing benchmarks.
- `src/util/stack.h`: shared stack utility.
- `CMakeLists.txt`: builds the executable from sources directly under `src/`.

There are no separate test or asset directories. Register new algorithms in the appropriate speed group in `main.cpp`; rerun CMake after adding source files.

## Build, Test, and Development Commands

Use CMake and a C++ compiler; the convenience script also requires Make.

```sh
sh go.sh                              # Configure Release, build, and run benchmarks
cmake -DCMAKE_BUILD_TYPE=Release .     # Configure the in-place build
cmake --build .                       # Build toysort
./toysort                             # Run correctness checks and timings
cmake -DCMAKE_BUILD_TYPE=Debug .       # Enable debug symbols and disable optimization
```

Rebuild after switching configurations. Release uses `-O3 -Wall`; Debug uses `-O0 -Wall -g -ggdb`. The benchmark reaches ten million elements, so allow time and memory for a full run.

## Coding Style & Naming Conventions

Follow neighboring code: implementation bodies predominantly use tabs, while some headers use spaces. Avoid unrelated reformatting. Use PascalCase class names such as `QuickSort2`, lowercase algorithm filenames, and the existing `GetName()`/`Run()` interface. Headers use `#pragma once`; file-local algorithm helpers commonly use `static`. Preserve the input vector and return the sorted result. No formatter or linter is configured.

## Testing Guidelines

The executable is the existing test harness; no testing framework or coverage threshold is configured. Check that every reported `isCorrect` value is `1`; process exit status alone does not indicate correctness. Current checks expect shuffled integers from zero through size minus one. For algorithm changes, additionally validate empty, singleton, duplicate, sorted, reverse-sorted, and negative-value inputs against `std::sort`. Adapt the expected-result check for those cases.

## Commit & Pull Request Guidelines

History uses short Chinese action summaries naming the affected algorithm, such as `更新quicksort` and `加入三路快排`. Keep commits focused and similarly descriptive. In pull requests, explain the algorithm change, commands run, correctness results, and relevant timing comparisons. Link related issues when applicable. Keep generated build files and binaries out of commits.
