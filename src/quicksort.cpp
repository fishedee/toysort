#include "quicksort.h"
#include <algorithm>
using namespace std;

QuickSort::QuickSort(){

}

QuickSort::~QuickSort(){
}

std::string QuickSort::GetName(){
	return "QuickSort";
}

std::vector<int> QuickSort::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	sort(result.begin(),result.end());
	return result;
}
