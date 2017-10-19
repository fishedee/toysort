#include "toysort1.h"
#include <stack>
#include <algorithm>
#include <climits>
using namespace std;

ToySort1::ToySort1(){
}

ToySort1::~ToySort1(){
}

std::string ToySort1::GetName(){
	return "ToySort1";
}

static void reconstruct(const std::vector<int>& data, int end,int k ,std::stack<int>& st,int maxData){
	for( ; k < end ; k++ ){
		if( data[k] > maxData ){
			st.push(k);
			maxData = data[k];
		}
	} 
}

std::vector<int> ToySort1::Run(const std::vector<int>& data){
	std::vector<int> result = data;
	std::stack<int> st;
	reconstruct(result,result.size(),0,st,-1);
	for( int i = result.size() - 1 ; i>= 1 ; i-- ){
		int maxI = st.top();
		st.pop();
		swap(result[maxI],result[i]);
		int maxData = -1;
		if(st.size() > 0 ){
			maxData = result[st.top()];
		}
		reconstruct(result,i,maxI,st,maxData);
	}
	return result;
}
