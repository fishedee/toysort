#pragma once

#include <string>

template<typename T>
class Stack{
private:
	int topsize;
	int cacheSize;
	T* data;
public:
	Stack(int cacheSize){
		this->cacheSize = cacheSize;
		this->topsize = 0;
		this->data = new T[cacheSize];
	}
	int size(){
		return this->topsize;
	}
	T& top(int n = 1){
		if( this->topsize < n || this->topsize - n < 0){
			throw std::string("unexpected top");
		}
		return this->data[this->topsize-n];
	}
	void push(T data){
		if( this->topsize == this->cacheSize ){
			throw std::string("can not push,it is full!");
		}
		this->data[this->topsize++]= data;
	}
	T pop(){
		if( this->topsize < 1 ){
			throw std::string("unexpected pop");
		}
		this->topsize--;
		return this->data[this->topsize];
	}
	~Stack(){
		delete this->data;
	}
};