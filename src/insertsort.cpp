#include "insertsort.h"
#include <algorithm>
using namespace std;

InsertSort::InsertSort(){

}

InsertSort::~InsertSort(){
}

std::string InsertSort::GetName(){
	return "InsertSort";
}

std::vector<int> InsertSort::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	for( int i = 1 ;i < result.size();i++){
		int data = result[i];
		int j = i - 1;
		for( ; j >= 0 ; j-- ){
			if(result[j] > data ){
				result[j+1] = result[j];
			}else{
				break;
			}
		}
		result[j+1] = data;
	}
	return result;
}
