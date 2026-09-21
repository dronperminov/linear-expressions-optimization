#pragma once

#include <vector>

#include <leo/optimization/solvers/cse/context.h>
#include <leo/optimization/solvers/cse/subexpression.h>

namespace leo::cse {

class CommonSubexpressionScorer {
public:
    virtual void score(const std::vector<Subexpression>& subexpressions, const Context& context, std::vector<double>& scores) const = 0;

    virtual ~CommonSubexpressionScorer() = default;
};

} // namespace leo::cse
