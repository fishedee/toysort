#pragma once
#include <string>
#include <vector>

namespace fastsort4Detail {
// False leaves output unusable; callers must fall back from the original input.
bool metalSort(const std::vector<int>& input, std::vector<int>& output, std::string& error);
}
