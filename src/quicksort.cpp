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

static void quicksort(int* data,int left,int right){
	int first = data[left];
	int i = left + 1;
	int j = right;
	while( true ){
		while( i <= j && data[i] <= first ){
			i++;
		}
		while( i <= j && data[j] > first ){
			j--;
		}
		if( i <= j ){
			swap(data[i],data[j]);
		}else{
			break;
		}
	}
	swap(data[j],data[left]);
	if( left < j - 1 ){
		quicksort(data,left,j-1);
	}
	if( j + 1 < right ){
		quicksort(data,j+1,right);
	}
}

std::vector<int> QuickSort::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	quicksort(&result[0],0,result.size()-1);
	return result;
}
