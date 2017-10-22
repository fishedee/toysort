#include "heapsort.h"
#include <algorithm>
#include <iostream>
using namespace std;

HeapSort::HeapSort(){

}

HeapSort::~HeapSort(){
}

std::string HeapSort::GetName(){
	return "HeapSort";
}

static void heapDown(int* data,int i,int size){
	int temp = data[i];
	int k =  i;
	while( k * 2 <= size ){
		int leftChild = k * 2;
		int rightChild = k * 2 + 1;
		int child = 0;
		if( rightChild <= size && data[rightChild] > data[leftChild] ){
			child = rightChild;
		}else{
			child = leftChild;
		}
		if( data[child] > temp ){
			data[k] = data[child];
			k = child;
		}else{
			break;
		}
	}
	data[k] = temp;
}

static void heapSort(int* data,int size){
	for( int i = size/2; i >= 1 ;i-- ){
		heapDown(data,i,size);
	}
	for( int i = size ; i > 1 ; i-- ){
		swap(data[i],data[1]);
		heapDown(data,1,i-1);
	}
}

std::vector<int> HeapSort::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	heapSort(&result[0]-1,result.size());
	return result;
}
