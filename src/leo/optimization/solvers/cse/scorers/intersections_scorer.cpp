#include <leo/optimization/solvers/cse/scorers/intersections_scorer.h>

namespace leo::cse {

IntersectionsScorer::IntersectionsScorer(double alpha, double beta) {
    setAlpha(alpha);
    setBeta(beta);
}

void IntersectionsScorer::setAlpha(double alpha) {
    this->alpha = alpha;
}

void IntersectionsScorer::setBeta(double beta) {
    this->beta = beta;
}

void IntersectionsScorer::score(const std::vector<Subexpression>& subexpressions, const Context& context, std::vector<double>& scores) const {
    scores.resize(subexpressions.size());

    for (size_t i = 0; i < subexpressions.size(); i++) {
        double score = 0;

        for (size_t j = 0; j < subexpressions.size(); j++) {
            if (i == j)
                continue;

            if (isIntersects(subexpressions[i], subexpressions[j]))
                score += beta * (subexpressions[j].rows.size() - 1);
            else
                score += (1 - beta) * (subexpressions[j].rows.size() - 1);
        }

        scores[i] = subexpressions[i].rows.size() - 1 + alpha * score;
    }
}

bool IntersectionsScorer::isIntersects(const Subexpression& s1, const Subexpression& s2) const {
    return s1.i == s2.i || s1.i == s2.j || s1.j == s2.i || s1.j == s2.j;
}

} // namespace leo::cse
