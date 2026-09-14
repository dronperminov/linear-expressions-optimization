#include <leo/optimization/solvers/cse/scorers/default_scorer.h>

namespace leo::cse {

void DefaultScorer::score(const std::vector<Subexpression>& subexpressions, const Context& context, std::vector<double>& scores) const {
    scores.resize(subexpressions.size());

    for (size_t i = 0; i < subexpressions.size(); i++)
        scores[i] = subexpressions[i].rows.size() - 1;
}

} // namespace leo::cse
