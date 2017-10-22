#include "toysort2.h"
#include <stack>
#include <algorithm>
#include <climits>
#include <iostream>
using namespace std;

ToySort2::ToySort2(){
}

ToySort2::~ToySort2(){
}

std::string ToySort2::GetName(){
	return "ToySort2";
}

template<typename T>
class Heap{
private:
	T* data;
	int size;
	int cacheSize;
	T* cacheData;
private:
	void heapUp(int i){
		T temp = this->data[i];
		int k =  i;
		while( k /2 >= 1 ){
			int parent = k / 2;
			if( this->data[parent] < temp ){
				this->data[k] = this->data[parent];
				k = parent;
			}else{
				break;
			}
		}
		this->data[k] = temp;
	}
	void heapDown(int i){
		T temp = this->data[i];
		int k =  i;
		while( k * 2 <= this->size ){
			int leftChild = k * 2;
			int rightChild = k * 2 + 1;
			int child = 0;
			if( rightChild <= this->size && this->data[leftChild] < this->data[rightChild] ){
				child = rightChild;
			}else{
				child = leftChild;
			}
			if( temp < this->data[child] ){
				this->data[k] = this->data[child];
				k = child;
			}else{
				break;
			}
		}
		this->data[k] = temp;
	}

public:
	Heap(int size){
		this->cacheSize = size;
		this->cacheData = new T[size];
		this->data = this->cacheData-1;
		this->size = 0;
	}
	~Heap(){
		delete []this->cacheData;
	}
	T& top(){
		if( this->size == 0 ){
			throw string("heap is empty1!");
		}
		return this->data[1];
	}
	int getSize(){
		return this->size;
	}
	void pop(){
		if( this->size == 0 ){
			throw string("heap is empty2!");
		}
		swap(this->data[1],this->data[this->size]);
		this->size --;
		this->heapDown(1);
	}
	void push(const T& item){
		if( this->size == this->cacheSize  ){
			throw string("heap is full!");
		}
		this->size ++;
		this->data[this->size] = item;
		this->heapUp(this->size);
	}
	void adjust(){
		if( this->size == 0 ){
			throw string("heap is empty3!");
		}
		this->heapDown(1);
	}
};

class Seg{
private:
	int begin;
	int end;
	const int *data;
	vector<int> *st;
private:
	void init(const int * data,int begin,int end){
		this->data = data;
		this->begin = begin;
		this->end = end;
		this->st = new vector<int>();
		int maxData = -1;
		for( int k = this->begin; k < this->end ; k++ ){
			if( this->data[k] > maxData ){
				this->st->push_back(k);
				maxData = data[k];
			}
		}
	}

public:
	Seg(){
		this->data = NULL;
	}
	Seg(const int * data,int begin,int end){
		this->init(data,begin,end);
	}
	bool isValid(){
		return this->data != NULL;
	}
	void pop(int*max , Seg* newSeg){
		int maxId = *this->st->rbegin();
		this->st->pop_back();
		*max = this->data[maxId];
		if( this->st->size() == 0 ){
			this->begin = maxId + 1;
			if( this->end > this->begin ){
				this->init(this->data,this->begin,this->end);
			}else{
				this->data = NULL;
			}
		}else{
			if( this->end > maxId + 1 ){
				newSeg->init(this->data,maxId+1,this->end);
			}
			this->end = maxId;
		}
	}
	bool operator<(Seg& b){
		int alast = *st->rbegin();
		int blast = *b.st->rbegin();
		return data[alast]<data[blast];
	}
};

std::vector<int> ToySort2::Run(const std::vector<int>& data){
	Heap<Seg> heap(data.size());
	std::vector<int> result;
	result.resize(data.size());
	Seg firstSeg(&data[0],0,data.size());
	heap.push(firstSeg);
	for( int i = data.size() - 1 ; i >= 0 ;i -- ){

		Seg& seg = heap.top();
		int max;
		Seg nextSeg;
		seg.pop(&max,&nextSeg);
		if( seg.isValid() == false ){
			heap.pop();
		}else{
			heap.adjust();
		}
		if( nextSeg.isValid() == true ){
			heap.push(nextSeg);
		}
		result[i] = max;
	}
	return result;
}
