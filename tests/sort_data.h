#pragma once
#include <algorithm>
#include <climits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace sortData {
inline const std::vector<std::string>& distributions(){
	static const std::vector<std::string> names = {
		"permutation", "random", "sorted", "reversed", "nearly_sorted", "duplicates", "equal", "periodic", "organ_pipe"
	};
	return names;
}

inline std::vector<int> make(std::size_t size, const std::string& distribution, unsigned seed){
	std::vector<int> data(size);
	std::mt19937 random(seed);
	std::iota(data.begin(), data.end(), 0);
	if(distribution == "permutation"){
		std::shuffle(data.begin(), data.end(), random);
	}else if(distribution == "random"){
		std::uniform_int_distribution<int> value(INT_MIN, INT_MAX);
		for(int& item : data) item = value(random);
	}else if(distribution == "reversed"){
		std::reverse(data.begin(), data.end());
	}else if(distribution == "nearly_sorted"){
		if(size > 1){
			std::uniform_int_distribution<std::size_t> index(0, size - 1);
			for(std::size_t i = 0; i < std::max<std::size_t>(1, size / 1000); ++i){
				std::size_t a = index(random), b = index(random);
				std::swap(data[a], data[b]);
			}
		}
	}else if(distribution == "duplicates"){
		std::uniform_int_distribution<int> value(-8, 7);
		for(int& item : data) item = value(random);
	}else if(distribution == "equal"){
		std::fill(data.begin(), data.end(), -7);
	}else if(distribution == "periodic"){
		for(std::size_t i = 0; i < size; ++i) data[i] = static_cast<int>(i % 31) - 15;
	}else if(distribution == "organ_pipe"){
		for(std::size_t i = 0; i < size; ++i) data[i] = static_cast<int>(std::min(i, size - 1 - i));
	}else if(distribution != "sorted"){
		throw std::invalid_argument("unknown distribution: " + distribution);
	}
	return data;
}
}
