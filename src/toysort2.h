#pragma once
#include "sort.h"

class ToySort2 :public Sort {  
public:  
    ToySort2();
    ~ToySort2();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  