# toysort

C++ 比较排序实验，包含多种手写算法和标准库基准。

## FastSort

`src/fastsort.cpp` 是完全手写的**单线程比较排序**，不调用标准库排序函数，
不使用线程池、并行执行策略、计数排序、桶排序或基数排序。
`FastSort::Run(const std::vector<int>&)` 返回升序副本，不修改输入；不保证稳定性。

- 分块快速排序：批量记录并交换错位元素；默认块大小为 32。
- 小区间插入排序：默认阈值为 32；三数取中，长度达到 128 时采用九点采样。
- 采样检测到重复主元时集中相等元素，跳过它们的递归。
- 已有序数据直接返回，逆序数据反转；无需交换的分区尝试有预算的插入排序。
- 深度预算耗尽后回退到手写堆排序，最坏时间复杂度为 `O(n log n)`。
  优先递归较小分区，除返回副本外额外空间为 `O(log n)`。

## 构建与测试

以下命令使用 CMake 3.20+ 和支持 C++14 的编译器；构建产物放在仓库外。

```sh
cmake -S . -B /tmp/toysort-release -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/toysort-release
ctest --test-dir /tmp/toysort-release --output-on-failure
/tmp/toysort-release/toysort
```

`toysort` 保留原来的算法对比和输出格式，FastSort 已加入快速算法组。
也可使用 `sh go.sh` 在仓库内构建并运行原有对比。

独立测试位于 `tests/`，无第三方测试依赖。普通路径和强制堆排序回退路径
各运行 39,907 个用例，使用 `std::sort` 校验完整结果、长度及输入不变性。
覆盖小数组穷举、阈值附近长度、固定种子随机数据、重复值、整数极值、
有序、逆序、近乎有序、周期、山峰形和稀疏异常值；失败返回非零状态。

```sh
cmake -S . -B /tmp/toysort-sanitize -DCMAKE_BUILD_TYPE=Debug -DFASTSORT_SANITIZERS=ON
cmake --build /tmp/toysort-sanitize --target fastsort_test fastsort_heap_test
ctest --test-dir /tmp/toysort-sanitize --output-on-failure
```

地址/未定义行为检查仅用于新增的 FastSort 测试和基准目标；不用于性能测量。
现有其他排序实现不在这次修复范围内。

## 可复现性能测试

```sh
/tmp/toysort-release/sort_benchmark > /tmp/toysort-benchmark.csv
/tmp/toysort-release/sort_benchmark --tune --seed 314159 > /tmp/toysort-confirmation.csv
# 更短的测试，只跑不超过十万个元素的规模
/tmp/toysort-release/sort_benchmark --max-size 100000
# 可选：Python 3，顺序测试全部九组参数
python3 benchmarks/tune.py --build-root /tmp/toysort-tuning
```

基准程序依次执行 FastSort、StdSort 和 StdStableSort，**排序和测速均无多线程**。
每组共享相同输入，预热一次、测量五次，轮换执行顺序，用 `steady_clock`
计时并输出中位数。计时包括各算法 `Run()` 内的输入复制；数据生成、
预期结果计算和输出校验不计时。结果错误立即失败，性能波动不作为测试失败条件。

默认种子为 `20261004`；规模为一千、一万、十万、一百万、一千万。
九种分布包括随机排列、全范围随机整数、升序、逆序、近乎有序（约 0.1% 次随机交换）、
16 种重复值、全相等、31 值周期及山峰形。`--tune` 仅测十万和一百万。
CSV 中 `ratio_to_std` 是该算法耗时除以 StdSort 耗时，小于 1 表示更快。

调优脚本比较插入阈值 16/24/32 和块大小 32/64/128，每组先运行正确性测试，
再按 18 个分布/规模组合的等权几何平均耗时选择参数。
构建可用 `-DFASTSORT_INSERTION_THRESHOLD=32 -DFASTSORT_BLOCK_SIZE=32` 覆盖默认值。
调优中的 `-j 4` 仅用于编译，算法仍为单线程。

## 本机实测

2026-10-04，macOS 26.6.2 / Apple ARM64，Apple Clang 21.0.0，libc++，
Release 参数 `-O3 -Wall -std=c++14`，未开启 sanitizer。
九组参数中 32/32 的几何平均耗时最低（1.128417 ms）；相近参数差距很小，
这个选择只代表本机这轮数据，不保证在其他机器上最佳。

一百万个元素、种子 `20261004` 的中位数如下：

| 数据分布 | FastSort (ms) | StdSort (ms) | 耗时比 |
| --- | ---: | ---: | ---: |
| 随机排列 | 19.377 | 16.132 | 1.201 |
| 全范围随机整数 | 19.164 | 16.112 | 1.189 |
| 升序 | 0.547 | 0.792 | 0.690 |
| 逆序 | 0.694 | 1.276 | 0.544 |
| 近乎有序 | 4.141 | 3.353 | 1.235 |
| 大量重复值 | 4.720 | 4.985 | 0.947 |
| 全相等 | 0.577 | 1.098 | 0.525 |
| 周期 | 4.424 | 3.998 | 1.107 |
| 山峰形 | 13.514 | 32.969 | 0.410 |

完整 45 组场景的等权几何平均耗时比为 **0.851**；独立种子 `314159`
复核十万/一百万规模的 18 组场景，耗时比为 **0.832**。
这不是总运行时间之比，也不表示每种输入都更快：随机和近乎有序数据仍落后于
本机 `std::sort`，重复值数据对种子敏感。不要将这个实现视为所有场景的最快算法。

原始结果：[完整测速](benchmarks/results/apple-arm64-release.csv)、
[独立种子复核](benchmarks/results/apple-arm64-confirmation.csv)、
[参数调优](benchmarks/results/apple-arm64-tuning.csv)。
