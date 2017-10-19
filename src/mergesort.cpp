#include "mergesort.h"
#include <algorithm>
using namespace std;

MergeSort::MergeSort(){

}

MergeSort::~MergeSort(){
}

std::string MergeSort::GetName(){
	return "MergeSort";
}

std::vector<int> MergeSort::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	stable_sort(result.begin(),result.end());
	return result;
}
