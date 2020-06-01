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

static void quicksort2(int* data,int left,int right){
	//总是让第一个小于第二个
	if( data[left] > data[left+1] ){
		swap(data[left],data[left+1]);
	}
	if( right == left + 1 ){
		//只有两个元素的时候
		return;
	}

	int first = data[left];
	int second = data[left+1];
	int i = left + 2;
	int j = right;
	int k = left + 2;
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
	if( i - k != 0 ){
		//有中间数据
		data[left] = data[k-1];
		data[left+1] = data[k-2];
		data[k-1] = data[i-1];
		data[k-2] = first;
		data[i-1] = second;
		k = k-2;
	}else {
		//没有中间数据
		data[left] = data[k-1];
		data[left+1] = data[k-2];
		data[k-2] = first;
		data[k-1] = second;
		k = k - 2;
	}
	if( left < k - 1 ){
		quicksort2(data,left,k-1);
	}
	if( k + 1 < i - 2 ){
		quicksort2(data,k+1,i-2);
	}
	if( i < right ){
		quicksort2(data,i,right);
	}
}

std::vector<int> QuickSort2::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	quicksort2(&result[0],0,result.size()-1);
	return result;
}
