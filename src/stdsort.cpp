#include "stdsort.h"
#include <algorithm>
using namespace std;

StdSort::StdSort(){

}

StdSort::~StdSort(){
}

std::string StdSort::GetName(){
	return "StdSort";
}

std::vector<int> StdSort::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	sort(result.begin(),result.end());
	return result;
}
