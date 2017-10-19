#include "selectsort.h"
#include <algorithm>
using namespace std;

SelectSort::SelectSort(){

}

SelectSort::~SelectSort(){
}

std::string SelectSort::GetName(){
	return "SelectSort";
}

std::vector<int> SelectSort::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	for( int i = 0 ;i < result.size();i++){
		int min = i;
		for( int j = i+1 ; j < result.size() ; j++ ){
			if(result[j] < result[min]){
				min = j;
			}
		}
		swap(result[i],result[min]);
	}
	return result;
}
