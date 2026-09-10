#pragma once

#include <vector>

#include "common_subexpression_context.h"
#include "subexpression.h"


class CommonSubexpressionScorer {
public:
    virtual void score(const std::vector<Subexpression>& subexpressions, const CommonSubexpressionContext& context, std::vector<double>& scores) const = 0;

    virtual ~CommonSubexpressionScorer() = default;
};
