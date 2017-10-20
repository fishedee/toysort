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

static void combine(std::vector<int>& data, std::vector<int>& tmp,int left , int middle ,int right){
	int i = left;
	int j = middle ;
	int k = left;
	while( i < middle && j < right ){
		if( data[i] < data[j] ){
			tmp[k++] = data[i++];
		}else{
			tmp[k++] = data[j++];
		}
	}
	while( i < middle ){
		tmp[k++] = data[i++];
	}
	while( j < right ){
		tmp[k++] = data[j++];
	}
	copy(&tmp[left],&tmp[right],&data[left]);
}

static void mergesort(std::vector<int>& data,std::vector<int>& tmp,int left,int right){
	int k = (left+right)/2;
	if( left < k ){
		mergesort(data,tmp,left,k);
	}
	if( k + 1 < right ){
		mergesort(data,tmp,k+1,right);
	}
	combine(data,tmp,left,k+1,right+1);
}

std::vector<int> MergeSort::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	std::vector<int> tmp;
	tmp.resize(data.size());
	mergesort(result,tmp,0,result.size()-1);
	return result;
}
