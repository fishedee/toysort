#pragma once
#include "sort.h"

class FastSort : public Sort {
public:
	std::string GetName() override;
	std::vector<int> Run(const std::vector<int>& data) override;
};
