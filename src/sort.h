#pragma once
#include <vector>
#include <string>
class Sort {  
public:  
    Sort() {}  
    virtual ~Sort(){};
    virtual std::string GetName() = 0;
    virtual std::vector<int> Run(const std::vector<int>& data) = 0;
};  