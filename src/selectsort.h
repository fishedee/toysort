#pragma once
#include "sort.h"

class SelectSort :public Sort {  
public:  
    SelectSort();
    ~SelectSort();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  