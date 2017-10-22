#pragma once
#include "sort.h"

class MapSort :public Sort {  
public:  
    MapSort();
    ~MapSort();
    std::string GetName();
   	std::vector<int> Run(const std::vector<int>& data);
};  