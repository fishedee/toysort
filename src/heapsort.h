#pragma once
#include "sort.h"

class HeapSort :public Sort {  
public:  
    HeapSort();
    ~HeapSort();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  