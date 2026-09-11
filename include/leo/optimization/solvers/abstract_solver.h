#pragma once

#include <iostream>
#include <vector>

#include <leo/entities/solution.h>

namespace leo {

class AbstractSolver {
protected:
    size_t dimension;
    std::vector<std::vector<int>> expressions;
public:
    AbstractSolver(const std::vector<std::vector<int>>& expressions);

    const std::vector<std::vector<int>>& getExpressions() const;

    virtual size_t solve() = 0;
    virtual Solution getSolution() const = 0;

    virtual ~AbstractSolver() = default;
};

} // namespace leo
