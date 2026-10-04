#include "fastsort.h"
#include "fastsort2.h"
#include "fastsort3.h"
#include "stdsort.h"
#include "stdstablesort.h"
#include "sort_data.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>

int main(int argc, char** argv){
	try{
		unsigned seed = 20261004;
		std::size_t maxSize = 10000000;
		bool tune = false;
		for(int i = 1; i < argc; ++i){
			std::string option = argv[i];
			if(option == "--tune"){
				tune = true;
			}else if((option == "--seed" || option == "--max-size") && i + 1 < argc){
				std::string text = argv[++i];
				if(text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
					throw std::invalid_argument("expected a nonnegative integer");
				unsigned long long value = std::stoull(text);
				if(option == "--seed"){
					if(value > std::numeric_limits<unsigned>::max()) throw std::invalid_argument("seed too large");
					seed = static_cast<unsigned>(value);
				}else{
					if(value < 1000 || value > 10000000) throw std::invalid_argument("max-size must be 1000..10000000");
					maxSize = static_cast<std::size_t>(value);
				}
			}else{
				throw std::invalid_argument("usage: sort_benchmark [--tune] [--seed N] [--max-size N]");
			}
		}
		FastSort fast;
		FastSort2 fast2;
		FastSort3 fast3;
		StdSort standard;
		StdStableSort stable;
		Sort* algorithms[] = {&fast, &fast2, &fast3, &standard, &stable};
		const std::size_t algorithmCount = sizeof(algorithms) / sizeof(algorithms[0]);
		std::size_t standardIndex = 0;
		while(algorithms[standardIndex] != &standard) ++standardIndex;
		std::cout << "# seed=" << seed << ", warmups=1, repetitions=5, clock=steady_clock, includes Run copy\n";
		std::cout << "# FastSort3 backend=" << FastSort3::GetBackendName() << '\n';
		std::cout << "distribution,size,algorithm,median_ms,ratio_to_std\n";
		std::size_t group = 0;
		for(std::size_t size : {1000u, 10000u, 100000u, 1000000u, 10000000u}){
			if(size > maxSize || (tune && (size < 100000 || size > 1000000))) continue;
			for(const std::string& distribution : sortData::distributions()){
				std::vector<int> input = sortData::make(size, distribution, seed);
				std::vector<int> expected = input;
				std::sort(expected.begin(), expected.end());
				std::vector<double> samples[algorithmCount];
				for(int round = 0; round < 6; ++round){
					for(std::size_t slot = 0; slot < algorithmCount; ++slot){
						std::size_t index = (group + round + slot) % algorithmCount;
						auto begin = std::chrono::steady_clock::now();
						std::vector<int> result = algorithms[index]->Run(input);
						auto end = std::chrono::steady_clock::now();
						if(result != expected) throw std::runtime_error(algorithms[index]->GetName() + " failed " + distribution);
						if(round != 0) samples[index].push_back(std::chrono::duration<double, std::milli>(end - begin).count());
					}
				}
				double medians[algorithmCount];
				for(std::size_t i = 0; i < algorithmCount; ++i){
					std::sort(samples[i].begin(), samples[i].end());
					medians[i] = samples[i][2];
				}
				for(std::size_t i = 0; i < algorithmCount; ++i){
					std::cout << distribution << ',' << size << ',' << algorithms[i]->GetName() << ','
						<< std::fixed << std::setprecision(6) << medians[i] << ',' << medians[i] / medians[standardIndex] << '\n';
				}
				std::cout.flush();
				++group;
			}
		}
		return 0;
	}catch(const std::exception& error){
		std::cerr << "FAIL: " << error.what() << '\n';
		return 1;
	}
}
