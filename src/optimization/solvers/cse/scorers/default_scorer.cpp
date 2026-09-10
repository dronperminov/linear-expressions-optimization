#include "default_scorer.h"

void CommonSubexpressionDefaultScorer::score(const std::vector<Subexpression>& subexpressions, const CommonSubexpressionContext& context, std::vector<double>& scores) const {
    scores.resize(subexpressions.size());

    for (size_t i = 0; i < subexpressions.size(); i++)
        scores[i] = subexpressions[i].rows.size() - 1;
}
