#pragma once

#include <unordered_map>
#include "../vector_covering_scorer.h"

class VectorCoveringDefaultScorer : public VectorCoveringScorer {
    double coverWeight;
    double oneStepWeight;
    double hammingWeight;
    double matchesWeight;
public:
    VectorCoveringDefaultScorer();
    VectorCoveringDefaultScorer(double coverWeight, double oneStepWeight, double hammingWeight, double matchesWeight);

    void score(const std::vector<VectorCoveringCandidate>& candidates, const VectorCoveringContext& context, std::vector<double>& scores) const;
private:
    void addOneStepScores(const std::vector<VectorCoveringCandidate>& candidates, const VectorCoveringContext& context, std::vector<double>& scores) const;
};
