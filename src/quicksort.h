#pragma once
#include "sort.h"

class QuickSort :public Sort {  
public:  
    QuickSort();
    ~QuickSort();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  