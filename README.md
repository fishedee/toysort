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

`toysort` 保留原来的算法对比和输出格式，FastSort、FastSort2、FastSort3 和 FastSort4 已加入快速算法组。
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

基准程序对比 FastSort、FastSort2、FastSort3、FastSort4、StdSort 和 StdStableSort。
CPU 排序保持单线程，FastSort4 可使用 GPU 并行排序；各算法依次测速。
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

## FastSort4：Metal GPU 比较排序

`FastSort4` 复用 FastSort3 的 CPU 实现，大数组在 macOS 上使用手写 Metal 比较排序。
接口保持 `Run(const std::vector<int>&)`：返回升序副本、不修改输入、不保证稳定性。
不使用标准库排序、基数排序、桶排序或计数排序；`std::sort` 仅用于测试和基准校验。
本版未实现 NPU 或 CUDA 后端；其他平台以及禁用 Metal 的构建自动使用 FastSort3。

- GPU 先在 threadgroup 内执行固定大小 bitonic 排序，再逐轮执行 Merge Path 并行归并。
  每个线程通过对角线二分确定不重叠的输出片段；归并相等元素时左侧优先。
  尾块填充 `INT_MAX` 并只写回有效长度，完整支持负数、整数极值及任意长度。
- 小于启用阈值的数组直接进入 FastSort3。大数组的升序、全相等和逆序输入在 CPU
  上直接复制或反转；其他输入进入 GPU。固定块大小下总工作量为 `O(n log n)`，
  除返回结果外，GPU 使用两个 `O(n)` 缓冲区。
- 设备、队列和计算管线首次使用时初始化，并在进程内缓存。着色器内嵌，运行时不需要
  源码目录或外部 `.metallib`。每次调用单独分配共享缓冲区，并在 GPU 完成后复制结果。
  不采用跨调用缓冲池，稳态时间包含每次分配、复制、命令提交和等待成本。
- 默认 `Auto` 模式在设备不可用、初始化失败、缓冲区分配失败、输入超出 GPU 上限或
  GPU 命令失败时，从原始输入回退 FastSort3。当前 GPU 索引上限为 `2^30` 个元素，
  同时检查设备的 `maxBufferLength`；CPU 结果分配失败仍可能抛出 `std::bad_alloc`。

```cpp
std::string warmupError;
FastSort4::WarmUp(warmupError); // 应用启动时预热，失败返回 false 并写入原因
FastSort4 automatic; // 自动选择
FastSort4 cpu(FastSort4::Mode::CpuOnly);
FastSort4 gpu(FastSort4::Mode::MetalOnly); // 强制 GPU；失败抛出 std::runtime_error
std::vector<int> result = automatic.Run(input);
auto backend = automatic.GetLastBackendName();
auto reason = automatic.GetLastFallbackReason();
```

`GetLastBackendName()` 返回实际执行路径：`Metal`、`CPU-ordered` 或 FastSort3 的
`NEON`／`AVX2`／`scalar`；调用前为 `not-run`。仅失败回退时设置原因；小数组正常
选择 CPU 不属于失败。诊断状态按实例保存，同一个实例不支持并发调用。
`MetalOnly` 跳过 CPU 的规模及有序快捷路径；空输入仅检查后端可用性，无需提交 GPU 工作。

### 构建与验证

macOS 默认启用 Metal，需要可用的 Apple SDK 和 Objective-C++ 编译器；CPU 部分保持 C++14。
CMake 最低版本为 3.16，推荐按本文原有命令使用 3.20+。Metal 后端单独启用 ARC，
链接系统 Foundation 和 Metal 框架，不引入第三方依赖。

```sh
cmake -S . -B /tmp/toysort-fast4-release -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/toysort-fast4-release -j 4
ctest --test-dir /tmp/toysort-fast4-release --output-on-failure
/tmp/toysort-fast4-release/toysort
/tmp/toysort-fast4-release/sort_benchmark > /tmp/fast4.csv

# 强制 GPU 路径；不能访问 GPU 时明确失败
/tmp/toysort-fast4-release/sort_benchmark --tune --fast4-mode metal
# CPU 路径对照
/tmp/toysort-fast4-release/sort_benchmark --tune --fast4-mode cpu
# 独立构建不包含 Metal 的版本
cmake -S . -B /tmp/toysort-fast4-cpu -DCMAKE_BUILD_TYPE=Release -DFASTSORT4_ENABLE_METAL=OFF
cmake --build /tmp/toysort-fast4-cpu -j 4
ctest --test-dir /tmp/toysort-fast4-cpu -R fastsort4 --output-on-failure

# CPU 包装层、回退和 GPU 调用的地址/未定义行为检查
cmake -S . -B /tmp/toysort-fast4-sanitize -DCMAKE_BUILD_TYPE=Debug -DFASTSORT_SANITIZERS=ON
cmake --build /tmp/toysort-fast4-sanitize --target fastsort4_test fastsort4_failure_test fastsort4_gpu_test -j 4
ctest --test-dir /tmp/toysort-fast4-sanitize -R fastsort4 --output-on-failure

# GPU 内存访问及 Metal API 验证（不用于测速）
MTL_DEBUG_LAYER=1 MTL_SHADER_VALIDATION=1 MTL_SHADER_VALIDATION_REPORT_TO_STDERR=1 \
MTL_SHADER_VALIDATION_ABORT_ON_FAULT=1 /tmp/toysort-fast4-release/fastsort4_gpu_test

# 九组参数顺序调优，并执行最终配置的完整验证和四轮基准
python3 benchmarks/tune_fast4.py --output /tmp/fast4-results
```

GPU 专项测试不可访问设备时返回 77，在 CTest 中标记为 **Skipped**；它同时检查自动
CPU 回退。这不算 GPU 测试通过。调优脚本必须在有真实 GPU 访问权限的环境运行，
将 77 视为失败。测试还用独立替身模拟写入部分结果后的命令失败，验证回退不会使用损坏结果。

调优比较分块 128/256/512 与每线程归并输出 4/8/16，按种子 `20261004` 的十万／百万
随机排列、全范围随机整数四项 GPU 中位耗时的几何平均选优，每组先通过 GPU 正确性测试。
可用 `FASTSORT4_BLOCK_SIZE`、`FASTSORT4_MERGE_ITEMS`、`FASTSORT4_GPU_THRESHOLD` 覆盖默认值。
十万规模两种随机分布均比 FastSort3 快至少 5% 时，候选启用阈值设为十万，否则设为百万；
最终使用两个种子各复测两轮完整基准验证。

`toysort` 和 `sort_benchmark` 在启动时调用 `FastSort4::WarmUp()`：初始化设备、队列和
管线，再同步完成一次 4,096 个元素的 GPU 排序，实际执行块内排序及多轮归并。
预热耗时单独输出，不计入正式排序；重复调用复用管线，但仍执行一次小排序。
GPU 不可用时预热返回 false，自动模式继续使用 FastSort3，强制 Metal 基准则报错。
`--fast4-mode cpu` 跳过 GPU 预热。预热样本独立于 `--max-size` 对正式基准的规模限制。

CSV 保留原有列，注释记录每轮实际后端、回退原因、GPU 参数及 `startup_warmup_ms`。
正式基准仍每场景预热一次、测量五次。系统 Metal 编译缓存可能已存在，启动预热耗时
不能等同于清空系统缓存后的首次安装成本。原有 `toysort` 使用 `steady_clock` 墙钟计时，
每次正式排序仍包含分配、复制、提交及 GPU 等待时间。

实现参考：[GPU Merge Path 论文](https://davidbader.net/publication/2012-gm-ba/)、
[Apple Metal 着色器验证](https://developer.apple.com/documentation/xcode/validating-your-apps-metal-shader-usage)。

### FAST4 本机验收结果

2026-10-04，Apple M3 Pro（14 核 GPU，18 GB 统一内存），macOS 26.6.2，
Apple Clang 21.0.0，Release，CPU 后端 NEON，GPU 后端 Metal。
九组参数选中 **分块 128、每线程归并 8 个输出**，调优四项几何平均耗时为
**1.933515 ms**；默认 GPU 启用阈值为 **100,000** 个元素。
这些是本机参数，不保证其他 GPU 上最优。

两个种子各复测两轮，每场景预热一次、测量五次、报告中位数。下表是
FastSort4 / FastSort3 的耗时比，小于 1 表示更快；全部 16 个重点结果通过：

| 分布 | 元素数 | 种子 20261004 | 同种子复测 | 种子 314159 | 同种子复测 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 随机排列 | 1,000,000 | 0.208 | 0.267 | 0.302 | 0.327 |
| 全范围随机整数 | 1,000,000 | 0.306 | 0.248 | 0.305 | 0.307 |
| 随机排列 | 10,000,000 | 0.153 | 0.161 | 0.165 | 0.173 |
| 全范围随机整数 | 10,000,000 | 0.155 | 0.169 | 0.160 | 0.160 |

百万和千万随机数据相对 FastSort3 加速 **3.06–6.53 倍**。百万随机数据的
中位耗时为 **3.22–4.82 ms**，千万为 **25.58–27.66 ms**。
十万规模两种随机分布在四轮中耗时比为 **0.598–0.774**，支持选择十万的启用阈值。
四轮完整 45 场景的等权几何平均耗时比分别为 **0.827、0.850、0.834、0.830**。

上述四轮历史记录中，进程内第一次百万随机整数调用为 **34.5–38.8 ms**，包含初始化成本。
此前 `toysort` 的单次对比不预热，第一次达到 GPU 阈值的排序承担初始化开销，
后续排序复用管线，因此可能出现“百万耗时 44 ms、千万仅 31 ms”。这不是百万数据
排序本身更慢。现在两个程序都在启动时单独预热 GPU，正式排序已排除该初始化开销；
比较算法吞吐量仍建议使用 `sort_benchmark` 的多次测量中位数。
更改默认值后，已有 CMake 缓存不会自动更新；旧构建可显式传入
`-DFASTSORT4_GPU_THRESHOLD=100000 -DFASTSORT4_BLOCK_SIZE=128 -DFASTSORT4_MERGE_ITEMS=8`，
或使用新的构建目录。

存在明确退化：十万规模的近乎有序数据耗时约为 FastSort3 的 **3.66–3.85 倍**，
16 种重复值约 **2.85–3.15 倍**，周期分布约 **2.72–2.80 倍**。
部分百万重复值／周期数据也退化，最高约 **1.67 倍**。当前只检测完全有序／逆序，
不根据采样识别近乎有序或低基数输入；没有所有输入均更快的保证。

验证：最终 Release 全部 **11 项 CTest** 通过（GPU 未跳过），每组调优配置先通过
**1,303 个 GPU／自动／强制 CPU 用例**；最终配置另通过 Metal API 与着色器验证。
FastSort4 通用正确性（41,025 个输入）、故障回退测试通过 ASan/UBSan；
同一 sanitizer 构建的 1,303 个 GPU／自动／CPU 专项用例另在真实设备上通过。
原有 `toysort` 完整运行到千万规模，**100 条 `isCorrect` 均为 1**。
禁用 Metal 的独立构建通过正确性与故障回退测试，GPU 专项明确跳过并验证自动 CPU 回退。

原始数据：[首次完整测速](benchmarks/results/fast4-apple-arm64-release.csv)、
[默认种子复测](benchmarks/results/fast4-apple-arm64-release-repeat.csv)、
[独立种子完整测速](benchmarks/results/fast4-apple-arm64-confirmation.csv)、
[独立种子复测](benchmarks/results/fast4-apple-arm64-confirmation-repeat.csv)、
[FastSort3 对比](benchmarks/results/fast4-apple-arm64-comparison.csv)、
[九组参数汇总](benchmarks/results/fast4-apple-arm64-tuning.csv)、
[九组原始测速](benchmarks/results/fast4-apple-arm64-tuning-raw.csv)。

加入启动预热后另行复核：`toysort` 单独报告 GPU 预热 **29.58 ms**，随后百万排序
**4.67 ms**、千万排序 **35.34 ms**，100 条正确性结果仍全部为 1；完整 11 项 CTest
再次通过。单次耗时会波动，不替代上面的多轮中位数验收。
[启动预热后主程序输出](benchmarks/results/fast4-apple-arm64-startup-warmup.txt)、
[启动预热后百万以内基准](benchmarks/results/fast4-apple-arm64-startup-warmup.csv)。
