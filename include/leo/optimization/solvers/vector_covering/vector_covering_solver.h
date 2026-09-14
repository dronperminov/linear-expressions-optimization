#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/vector.h>
#include <leo/optimization/selection/score_selector.h>
#include <leo/optimization/solvers/solver.h>
#include <leo/optimization/solvers/vector_covering/candidate.h>
#include <leo/optimization/solvers/vector_covering/context.h>
#include <leo/optimization/solvers/vector_covering/vector_covering_parameters.h>
#include <leo/optimization/solvers/vector_covering/vector_covering_scorer.h>

namespace leo::vector_covering {

class VectorCoveringSolver : public Solver {
    VectorCoveringParameters parameters;
    const VectorCoveringScorer* scorer;
    const ScoreSelector* selector;

    size_t naiveComplexity;
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
    void setSelector(const ScoreSelector& selector);

    size_t solve() override;
    Solution getSolution() const override;
private:
    void initialize();

    std::vector<Candidate> getCandidates() const;
    void addCandidate(const Candidate& candidate);

    void fallbackToNaive();
};

} // namespace leo::vector_covering
