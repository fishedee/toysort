#pragma once
#include "sort.h"

class ToySort1 :public Sort {  
public:  
    ToySort1();
    ~ToySort1();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  