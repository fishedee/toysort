#pragma once
#include "sort.h"
#include <cstddef>

class FastSort4 : public Sort {
public:
	enum class Mode { Auto, CpuOnly, MetalOnly };
	explicit FastSort4(Mode mode = Mode::Auto) : mode_(mode) {}
	std::string GetName() override;
	std::vector<int> Run(const std::vector<int>& data) override;
	// Per-instance diagnostics; an instance must not be used concurrently.
	const std::string& GetLastBackendName() const { return backend_; }
	const std::string& GetLastFallbackReason() const { return reason_; }
	// Initializes Metal and executes a small GPU sort before measured work.
	static bool WarmUp(std::string& error);
	static std::size_t GetGpuThreshold();
	static int GetBlockSize();
	static int GetMergeItems();
private:
	Mode mode_;
	std::string backend_ = "not-run";
	std::string reason_;
};
