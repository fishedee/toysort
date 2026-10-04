#pragma once
#include "sort.h"

class FastSort3 : public Sort {
public:
	// Reports the backend selected for this build and CPU.
	static const char* GetBackendName();
	std::string GetName() override;
	std::vector<int> Run(const std::vector<int>& data) override;
};
