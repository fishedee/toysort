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

`toysort` 保留原来的算法对比和输出格式，FastSort、FastSort2 和 FastSort3 已加入快速算法组。
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

基准程序对比 FastSort、FastSort2、FastSort3、StdSort 和 StdStableSort，**排序和测速均无多线程**。
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

## FastSort 历史实测

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

## FastSort2（FAST2）

`FastSort2` 是独立的手写单线程比较排序，保留原来的 FastSort 实现。
接口仍为 `Run(const std::vector<int>&)`：返回升序副本，不修改输入，不保证稳定性。
不调用标准库排序，不使用基数、桶、计数排序或多线程。

- 分块比较生成位掩码，按置位位置循环搬移错位元素；短分区使用条件赋值。
- 三数取中 / 九点采样选择主元，小分区使用插入排序；采样发现重复主元时集中相等元素。
- 升序、全相等和逆序输入有快捷路径；未发生交换的分区尝试有预算的插入排序。
- 连续不平衡分区打散采样位置，深度预算耗尽后回退手写堆排序。
- 最坏时间为 `O(n log n)`，除结果副本外使用固定分区缓冲和 `O(log n)` 栈空间。

FastSort2 复用正确性用例，并增加分区边界、交替极值、周期 127、相邻逆序对和
旋转升序排列。普通路径和强制堆排序路径各检查 39,967 个用例。

```sh
cmake -S . -B /tmp/toysort-fast2-release -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/toysort-fast2-release -j 4
ctest --test-dir /tmp/toysort-fast2-release --output-on-failure
/tmp/toysort-fast2-release/sort_benchmark > /tmp/fast2-release.csv
/tmp/toysort-fast2-release/sort_benchmark --seed 314159 > /tmp/fast2-confirmation.csv

cmake -S . -B /tmp/toysort-fast2-sanitize -DCMAKE_BUILD_TYPE=Debug -DFASTSORT_SANITIZERS=ON
cmake --build /tmp/toysort-fast2-sanitize --target fastsort2_test fastsort2_heap_test
ctest --test-dir /tmp/toysort-fast2-sanitize -R '^fastsort2_' --output-on-failure

python3 benchmarks/tune.py --algorithm FastSort2 --build-root /tmp/toysort-fast2-tuning
```

调优仅使用种子 `20261004`，比较插入阈值 16/24/32 和分块大小 32/64/128，
按十万、百万规模随机排列和全范围随机整数四项耗时的几何平均选择参数。
`--algorithm FastSort` 保留原来的九分布调优口径；两种算法使用独立的 CMake 参数。
最终验收还覆盖千万规模，并使用独立种子 `314159` 复核，不以综合平均代替随机场景逐项领先。

### FAST2 本机验收结果

2026-10-04，macOS 26.6.2 / Apple ARM64，Apple Clang 21.0.0 / libc++，
Release `-O3 -Wall -std=c++14`，未开启 sanitizer。默认插入阈值为 **24**，
分块大小为 **64**；可分别通过 `FASTSORT2_INSERTION_THRESHOLD` 和
`FASTSORT2_BLOCK_SIZE` 覆盖，后者支持 32、64、128。
九组参数中 24/64 的四项几何平均耗时为 4.971306 ms，在本轮调优中最低。

以下中位耗时包含输入复制成本；耗时比为 FAST2 / StdSort，小于 1 表示更快：

| 分布 | 元素数 | FAST2 (ms)，种子 20261004 | StdSort (ms)，种子 20261004 | 耗时比 | 独立种子 314159 耗时比 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 随机排列 | 100,000 | 1.546 | 1.582 | 0.977 | 0.943 |
| 全范围随机整数 | 100,000 | 1.461 | 1.517 | 0.963 | 0.953 |
| 随机排列 | 1,000,000 | 15.775 | 16.654 | 0.947 | 0.957 |
| 全范围随机整数 | 1,000,000 | 15.287 | 16.245 | 0.941 | 0.948 |
| 随机排列 | 10,000,000 | 168.062 | 177.587 | 0.946 | 0.960 |
| 全范围随机整数 | 10,000,000 | 172.787 | 178.244 | 0.969 | 0.929 |

两个种子的六个重点场景逐项通过，耗时降低约 **2.3%–7.1%**。
完整 45 场景的等权几何平均耗时比分别为 **0.867** 和 **0.860**。
这只代表本机测量，不保证所有机器或输入都优于标准库。

存在明确退化：种子 `20261004` 下，一千 / 一万规模的随机排列和随机整数
耗时比为 1.029–1.346；近乎有序分布五个规模均落后，耗时比为 1.229–4.707，
其中百万规模为 2.172。部分小规模重复值、周期及山峰形输入也慢于标准库。
FAST2 的验收目标是大规模随机数据，不是所有场景逐项更快。

验证：Release 的四项 CTest 全部通过；FAST2 普通 / 强制堆排序各 39,967 用例
均通过 ASan 和 UBSan。原有 `toysort` 完整运行至千万规模，逐项检查 `isCorrect=1`。

原始数据：[完整测速](benchmarks/results/fast2-apple-arm64-release.csv)、
[独立种子完整复核](benchmarks/results/fast2-apple-arm64-confirmation.csv)、
[参数调优](benchmarks/results/fast2-apple-arm64-tuning.csv)。

## FastSort3：SIMD 分区

`FastSort3` 在 FastSort2 的基础上，使用 SIMD 批量比较整数并生成分区位掩码；
保留 FastSort2 作为独立对照。接口仍是 `Run(const std::vector<int>&)`，返回升序副本，
不修改输入，不保证稳定性，保持单线程比较排序及最坏 `O(n log n)` 时间复杂度。
主元采样、重复值处理、小区间插入排序、已有序快捷路径和堆排序保护沿用 FastSort2。

- ARM64 NEON：每次向量比较 4 个有符号整数，合并 16 个比较结果后压缩为位掩码。
- x86 GCC/Clang：用独立 `target("avx2")` 函数每次比较 8 个整数，并在排序入口检测
  CPU 支持情况；无 AVX2 时自动使用标量路径。不需要全局开启 `-mavx2` 或 `-march=native`。
- 其他架构/编译器使用标量路径。可通过 `-DFASTSORT3_ENABLE_SIMD=OFF` 强制禁用显式 SIMD 后端。
- 只读取完整有效向量，短分区保持标量处理；左右掩码保留原来的比较语义和反向位序。
  `FastSort3::GetBackendName()` 返回 `NEON`、`AVX2` 或 `scalar`，基准 CSV 注释也记录后端。

```sh
cmake -S . -B /tmp/toysort-fast3-release -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/toysort-fast3-release -j 4
ctest --test-dir /tmp/toysort-fast3-release --output-on-failure
/tmp/toysort-fast3-release/sort_benchmark > /tmp/fast3-release.csv
/tmp/toysort-fast3-release/sort_benchmark --seed 314159 > /tmp/fast3-confirmation.csv

cmake -S . -B /tmp/toysort-fast3-sanitize -DCMAKE_BUILD_TYPE=Debug -DFASTSORT_SANITIZERS=ON
cmake --build /tmp/toysort-fast3-sanitize --target fastsort3_test fastsort3_scalar_test fastsort3_heap_test fastsort3_masks_test -j 4
ctest --test-dir /tmp/toysort-fast3-sanitize -R '^fastsort3_' --output-on-failure

python3 benchmarks/tune.py --algorithm FastSort3 --build-root /tmp/toysort-fast3-tuning
```

FastSort3 的普通、强制标量和强制堆排序测试各检查 41,025 个输入，包括 SIMD lane、
分区块及尾部边界。独立掩码测试检查 326,432 个场景，覆盖所有 16 位模式、
单个置位/清零、全空/全满、有符号极值、反向位序和不同地址对齐。

调优使用独立的 `FASTSORT3_INSERTION_THRESHOLD`、`FASTSORT3_BLOCK_SIZE` 参数，
候选分别为 16/24/32 和 32/64/128，按十万/百万随机排列、全范围随机整数四项
耗时的等权几何平均选择参数；最终另外验收千万规模和独立种子。

### FAST3 本机验收结果

2026-10-04，Apple ARM64 / Apple Clang 21.0.0 / libc++，Release `-O3 -Wall -std=c++14`，
NEON 后端，未开启 sanitizer。九组参数中 **插入阈值 24、分块大小 64** 的四项
几何平均耗时最低，为 **4.549758 ms**，因此采用此默认值。

两个种子各运行两轮完整基准，每轮每场景预热一次、测量五次、报告中位数；
每轮均轮换算法顺序，计时包含输入复制。以下为 FastSort3 / FastSort2 的耗时比，
小于 1 表示 FastSort3 更快：

| 分布 | 元素数 | 种子 20261004 | 同种子复测 | 种子 314159 | 同种子复测 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 随机排列 | 100,000 | 0.958 | 0.944 | 0.977 | 0.920 |
| 全范围随机整数 | 100,000 | 0.940 | 0.916 | 0.940 | 0.959 |
| 随机排列 | 1,000,000 | 0.933 | 0.932 | 0.935 | 0.945 |
| 全范围随机整数 | 1,000,000 | 0.922 | 0.932 | 0.932 | 0.931 |
| 随机排列 | 10,000,000 | 0.949 | 0.930 | 0.931 | 0.933 |
| 全范围随机整数 | 10,000,000 | 0.923 | 0.929 | 0.929 | 0.938 |

12 个重点场景在两轮测量中均优于 FastSort2，耗时降低 **2.3%–8.4%**。
四轮完整 45 场景的等权几何平均耗时比分别为 0.958、0.955、0.952、0.947（首次两个种子、复测两个种子）。
这些结果仅代表本机，不能推断其他 CPU 上的收益，也不表示每种输入都更快。
例如首次测量中，一千元素升序输入耗时比为 1.059；短耗时场景受计时与执行顺序影响，
请结合原始 CSV 比较，而不要将小差异视为稳定提升。

验证结果：Release 全部 8 项 CTest 通过；FastSort3 普通、强制标量、强制堆排序及掩码测试
通过 ASan/UBSan；九组调优配置均先通过正确性测试；原有 `toysort` 跑至千万规模，
93 条 `isCorrect` 结果全部为 1。

x86_64 版本已通过 Apple Clang 交叉编译，并在本机 x86 转译环境通过普通、强制标量及
掩码测试。该环境报告不支持 AVX2，因此实际运行并验证了自动标量回退；
**AVX2 指令路径尚未在支持 AVX2 的 x86 实机上运行验证，暂无其性能结论**。

原始数据：[首次完整测速](benchmarks/results/fast3-apple-arm64-release.csv)、
[独立种子完整测速](benchmarks/results/fast3-apple-arm64-confirmation.csv)、
[默认种子复测](benchmarks/results/fast3-apple-arm64-repeat.csv)、
[独立种子复测](benchmarks/results/fast3-apple-arm64-confirmation-repeat.csv)、
[参数调优](benchmarks/results/fast3-apple-arm64-tuning.csv)。
