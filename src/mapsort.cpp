#include "mapsort.h"
#include <map>
#include <set>
#include <algorithm>
using namespace std;

MapSort::MapSort(){

}

MapSort::~MapSort(){
}

std::string MapSort::GetName(){
	return "MapSort";
}

std::vector<int> MapSort::Run(const std::vector<int>& data){
	std::set<int> mapData(data.begin(),data.end());
	std::vector<int> result;
	result.resize(data.size());
	copy(mapData.begin(),mapData.end(),result.begin());
	return result;
}
