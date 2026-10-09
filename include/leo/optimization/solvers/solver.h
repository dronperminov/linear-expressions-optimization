#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>

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

    virtual bool canStartFromSubstitutions() const;

    virtual std::optional<size_t> solve() = 0;
    virtual std::optional<size_t> solve(const std::vector<Substitution>& substitutions);
    virtual Solution getSolution() const = 0;

    virtual ~Solver() = default;
};

} // namespace leo
