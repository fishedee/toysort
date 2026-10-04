#include "fastsort3.h"
#include <cstddef>
#include <cstdint>
#include "util/fastsort3_masks.h"

// Build-time overrides are used by the benchmark's parameter sweep.
#ifndef FASTSORT3_INSERTION_THRESHOLD
#define FASTSORT3_INSERTION_THRESHOLD 24
#endif
#ifndef FASTSORT3_BLOCK_SIZE
#define FASTSORT3_BLOCK_SIZE 64
#endif

namespace {
const std::ptrdiff_t insertionThreshold = FASTSORT3_INSERTION_THRESHOLD;
const std::ptrdiff_t blockSize = FASTSORT3_BLOCK_SIZE;
static_assert(insertionThreshold >= 2, "insertion threshold must be at least two");
static_assert(blockSize == 32 || blockSize == 64 || blockSize == 128, "block size must be 32, 64 or 128");

// Called only for nonzero masks. Keep a C++14 fallback for other compilers.
int firstSetBit(std::uint64_t mask){
#if defined(__GNUC__) || defined(__clang__)
	return __builtin_ctzll(mask);
#else
	int bit = 0;
	while((mask & 1) == 0){ mask >>= 1; ++bit; }
	return bit;
#endif
}

void exchange(int& a, int& b){
	int saved = a;
	a = b;
	b = saved;
}

void insertionSort(int* first, int* last){
	if(first == last) return;
	for(int* i = first + 1; i < last; ++i){
		int value = *i;
		int* j = i;
		while(j > first && value < j[-1]){
			*j = j[-1];
			--j;
		}
		*j = value;
	}
}

// Always finish the current insertion before giving up, preserving all values.
bool partialInsertionSort(int* first, int* last){
	if(first == last) return true;
	std::ptrdiff_t moves = 0;
	for(int* i = first + 1; i < last; ++i){
		int value = *i;
		int* j = i;
		while(j > first && value < j[-1]){
			*j = j[-1];
			--j;
			++moves;
		}
		*j = value;
		if(moves > 8) return false;
	}
	return true;
}

void siftDown(int* first, std::ptrdiff_t root, std::ptrdiff_t size){
	int value = first[root];
	while(root < size / 2){
		std::ptrdiff_t child = root * 2 + 1;
		if(child + 1 < size && first[child] < first[child + 1]) ++child;
		if(!(value < first[child])) break;
		first[root] = first[child];
		root = child;
	}
	first[root] = value;
}

void heapSort(int* first, int* last){
	std::ptrdiff_t size = last - first;
	for(std::ptrdiff_t i = size / 2; i > 0; --i) siftDown(first, i - 1, size);
	for(std::ptrdiff_t i = size - 1; i > 0; --i){
		exchange(first[0], first[i]);
		siftDown(first, 0, i);
	}
}

int* median(int* a, int* b, int* c){
	if(*a < *b) return *b < *c ? b : (*a < *c ? c : a);
	return *a < *c ? a : (*b < *c ? c : b);
}

struct Pivot {
	int* position;
	bool repeated;
};

Pivot choosePivot(int* first, int* last){
	std::ptrdiff_t size = last - first;
	int* a = first;
	int* b = first + size / 2;
	int* c = last - 1;
	if(size >= 128){
		std::ptrdiff_t step = size / 8;
		a = median(first, first + step, first + 2 * step);
		b = median(first + 3 * step, first + 4 * step, first + 5 * step);
		c = median(first + 6 * step, first + 7 * step, last - 1);
	}
	return {median(a, b, c), *a == *b || *b == *c || *a == *c};
}

// Partition into < pivot and >= pivot. Bit masks record misplaced elements;
// only inspect nonzero masks with ctz. Retain an unfinished block until its
// opposite is available. The short tail rescans any unfinished blocks.
template<class Masks>
int* blockPartition(int* first, int* last, int pivot, bool& swapped){
	constexpr std::ptrdiff_t words = (blockSize + 63) / 64;
	std::uint64_t leftMasks[words] = {}, rightMasks[words] = {};
	std::ptrdiff_t leftWord = words, rightWord = words;
	while(last - first > 2 * blockSize){
		if(leftWord == words){
			for(std::ptrdiff_t w = 0; w < words; ++w){
				constexpr std::ptrdiff_t width = blockSize < 64 ? blockSize : 64;
				leftMasks[w] = ~Masks::less(first + w * 64, pivot, width)
					& fastsort3Detail::validBits(width);
			}
			leftWord = 0;
		}
		if(rightWord == words){
			for(std::ptrdiff_t w = 0; w < words; ++w){
				constexpr std::ptrdiff_t width = blockSize < 64 ? blockSize : 64;
				rightMasks[w] = fastsort3Detail::reverseBits(
					Masks::less(last - w * 64 - width, pivot, width)) >> (64 - width);
			}
			rightWord = 0;
		}
		while(leftWord < words && rightWord < words){
			if(leftMasks[leftWord] == 0){ ++leftWord; continue; }
			if(rightMasks[rightWord] == 0){ ++rightWord; continue; }
			std::uint64_t leftMask = leftMasks[leftWord];
			std::uint64_t rightMask = rightMasks[rightWord];
			int* left = first + leftWord * 64 + firstSetBit(leftMask);
			int* right = last - 1 - rightWord * 64 - firstSetBit(rightMask);
			int saved = *left;
			*left = *right;
			leftMask &= leftMask - 1;
			rightMask &= rightMask - 1;
			while(leftMask != 0 && rightMask != 0){
				left = first + leftWord * 64 + firstSetBit(leftMask);
				*right = *left;
				right = last - 1 - rightWord * 64 - firstSetBit(rightMask);
				*left = *right;
				leftMask &= leftMask - 1;
				rightMask &= rightMask - 1;
			}
			*right = saved;
			leftMasks[leftWord] = leftMask;
			rightMasks[rightWord] = rightMask;
			swapped = true;
		}
		if(leftWord == words) first += blockSize;
		if(rightWord == words) last -= blockSize;
	}
	// The short tail uses conditional moves instead of unpredictable scans.
	for(int* scan = first; scan < last; ++scan){
		int value = *scan;
		bool less = value < pivot;
		int saved = *first;
		*first = less ? value : saved;
		*scan = less ? saved : value;
		first += less;
		swapped = swapped || less;
	}
	return first;
}

void breakPattern(int* first, int* last){
	std::ptrdiff_t size = last - first;
	if(size < 8) return;
	exchange(first[size / 4], first[size / 2]);
	exchange(first[size / 2 - 1], first[3 * (size / 4)]);
}

template<class Masks>
void sortRange(int* first, int* last, int depth, int badStreak = 0){
	while(last - first > insertionThreshold){
		if(depth == 0){
			heapSort(first, last);
			return;
		}
		--depth;
		if(badStreak >= 2){
			breakPattern(first, last);
			badStreak = 0;
		}
		Pivot selected = choosePivot(first, last);
		int pivot = *selected.position;
		exchange(*selected.position, last[-1]);
		bool swapped = false;
		int* middle = blockPartition<Masks>(first, last - 1, pivot, swapped);
		exchange(*middle, last[-1]);
		int* greater = middle + 1;
		if(selected.repeated){
			// All values here are >= pivot; remove its entire equivalence class.
			for(int* i = greater; i < last; ++i){
				if(*i == pivot){
					exchange(*greater++, *i);
				}
			}
		}
		if(!swapped){
			bool leftSorted = partialInsertionSort(first, middle);
			bool rightSorted = partialInsertionSort(greater, last);
			if(leftSorted && rightSorted) return;
		}
		std::ptrdiff_t size = last - first;
		badStreak = (middle - first < size / 8 || last - greater < size / 8) ? badStreak + 1 : 0;
		// Recurse only into the smaller side to bound stack usage.
		if(middle - first < last - greater){
			sortRange<Masks>(first, middle, depth, badStreak);
			first = greater;
		}else{
			sortRange<Masks>(greater, last, depth, badStreak);
			last = middle;
		}
	}
	insertionSort(first, last);
}
}

std::string FastSort3::GetName(){
	return "FastSort3";
}

std::vector<int> FastSort3::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	if(result.size() < 2) return result;
	bool ascending = true, descending = true;
	for(std::size_t i = 1; i < result.size() && (ascending || descending); ++i){
		ascending = ascending && !(result[i] < result[i - 1]);
		descending = descending && !(result[i - 1] < result[i]);
	}
	if(ascending) return result;
	if(descending){
		for(std::size_t i = 0, j = result.size() - 1; i < j; ++i, --j) exchange(result[i], result[j]);
		return result;
	}
	int depth = 0;
	for(std::size_t size = result.size(); size > 1; size >>= 1) depth += 2;
#ifdef FASTSORT3_FORCE_HEAP
	depth = 0; // Test-only build exercises the fallback on every nontrivial input.
#endif
#if FASTSORT3_HAS_NEON
	sortRange<fastsort3Detail::NeonMasks>(result.data(), result.data() + result.size(), depth);
#elif FASTSORT3_HAS_AVX2
	if(fastsort3Detail::useAvx2())
		sortRange<fastsort3Detail::Avx2Masks>(result.data(), result.data() + result.size(), depth);
	else
		sortRange<fastsort3Detail::ScalarMasks>(result.data(), result.data() + result.size(), depth);
#else
	sortRange<fastsort3Detail::ScalarMasks>(result.data(), result.data() + result.size(), depth);
#endif
	return result;
}

const char* FastSort3::GetBackendName(){
	return fastsort3Detail::backendName();
}
