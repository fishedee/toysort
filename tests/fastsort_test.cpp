#ifdef TEST_FASTSORT2
#include "fastsort2.h"
using TestedSort = FastSort2;
#else
#include "fastsort.h"
using TestedSort = FastSort;
#endif
#include "sort_data.h"
#include <iostream>
#include <stdexcept>

namespace {
std::size_t cases = 0;

void check(const std::vector<int>& input, const std::string& label){
	std::vector<int> original = input;
	std::vector<int> expected = input;
	std::sort(expected.begin(), expected.end());
	TestedSort sorter;
	std::vector<int> actual = sorter.Run(input);
	if(actual != expected || input != original){
		throw std::runtime_error(label + ": incorrect output or changed input, size=" + std::to_string(input.size()));
	}
	++cases;
}

void exhaustive(std::vector<int>& input, std::size_t index){
	if(index == input.size()){
		check(input, "exhaustive {-1,0,1}");
		return;
	}
	for(int value = -1; value <= 1; ++value){
		input[index] = value;
		exhaustive(input, index + 1);
	}
}
}

int main(){
	try{
		check({}, "empty");
		check({INT_MIN}, "singleton");
		check({1, 0}, "two reversed");
		check({INT_MAX, 0, INT_MIN, -1, INT_MAX, INT_MIN}, "extreme values");
		for(std::size_t size = 0; size <= 9; ++size){
			std::vector<int> input(size);
			exhaustive(input, 0);
		}
		// Includes every neighborhood used by the insertion/block parameter sweep.
		const std::size_t sizes[] = {0, 1, 2, 3, 15, 16, 17, 23, 24, 25, 31, 32, 33,
			63, 64, 65, 95, 96, 97, 127, 128, 129, 255, 256, 257, 511, 512, 513, 1024, 4096, 65536};
		for(std::size_t size : sizes){
			for(const std::string& distribution : sortData::distributions()){
				check(sortData::make(size, distribution, 20261004), distribution);
			}
		}
		std::mt19937 random(314159);
		std::uniform_int_distribution<int> value(INT_MIN, INT_MAX);
		for(int trial = 0; trial < 10000; ++trial){
			std::size_t size = random() % 4097;
			std::vector<int> input(size);
			for(int& item : input){
				item = trial % 3 == 0 ? static_cast<int>(random() % 5) - 2 : value(random);
			}
			check(input, "random seed=314159 trial=" + std::to_string(trial));
		}
		// Rare outliers can evade duplicate detection in the first pivot sample.
		for(int trial = 0; trial < 100; ++trial){
			std::vector<int> input(10000, 0);
			for(int i = 0; i < 20; ++i) input[random() % input.size()] = i % 2 ? INT_MIN : INT_MAX;
			check(input, "sparse outliers");
		}
#ifdef TEST_FASTSORT2
		// Exercise sampling/block edges and skewed partitions independently of seeds.
		for(std::size_t size : {30u, 32u, 34u, 62u, 64u, 66u, 126u, 128u, 130u,
			254u, 256u, 258u, 1023u, 1025u, 100000u}){
			std::vector<int> input(size);
			for(std::size_t i = 0; i < size; ++i)
				input[i] = i % 2 ? INT_MAX - static_cast<int>(i) : INT_MIN + static_cast<int>(i);
			check(input, "alternating extremes");
			for(std::size_t i = 0; i < size; ++i) input[i] = static_cast<int>(i % 127);
			check(input, "sawtooth 127");
			std::iota(input.begin(), input.end(), 0);
			for(std::size_t i = 0; i + 1 < size; i += 2) std::swap(input[i], input[i + 1]);
			check(input, "adjacent inversions");
			std::iota(input.begin(), input.end(), 0);
			std::rotate(input.begin(), input.begin() + size / 3, input.end());
			check(input, "rotated ascending");
		}
#endif
		std::cout << "PASS: " << cases << " cases (output, size, and input preservation)\n";
		return 0;
	}catch(const std::exception& error){
		std::cerr << "FAIL: " << error.what() << '\n';
		return 1;
	}
}
