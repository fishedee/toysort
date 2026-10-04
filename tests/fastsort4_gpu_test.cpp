#include "fastsort4.h"
#include "sort_data.h"
#include <iostream>

namespace {
std::size_t cases = 0;
void check(FastSort4& sorter, const std::vector<int>& input){
	std::vector<int> original = input, expected = input;
	std::sort(expected.begin(), expected.end());
	if(sorter.Run(input) != expected || input != original)
		throw std::runtime_error("incorrect result/input changed, size=" + std::to_string(input.size()));
	++cases;
}
}
int main(){
	try{
		FastSort4 gpu(FastSort4::Mode::MetalOnly), automatic, cpu(FastSort4::Mode::CpuOnly);
		std::string warmupError;
		bool warmed = FastSort4::WarmUp(warmupError);
		try { check(gpu, {3, -1, 2}); }
		catch(const std::runtime_error& error){
			std::string reason = gpu.GetLastFallbackReason();
			if(reason != "no Metal device" && reason != "Metal backend not built") throw;
			if(warmed || warmupError != reason) throw std::runtime_error("incorrect unavailable warmup");
			check(automatic, sortData::make(1000001, "random", 314159));
			if(automatic.GetLastFallbackReason() != reason) throw std::runtime_error("missing fallback reason");
			std::cout << "SKIP: " << error.what() << "; automatic CPU fallback verified\n";
			return 77;
		}
		if(!warmed || !warmupError.empty()) throw std::runtime_error("GPU warmup failed");
		warmupError = "stale";
		if(!FastSort4::WarmUp(warmupError) || !warmupError.empty()) throw std::runtime_error("repeated warmup failed");
		if(gpu.GetLastBackendName() != "Metal") throw std::runtime_error("GPU did not execute");
		for(std::size_t size = 0; size <= 530; ++size){
			check(gpu, sortData::make(size, "random", 314159));
			std::vector<int> extremes(size);
			for(std::size_t i = 0; i < size; ++i) extremes[i] = i % 3 == 0 ? INT_MIN : (i % 3 == 1 ? INT_MAX : 0);
			check(gpu, extremes);
		}
		for(std::size_t size : {1023u, 1024u, 1025u, 2047u, 2048u, 2049u, 65535u, 65536u, 65537u,
			999999u, 1000000u, 1000001u, 10000000u}){
			for(const auto& distribution : sortData::distributions()){
				auto data = sortData::make(size, distribution, 20261004);
				check(gpu, data);
				if(gpu.GetLastBackendName() != "Metal") throw std::runtime_error("GPU silently fell back");
				check(automatic, data);
				if(!automatic.GetLastFallbackReason().empty()) throw std::runtime_error("unexpected fallback");
			}
		}
		for(std::size_t size : {FastSort4::GetGpuThreshold() - 1, FastSort4::GetGpuThreshold(), FastSort4::GetGpuThreshold() + 1}){
			auto data = sortData::make(size, "random", 314159);
			check(automatic, data);
			if((automatic.GetLastBackendName() == "Metal") != (size >= FastSort4::GetGpuThreshold()))
				throw std::runtime_error("incorrect automatic dispatch");
			check(cpu, data);
		}
		std::cout << "PASS: " << cases << " GPU/automatic/CPU cases\n";
	}catch(const std::exception& error){ std::cerr << "FAIL: " << error.what() << '\n'; return 1; }
}
