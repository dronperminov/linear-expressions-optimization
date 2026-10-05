#pragma once

#include <vector>

#include <leo/optimization/solvers/vector_covering/candidate.h>
#include <leo/optimization/solvers/vector_covering/context.h>
#include <leo/optimization/solvers/vector_covering/vector_covering_scorer.h>

namespace leo::vector_covering {

class DefaultScorer : public VectorCoveringScorer {
    double coverWeight;
    double oneStepWeight;
    double hammingWeight;
    double matchesWeight;
    double distanceWeight;
    double savingsWeight;
public:
    DefaultScorer();
    DefaultScorer(double coverWeight, double oneStepWeight, double hammingWeight, double matchesWeight, double distanceWeight, double savingsWeight);

    void score(const std::vector<Candidate>& candidates, const Context& context, std::vector<double>& scores) const override;
private:
    void addOneStepScores(const std::vector<Candidate>& candidates, const Context& context, std::vector<double>& scores) const;
};

} // namespace leo::vector_covering
