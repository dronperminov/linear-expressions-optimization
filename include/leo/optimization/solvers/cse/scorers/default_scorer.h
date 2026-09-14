#pragma once

#include <leo/optimization/solvers/cse/common_subexpression_scorer.h>

namespace leo::cse {

class DefaultScorer : public CommonSubexpressionScorer {
public:
    void score(const std::vector<Subexpression>& subexpressions, const Context& context, std::vector<double>& scores) const override;
};

} // namespace leo::cse
