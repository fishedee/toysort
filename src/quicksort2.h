#pragma once
#include "sort.h"

class QuickSort2 :public Sort {  
public:  
    QuickSort2();
    ~QuickSort2();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  