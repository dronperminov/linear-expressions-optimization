#pragma once


#include <vector>
#include "../entities/solution.h"
#include "../entities/substitution.h"
#include "../entities/term.h"

class SolutionValidator {
public:
    bool validate(const std::vector<std::vector<int>>& expressions, const Solution& solution) const;
};
