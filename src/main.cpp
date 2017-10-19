#include <iostream>
#include <vector>
#include <ctime>
#include "sort.h"
#include "quicksort.h"
#include "mergesort.h"
#include "heapsort.h"
using namespace std;

struct RunResult{
	double time;
	bool isCorrect;
};

RunResult testSingle(Sort * sort,const vector<int>& data){
	clock_t begin = clock();
	vector<int> result = sort->Run(data);
	clock_t end = clock();

	struct RunResult runResult;
	runResult.time = ((double)(end-begin))/CLOCKS_PER_SEC*1000;
	runResult.isCorrect = true;
	for (size_t i = 0 ;i != result.size() ; i++){
		if( result[i] != i ){
			runResult.isCorrect = false;
			break;
		}
	}
	return runResult;
}

void  test(vector<Sort*> sorts,size_t number){
	vector<int> data;
	for(size_t i = 0 ;i != number ; i++ ){
		data.push_back(i);
	}
	random_shuffle ( data.begin(), data.end() );

	cout<<endl<<"-------  test: "<<number<<" -------"<<endl;
	for(size_t i = 0 ; i != sorts.size();i++ ){
		RunResult result = testSingle(sorts[i],data);
		cout<<"["<<sorts[i]->GetName()<<"] time: "<<result.time<<" ms,isCorrect: "<<result.isCorrect<<endl;
	} 
}

int main(){
	vector<Sort*> sorts;
	sorts.push_back(new QuickSort());
	sorts.push_back(new MergeSort());
	sorts.push_back(new HeapSort());

	test(sorts,10000);
	test(sorts,100000);
	test(sorts,1000000);
	return 0;
}