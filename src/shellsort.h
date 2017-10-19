#pragma once
#include "sort.h"

class ShellSort :public Sort {  
public:  
    ShellSort();
    ~ShellSort();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  