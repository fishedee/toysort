#include "quicksort2.h"
#include <algorithm>
#include <iostream>
using namespace std;

QuickSort2::QuickSort2(){

}

QuickSort2::~QuickSort2(){
}

std::string QuickSort2::GetName(){
	return "QuickSort2";
}

static void insertsort(int *data,int left, int right){
	for( int i = left+1 ;i <= right;i++){
		int temp = data[i];
		int j = i - 1;
		for( ; j >= left ; j-- ){
			if(data[j] > temp ){
				data[j+1] = data[j];
			}else{
				break;
			}
		}
		data[j+1] = temp;
	}
}


static void quicksort2(int* data,int left,int right){
	if( right - left <= 12 ){
		insertsort(data,left,right);
		return;
	}
	//总是让第一个小于第二个
	if( data[left] > data[right] ){
		swap(data[left],data[right]);
	}
	int first = data[left];
	int second = data[right];
	int i = left + 1;
	int j = right - 1;
	int k = left + 1;
	while( true ){
		while( i <= j && data[i] <= second ){
			if( data[i] < first ){
				swap(data[i],data[k]);
				k++;
			}
			i++;
		}
		while( i <= j && data[j] > second ){
			j--;
		}
		if( i <= j ){
			swap(data[i],data[j]);
			if( data[i] < first ){
				swap(data[i],data[k]);
				k++;
			}
			i++;
			j--;
		}else{
			break;
		}
	}
	swap(data[k-1],data[left]);
	swap(data[i],data[right]);
	if( left < k - 2 ){
		quicksort2(data,left,k-2);
	}
	if( k < i-1 ){
		quicksort2(data,k,i-1);
	}
	if( i+1 < right ){
		quicksort2(data,i+1,right);
	}
}

std::vector<int> QuickSort2::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	quicksort2(&result[0],0,result.size()-1);
	return result;
}
