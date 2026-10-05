#pragma once

#include <vector>

#include <leo/entities/solution.h>

namespace leo {

class SolutionValidator {
public:
    bool validate(const std::vector<std::vector<int>>& expressions, const Solution& solution) const;
};

} // namespace leo
