#pragma once

#include <vector>

#include <leo/optimization/solvers/cse/common_subexpression_scorer.h>
#include <leo/optimization/solvers/cse/context.h>
#include <leo/optimization/solvers/cse/subexpression.h>

namespace leo::cse {

class IntersectionsScorer : public CommonSubexpressionScorer {
    double alpha;
    double beta;
public:
    IntersectionsScorer(double alpha, double beta);

    void setAlpha(double alpha);
    void setBeta(double beta);

    void score(const std::vector<Subexpression>& subexpressions, const Context& context, std::vector<double>& scores) const override;
private:
    bool isIntersects(const Subexpression& s1, const Subexpression& s2) const;
};

} // namespace leo::cse
