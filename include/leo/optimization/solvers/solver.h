#pragma once

#include <cstddef>
#include <vector>

#include <leo/entities/solution.h>

namespace leo {

class Solver {
protected:
    size_t dimension;
    const std::vector<std::vector<int>>& expressions;
    size_t bound;
    bool solved;
public:
    Solver(const std::vector<std::vector<int>>& expressions);

    const std::vector<std::vector<int>>& getExpressions() const;

    void setBound(size_t bound);

    virtual size_t solve() = 0;
    virtual Solution getSolution() const = 0;

    virtual ~Solver() = default;
};

} // namespace leo
