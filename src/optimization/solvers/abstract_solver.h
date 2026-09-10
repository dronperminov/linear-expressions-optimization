#pragma once

#include <iostream>
#include <vector>

#include "../../entities/solution.h"

class AbstractSolver {
protected:
    size_t dimension;
    std::vector<std::vector<int>> expressions;
public:
    AbstractSolver(const std::vector<std::vector<int>>& expressions);

    virtual size_t solve() = 0;
    virtual Solution getSolution() const = 0;

    virtual ~AbstractSolver() = default;
};
