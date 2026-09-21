#pragma once

#include <vector>

#include <leo/entities/solution.h>

namespace leo {

class Solver {
protected:
    size_t dimension;
    std::vector<std::vector<int>> expressions;
    bool solved;
public:
    Solver(const std::vector<std::vector<int>>& expressions);

    const std::vector<std::vector<int>>& getExpressions() const;

    virtual size_t solve() = 0;
    virtual Solution getSolution() const = 0;

    virtual ~Solver() = default;
};

} // namespace leo
