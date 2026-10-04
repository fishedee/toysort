#include "fastsort4.h"
#include "fastsort3.h"
#include "metal/fastsort4_backend.h"
#include <algorithm>
#include <stdexcept>

#ifndef FASTSORT4_GPU_THRESHOLD
#define FASTSORT4_GPU_THRESHOLD 100000
#endif
#ifndef FASTSORT4_BLOCK_SIZE
#define FASTSORT4_BLOCK_SIZE 128
#endif
#ifndef FASTSORT4_MERGE_ITEMS
#define FASTSORT4_MERGE_ITEMS 8
#endif
static_assert(FASTSORT4_GPU_THRESHOLD >= 2, "GPU threshold must be at least two");
static_assert(FASTSORT4_BLOCK_SIZE == 128 || FASTSORT4_BLOCK_SIZE == 256 || FASTSORT4_BLOCK_SIZE == 512,
	"Metal block size must be 128, 256 or 512");
static_assert(FASTSORT4_MERGE_ITEMS == 4 || FASTSORT4_MERGE_ITEMS == 8 || FASTSORT4_MERGE_ITEMS == 16,
	"Metal merge items must be 4, 8 or 16");

#ifndef FASTSORT4_HAS_METAL
namespace fastsort4Detail {
bool metalSort(const std::vector<int>&, std::vector<int>&, std::string& error){
	error = "Metal backend not built";
	return false;
}
}
#endif

std::string FastSort4::GetName(){ return "FastSort4"; }
std::size_t FastSort4::GetGpuThreshold(){ return FASTSORT4_GPU_THRESHOLD; }
int FastSort4::GetBlockSize(){ return FASTSORT4_BLOCK_SIZE; }
int FastSort4::GetMergeItems(){ return FASTSORT4_MERGE_ITEMS; }

bool FastSort4::WarmUp(std::string& error){
	error.clear();
	std::vector<int> input(4096), output;
	for(std::size_t i = 0; i < input.size(); ++i)
		input[i] = static_cast<int>(input.size() - 1 - i) - 2048;
	return fastsort4Detail::metalSort(input, output, error);
}

std::vector<int> FastSort4::Run(const std::vector<int>& data){
	backend_ = "not-run";
	reason_.clear();
	if(mode_ == Mode::CpuOnly || (mode_ == Mode::Auto && data.size() < GetGpuThreshold())){
		backend_ = FastSort3::GetBackendName();
		return FastSort3().Run(data);
	}
	if(mode_ == Mode::Auto){
		bool ascending = true, descending = true;
		for(std::size_t i = 1; i < data.size() && (ascending || descending); ++i){
			ascending = ascending && data[i - 1] <= data[i];
			descending = descending && data[i - 1] >= data[i];
		}
		if(ascending || descending){
			backend_ = "CPU-ordered";
			std::vector<int> result = data;
			if(!ascending) std::reverse(result.begin(), result.end());
			return result;
		}
	}
	std::vector<int> result;
	if(fastsort4Detail::metalSort(data, result, reason_)){
		backend_ = "Metal";
		return result;
	}
	if(mode_ == Mode::MetalOnly) throw std::runtime_error("FastSort4 Metal: " + reason_);
	backend_ = FastSort3::GetBackendName();
	return FastSort3().Run(data);
}
