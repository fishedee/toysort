#include "fastsort.h"
#include <cstddef>

// Build-time overrides are used by the benchmark's parameter sweep.
#ifndef FASTSORT_INSERTION_THRESHOLD
#define FASTSORT_INSERTION_THRESHOLD 32
#endif
#ifndef FASTSORT_BLOCK_SIZE
#define FASTSORT_BLOCK_SIZE 32
#endif

namespace {
const std::ptrdiff_t insertionThreshold = FASTSORT_INSERTION_THRESHOLD;
const std::ptrdiff_t blockSize = FASTSORT_BLOCK_SIZE;
static_assert(insertionThreshold >= 2, "insertion threshold must be at least two");
static_assert(blockSize > 0 && blockSize <= 256, "offsets must fit in a byte");

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
	int* samples[9];
	int count;
	int* position;
	if(size < 128){
		samples[0] = first;
		samples[1] = first + size / 2;
		samples[2] = last - 1;
		count = 3;
		position = median(samples[0], samples[1], samples[2]);
	}else{
		for(int i = 0; i < 8; ++i) samples[i] = first + (size / 8) * i;
		samples[8] = last - 1;
		count = 9;
		position = median(median(samples[0], samples[1], samples[2]),
			median(samples[3], samples[4], samples[5]),
			median(samples[6], samples[7], samples[8]));
	}
	int equal = 0;
	for(int i = 0; i < count; ++i) equal += *samples[i] == *position;
	return {position, equal > 1};
}

// Partition [first, last) into < pivot and >= pivot. Retain unmatched offsets
// until their opposite block is available. The scalar tail also handles any
// partially consumed blocks, so every advanced block is completely resolved.
int* blockPartition(int* first, int* last, int pivot, bool& swapped){
	unsigned char leftOffsets[blockSize];
	unsigned char rightOffsets[blockSize];
	std::ptrdiff_t leftCount = 0, rightCount = 0;
	std::ptrdiff_t leftStart = 0, rightStart = 0;
	while(last - first > 2 * blockSize){
		if(leftCount == 0){
			leftStart = 0;
			for(std::ptrdiff_t i = 0; i < blockSize; ++i){
				leftOffsets[leftCount] = static_cast<unsigned char>(i);
				leftCount += !(first[i] < pivot);
			}
		}
		if(rightCount == 0){
			rightStart = 0;
			for(std::ptrdiff_t i = 0; i < blockSize; ++i){
				rightOffsets[rightCount] = static_cast<unsigned char>(i);
				rightCount += last[-1 - i] < pivot;
			}
		}
		std::ptrdiff_t count = leftCount < rightCount ? leftCount : rightCount;
		for(std::ptrdiff_t i = 0; i < count; ++i){
			exchange(first[leftOffsets[leftStart + i]], last[-1 - rightOffsets[rightStart + i]]);
		}
		swapped = swapped || count != 0;
		leftCount -= count;
		rightCount -= count;
		leftStart += count;
		rightStart += count;
		if(leftCount == 0) first += blockSize;
		if(rightCount == 0) last -= blockSize;
	}
	while(first < last){
		while(first < last && *first < pivot) ++first;
		while(first < last && !(last[-1] < pivot)) --last;
		if(first == last) break;
		exchange(*first++, *--last);
		swapped = true;
	}
	return first;
}

void sortRange(int* first, int* last, int depth){
	while(last - first > insertionThreshold){
		if(depth == 0){
			heapSort(first, last);
			return;
		}
		--depth;
		Pivot selected = choosePivot(first, last);
		int pivot = *selected.position;
		exchange(*selected.position, last[-1]);
		bool swapped = false;
		int* middle = blockPartition(first, last - 1, pivot, swapped);
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
		// Recurse only into the smaller side to bound stack usage.
		if(middle - first < last - greater){
			sortRange(first, middle, depth);
			first = greater;
		}else{
			sortRange(greater, last, depth);
			last = middle;
		}
	}
	insertionSort(first, last);
}
}

std::string FastSort::GetName(){
	return "FastSort";
}

std::vector<int> FastSort::Run(const std::vector<int>& data){
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
#ifdef FASTSORT_FORCE_HEAP
	depth = 0; // Test-only build exercises the fallback on every nontrivial input.
#endif
	sortRange(result.data(), result.data() + result.size(), depth);
	return result;
}
