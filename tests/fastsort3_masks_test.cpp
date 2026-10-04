#include "util/fastsort3_masks.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace {
std::size_t cases = 0;

void check(const int* input, int pivot, std::ptrdiff_t width){
	using namespace fastsort3Detail;
	std::uint64_t expected = 0, right = 0, left = 0;
	for(std::ptrdiff_t i = 0; i < width; ++i){
		expected |= std::uint64_t(input[i] < pivot) << i;
		right |= std::uint64_t(input[width - 1 - i] < pivot) << i;
		left |= std::uint64_t(input[i] >= pivot) << i;
	}
	std::uint64_t actual = ScalarMasks::less(input, pivot, width);
#if FASTSORT3_HAS_NEON
	actual = NeonMasks::less(input, pivot, width);
#elif FASTSORT3_HAS_AVX2
	if(useAvx2()) actual = Avx2Masks::less(input, pivot, width);
#endif
	if(actual != expected || ((~actual) & validBits(width)) != left
		|| (reverseBits(actual) >> (64 - width)) != right)
		throw std::runtime_error("mask mismatch, width=" + std::to_string(width));
	++cases;
}
}

int main(){
	try{
		std::mt19937 random(314159);
		std::uniform_int_distribution<int> value(INT_MIN, INT_MAX);
		for(std::ptrdiff_t width : {32, 64}){
			// Exact allocation end catches overreads under ASan; varying offsets
			// exercises unaligned addresses for both NEON and AVX2.
			for(int offset = 0; offset < 16; ++offset){
				std::vector<int> storage(width + offset);
				int* input = storage.data() + offset;
				for(int trial = 0; trial < 1000; ++trial){
					for(std::ptrdiff_t i = 0; i < width; ++i) input[i] = value(random);
					for(int pivot : {INT_MIN, -1, 0, 1, INT_MAX, value(random)}) check(input, pivot, width);
				}
				for(int fill : {INT_MIN, 0, INT_MAX}){
					std::fill(input, input + width, fill);
					for(int pivot : {INT_MIN, 0, INT_MAX}) check(input, pivot, width);
				}
				for(std::ptrdiff_t bit = 0; bit < width; ++bit){
					std::fill(input, input + width, 0);
					input[bit] = -1;
					check(input, 0, width);
					std::fill(input, input + width, -1);
					input[bit] = 0;
					check(input, 0, width);
				}
			}
			std::vector<int> input(width);
			// All 16-bit patterns, repeated across a full mask, test packing.
			for(unsigned pattern = 0; pattern < 65536; ++pattern){
				for(std::ptrdiff_t i = 0; i < width; ++i)
					input[i] = pattern & (1u << (i % 16)) ? -1 : 0;
				check(input.data(), 0, width);
			}
		}
		std::cout << "PASS: " << cases << " masks, backend=" << fastsort3Detail::backendName() << '\n';
		return 0;
	}catch(const std::exception& error){
		std::cerr << "FAIL: " << error.what() << '\n';
		return 1;
	}
}
