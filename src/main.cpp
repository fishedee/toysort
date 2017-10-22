#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include "sort.h"
#include "stdsort.h"
#include "stdstablesort.h"
#include "heapsort.h"
#include "selectsort.h"
#include "insertsort.h"
#include "shellsort.h"
#include "quicksort.h"
#include "mergesort.h"
#include "timsort.h"
#include "mapsort.h"
#include "toysort1.h"
#include "toysort2.h"
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

void testAll(vector<Sort*> slowSorts,vector<Sort*> mediemSorts,vector<Sort*> quickSorts){
	vector<Sort*> allSorts;
	allSorts.insert(allSorts.end(),slowSorts.begin(),slowSorts.end());
	allSorts.insert(allSorts.end(),mediemSorts.begin(),mediemSorts.end());
	allSorts.insert(allSorts.end(),quickSorts.begin(),quickSorts.end());

	vector<Sort*> mediemAllSorts;
	mediemAllSorts.insert(mediemAllSorts.end(),mediemSorts.begin(),mediemSorts.end());
	mediemAllSorts.insert(mediemAllSorts.end(),quickSorts.begin(),quickSorts.end());

	test(allSorts,5);
	test(allSorts,100);
	test(allSorts,1000);
	test(allSorts,10000);

	test(mediemAllSorts,100000);
	test(mediemAllSorts,1000000);
	test(mediemAllSorts,10000000);
}

int main(){
	try{
		srand(time(0));
		vector<Sort*> slowSorts;
		slowSorts.push_back(new SelectSort());
		slowSorts.push_back(new InsertSort());
		slowSorts.push_back(new ToySort1());
		slowSorts.push_back(new MapSort());

		vector<Sort*> mediemSorts;
		mediemSorts.push_back(new ShellSort());

		vector<Sort*> quickSorts;
		quickSorts.push_back(new StdSort());
		quickSorts.push_back(new StdStableSort());
		quickSorts.push_back(new QuickSort());
		quickSorts.push_back(new TimSort());
		quickSorts.push_back(new MergeSort());
		quickSorts.push_back(new HeapSort());
		quickSorts.push_back(new ToySort2());
		testAll(slowSorts,mediemSorts,quickSorts);
	}catch(string e){
		cout<<e<<endl;
	}
	return 0;
}