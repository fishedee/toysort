#include "stdstablesort.h"
#include <algorithm>
using namespace std;

StdStableSort::StdStableSort(){

}

StdStableSort::~StdStableSort(){
}

std::string StdStableSort::GetName(){
	return "StdStableSort";
}

std::vector<int> StdStableSort::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	stable_sort(result.begin(),result.end());
	return result;
}
