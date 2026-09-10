#pragma once

#include "../common_subexpression_scorer.h"

class CommonSubexpressionDefaultScorer : public CommonSubexpressionScorer {
public:
    void score(const std::vector<Subexpression>& subexpressions, const CommonSubexpressionContext& context, std::vector<double>& scores) const override;
};
