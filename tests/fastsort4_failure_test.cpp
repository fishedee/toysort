#include "fastsort4.h"
#include "metal/fastsort4_backend.h"
#include <iostream>
#include <stdexcept>

namespace fastsort4Detail {
bool metalSort(const std::vector<int>&, std::vector<int>& output, std::string& error){
	output = {42}; // Simulate a failure after partially modifying the GPU result.
	error = "injected command failure";
	return false;
}
}
int main(){
	std::string error = "stale";
	if(FastSort4::WarmUp(error) || error != "injected command failure") return 1;
	const std::vector<int> input = {3, -1, 2, 0}, expected = {-1, 0, 2, 3};
	FastSort4 automatic, cpu(FastSort4::Mode::CpuOnly), gpu(FastSort4::Mode::MetalOnly);
	if(automatic.Run(input) != expected || automatic.GetLastFallbackReason() != "injected command failure"
		|| automatic.GetLastBackendName() == "Metal" || cpu.Run(input) != expected
		|| !cpu.GetLastFallbackReason().empty()) return 1;
	try { gpu.Run(input); return 1; }
	catch(const std::runtime_error&) {}
	if(automatic.Run({1, 2, 3}) != std::vector<int>({1, 2, 3})
		|| !automatic.GetLastFallbackReason().empty() || automatic.GetLastBackendName() != "CPU-ordered") return 1;
	std::cout << "PASS: original-input fallback, strict GPU failure, diagnostics reset\n";
}
