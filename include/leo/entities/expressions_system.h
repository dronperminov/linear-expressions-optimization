#pragma once

#include <cstddef>
#include <iostream>
#include <vector>

#include <leo/entities/solution.h>

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

    bool isVertical() const;
    bool isHorizontal() const;
    bool isSquare() const;

    const std::vector<std::vector<int>>& getExpressions() const;
    std::vector<std::vector<int>> getTransposedExpressions() const;

    Solution getNaiveSolution() const;

    bool validateVariablesCount() const;
    bool validateSolution(const Solution& solution) const;

    void describe(std::ostream& os) const;
private:
    size_t getAdditionsLowerBound(const std::vector<std::vector<int>>& expressions) const;
};

} // namespace leo
