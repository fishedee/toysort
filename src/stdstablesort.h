#pragma once
#include "sort.h"

class StdStableSort :public Sort {  
public:  
    StdStableSort();
    ~StdStableSort();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  