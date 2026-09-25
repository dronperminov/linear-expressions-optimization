#pragma once

#include <iostream>
#include <vector>
#include <unordered_set>

#include <leo/entities/solution.h>
#include <leo/entities/vector.h>
#include <leo/utils/solution_validator.h>

namespace leo {

class ExpressionsSystem {
    size_t dimension;
    size_t count;
    std::vector<std::vector<int>> expressions;
public:
    ExpressionsSystem(const std::vector<std::vector<int>>& expressions);

    size_t getVariablesCount() const;
    size_t getExpressionsCount() const;
    size_t getNaiveAdditions() const;
    size_t getAdditionsLowerBound() const;

    int getMaxAbsValue() const;

    const std::vector<std::vector<int>>& getExpressions() const;
    std::vector<std::vector<int>> getTransposedExpressions() const;

    bool validateVariablesCount() const;
    bool validateSolution(const Solution& solution) const;

    void describe(std::ostream& os) const;
private:
    size_t getAdditionsLowerBound(const std::vector<std::vector<int>>& expressions) const;
};

} // namespace leo
