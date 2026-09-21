#pragma once

#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/term.h>

namespace leo::utils {

class SolutionValidator {
public:
    bool validate(const std::vector<std::vector<int>>& expressions, const Solution& solution) const;
};

} // namespace leo::utils
