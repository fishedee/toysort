#pragma once
#include "sort.h"

class MergeSort :public Sort {  
public:  
    MergeSort();
    ~MergeSort();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  