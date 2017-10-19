#pragma once
#include "sort.h"

class InsertSort :public Sort {  
public:  
    InsertSort();
    ~InsertSort();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  