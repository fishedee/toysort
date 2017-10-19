#include "heapsort.h"
#include <algorithm>
using namespace std;

HeapSort::HeapSort(){

}

HeapSort::~HeapSort(){
}

std::string HeapSort::GetName(){
	return "HeapSort";
}

std::vector<int> HeapSort::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	make_heap(result.begin(),result.end());
	sort_heap(result.begin(),result.end());
	return result;
}
