#pragma once
#include "sort.h"

class TimSort :public Sort {  
public:  
    TimSort();
    ~TimSort();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  