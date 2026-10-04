#pragma once
#include <cstddef>
#include <cstdint>
#include <climits>

#ifndef FASTSORT3_ENABLE_SIMD
#define FASTSORT3_ENABLE_SIMD 1
#endif

#if FASTSORT3_ENABLE_SIMD && defined(__aarch64__) && defined(__ARM_NEON)
#define FASTSORT3_HAS_NEON 1
#include <arm_neon.h>
#else
#define FASTSORT3_HAS_NEON 0
#endif

#if FASTSORT3_ENABLE_SIMD && (defined(__x86_64__) || defined(__i386__)) \
	&& (defined(__GNUC__) || defined(__clang__))
#define FASTSORT3_HAS_AVX2 1
#include <immintrin.h>
#else
#define FASTSORT3_HAS_AVX2 0
#endif

// Internal helpers, also exercised directly by the mask tests. Width is 32 or 64;
// bit i describes first[i] < pivot. Loads never extend beyond that width.
namespace fastsort3Detail {
inline std::uint64_t validBits(std::ptrdiff_t width){
	return width == 64 ? UINT64_MAX : (std::uint64_t(1) << width) - 1;
}

inline std::uint64_t reverseBits(std::uint64_t value){
#if defined(__clang__)
#if __has_builtin(__builtin_bitreverse64)
	return __builtin_bitreverse64(value);
#endif
#endif
	value = ((value & UINT64_C(0x5555555555555555)) << 1) | ((value >> 1) & UINT64_C(0x5555555555555555));
	value = ((value & UINT64_C(0x3333333333333333)) << 2) | ((value >> 2) & UINT64_C(0x3333333333333333));
	value = ((value & UINT64_C(0x0f0f0f0f0f0f0f0f)) << 4) | ((value >> 4) & UINT64_C(0x0f0f0f0f0f0f0f0f));
	value = ((value & UINT64_C(0x00ff00ff00ff00ff)) << 8) | ((value >> 8) & UINT64_C(0x00ff00ff00ff00ff));
	value = ((value & UINT64_C(0x0000ffff0000ffff)) << 16) | ((value >> 16) & UINT64_C(0x0000ffff0000ffff));
	return (value << 32) | (value >> 32);
}

struct ScalarMasks {
	static std::uint64_t less(const int* first, int pivot, std::ptrdiff_t width){
		std::uint64_t mask = 0;
		for(std::ptrdiff_t i = 0; i < width; ++i)
			mask |= std::uint64_t(first[i] < pivot) << i;
		return mask;
	}
};

#if FASTSORT3_HAS_NEON
static_assert(sizeof(int) * CHAR_BIT == 32, "NEON requires 32-bit int");
struct NeonMasks {
	static std::uint64_t less(const int* first, int pivot, std::ptrdiff_t width){
		const int32x4_t pivots = vdupq_n_s32(pivot);
		const std::uint8_t weightsData[16] = {1, 2, 4, 8, 16, 32, 64, 128, 1, 2, 4, 8, 16, 32, 64, 128};
		const uint8x16_t weights = vld1q_u8(weightsData);
		std::uint64_t mask = 0;
		for(std::ptrdiff_t i = 0; i < width; i += 16){
			uint32x4_t a = vcltq_s32(vld1q_s32(first + i), pivots);
			uint32x4_t b = vcltq_s32(vld1q_s32(first + i + 4), pivots);
			uint32x4_t c = vcltq_s32(vld1q_s32(first + i + 8), pivots);
			uint32x4_t d = vcltq_s32(vld1q_s32(first + i + 12), pivots);
			uint16x8_t ab = vcombine_u16(vmovn_u32(a), vmovn_u32(b));
			uint16x8_t cd = vcombine_u16(vmovn_u32(c), vmovn_u32(d));
			uint8x16_t packed = vcombine_u8(vmovn_u16(ab), vmovn_u16(cd));
			uint64x2_t sums = vpaddlq_u32(vpaddlq_u16(vpaddlq_u8(vandq_u8(packed, weights))));
			std::uint64_t bits = vgetq_lane_u64(sums, 0) | (vgetq_lane_u64(sums, 1) << 8);
			mask |= bits << i;
		}
		return mask;
	}
};
#endif

#if FASTSORT3_HAS_AVX2
static_assert(sizeof(int) * CHAR_BIT == 32, "AVX2 requires 32-bit int");
inline bool useAvx2(){
	static const bool supported = __builtin_cpu_supports("avx2");
	return supported;
}

struct Avx2Masks {
	// Keep AVX2 instructions inside this function; callers remain baseline x86.
	__attribute__((target("avx2")))
	static std::uint64_t less(const int* first, int pivot, std::ptrdiff_t width){
		const __m256i pivots = _mm256_set1_epi32(pivot);
		std::uint64_t mask = 0;
		for(std::ptrdiff_t i = 0; i < width; i += 8){
			__m256i values = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(first + i));
			__m256i comparison = _mm256_cmpgt_epi32(pivots, values);
			unsigned bits = static_cast<unsigned>(_mm256_movemask_ps(_mm256_castsi256_ps(comparison)));
			mask |= std::uint64_t(bits) << i;
		}
		return mask;
	}
};
#endif

inline const char* backendName(){
#if FASTSORT3_HAS_NEON
	return "NEON";
#elif FASTSORT3_HAS_AVX2
	return useAvx2() ? "AVX2" : "scalar";
#else
	return "scalar";
#endif
}
}
