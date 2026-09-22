#pragma once

#include <unordered_map>

#include <leo/optimization/solvers/vector_covering/vector_covering_scorer.h>

namespace leo::vector_covering {

class DefaultScorer : public VectorCoveringScorer {
    double coverWeight;
    double oneStepWeight;
    double hammingWeight;
    double matchesWeight;
    double distanceWeight;
public:
    DefaultScorer();
    DefaultScorer(double coverWeight, double oneStepWeight, double hammingWeight, double matchesWeight, double distanceWeight);

    void score(const std::vector<Candidate>& candidates, const Context& context, std::vector<double>& scores) const;
private:
    void addOneStepScores(const std::vector<Candidate>& candidates, const Context& context, std::vector<double>& scores) const;
};

} // namespace leo::vector_covering
