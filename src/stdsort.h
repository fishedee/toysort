#pragma once
#include "sort.h"

class StdSort :public Sort {  
public:  
    StdSort();
    ~StdSort();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  