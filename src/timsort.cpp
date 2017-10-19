#include "timsort.h"
#include <algorithm>
using namespace std;

TimSort::TimSort(){

}

TimSort::~TimSort(){
}

std::string TimSort::GetName(){
	return "TimSort";
}

static void insertSort(std::vector<int>& result ,int begin,int end){
	for( int i = begin ;i < end;i++){
		int data = result[i];
		int j = i - 1;
		for( ; j >= begin ; j-- ){
			if(result[j] > data ){
				result[j+1] = result[j];
			}else{
				break;
			}
		}
		result[j+1] = data;
	}
}

static int findNextRun( std::vector<int>& data , int i ,int minRun ){
	if( i == data.size() - 1 ){
		return data.size();
	}
	int j = 0;
	if( data[i+1] < data[i] ){
		//降序
		j = i + 1;
		while( j < data.size() && data[j] < data[j-1] ){
			j++;
		}
		reverse(&data[i],&data[j]);
	}else{
		//升序
		j = i + 1;
		while( j < data.size() && data[j] > data[j-1]){
			j++;
		}
	}
	int len = j - i;
	if( len < minRun ){
		int nextJ = i + minRun;
		if( nextJ >= data.size() ){
			nextJ = data.size();
		}
		insertSort(data,i,nextJ);
		j = nextJ;
	}
	return j;
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
	for(k = left ; k < right ; k++ ){
		data[k] = tmp[k];
	}
}

struct StackInfo{
	int begin;
	int len;
};

static bool combineTop2(std::vector<int>& result,std::vector<int>& tmp,std::vector<StackInfo>& stack){
	int stSize = stack.size();
	if( stSize == 1 ){
		return false;
	}
	StackInfo top1 = stack[stSize-1];
	StackInfo top2 = stack[stSize-2];
	combine(result,tmp,top2.begin,top1.begin,top1.begin+top1.len);
	StackInfo newTop;
	newTop.begin = top2.begin;
	newTop.len = top2.len + top1.len;
	stack.pop_back();
	stack.pop_back();
	stack.push_back(newTop);
	return true;
}

static bool combineStack(std::vector<int>& result,std::vector<int>& tmp,std::vector<StackInfo>& stack){
	int stSize = stack.size();

	if( stack.size() >= 2 && stack[stSize-1].len > stack[stSize-2].len ){
		combineTop2(result,tmp,stack);
		return true;
	}else if( stack.size() >= 3 && stack[stSize-1].len + stack[stSize-2].len > stack[stSize-3].len ){
		combineTop2(result,tmp,stack);
		combineTop2(result,tmp,stack);
		return true;
	}else{
		return false;
	}
}

std::vector<int> TimSort::Run(const std::vector<int>& data){
	std::vector<int> tmp;
	std::vector<int> result = data;
	std::vector<StackInfo> stack;
	tmp.resize(data.size());
	
	int runLastI = 0;
	int runI = 0;
	while( runLastI != result.size()){
		runI = findNextRun(result,runI,10);
		StackInfo stackInfo;
		stackInfo.begin = runLastI;
		stackInfo.len = runI - runLastI;
		stack.push_back(stackInfo);
		runLastI = runI;
		while(combineStack(result,tmp,stack)){
			;
		}
	}
	while(combineTop2(result,tmp,stack)){
		;
	}
	return result;
}
