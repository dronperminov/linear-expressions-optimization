#pragma once

#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "../abstract_solver.h"
#include "../../../entities/substitution.h"
#include "../../../entities/solution.h"
#include "../../../entities/vector.h"
#include "../../selection/score_selector.h"
#include "vector_covering_parameters.h"
#include "vector_covering_context.h"
#include "vector_covering_candidate.h"
#include "vector_covering_scorer.h"

class VectorCoveringSolver : public AbstractSolver {
    VectorCoveringParameters parameters;
    const VectorCoveringScorer* scorer;
    const ScoreSelector* selector;

    std::unordered_set<Vector> targets;
    std::unordered_set<Vector> uncovered;
    std::unordered_set<Vector> pool;
    std::vector<Vector> vectors;
    std::vector<Substitution> steps;
    std::vector<double> scores;
public:
    VectorCoveringSolver(const std::vector<std::vector<int>>& expressions, const VectorCoveringParameters& parameters, const VectorCoveringScorer& scorer, const ScoreSelector& selector);

    void setParameters(const VectorCoveringParameters& parameters);
    void setScorer(const VectorCoveringScorer& scorer);
    void setSelector(const ScoreSelector &selector);

    size_t solve() override;
    Solution getSolution() const override;
private:
    void initialize();

    std::vector<VectorCoveringCandidate> getCandidates() const;
    void addCandidate(const VectorCoveringCandidate& candidate);
};
