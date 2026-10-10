#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/vector.h>
#include <leo/optimization/selection/score_selector.h>
#include <leo/optimization/solvers/solver.h>
#include <leo/optimization/solvers/vector_covering/candidate.h>
#include <leo/optimization/solvers/vector_covering/parameters.h>
#include <leo/optimization/solvers/vector_covering/vector_covering_scorer.h>

namespace leo::vector_covering {

class VectorCoveringSolver : public Solver {
    Parameters parameters;
    std::shared_ptr<const VectorCoveringScorer> scorer;
    std::shared_ptr<const ScoreSelector> selector;
    std::mt19937 generator;
    std::chrono::steady_clock::time_point deadline;

    std::unordered_set<Vector> targets;

    std::unordered_set<Vector> uncovered;
    std::unordered_map<Vector, size_t> pool;
    std::vector<Vector> vectors;
    std::vector<Substitution> steps;
    std::vector<double> scores;
    std::vector<Candidate> candidates;
    std::unordered_set<Vector> unique;
public:
    VectorCoveringSolver(const std::vector<std::vector<int>>& expressions, const Parameters& parameters, std::shared_ptr<const VectorCoveringScorer> scorer, std::shared_ptr<const ScoreSelector> selector, uint32_t seed);

    void setParameters(const Parameters& parameters);
    void setScorer(std::shared_ptr<const VectorCoveringScorer> scorer);
    void setSelector(std::shared_ptr<const ScoreSelector> selector);

    bool canStartFromSubstitutions() const override;

    std::optional<size_t> solve() override;
    std::optional<size_t> solve(const std::vector<Substitution>& substitutions) override;
    Solution getSolution() const override;
private:
    void initializeTargets();
    void initialize();
    void initializeDeadline();
    void initializePartial(const std::vector<Substitution>& substitutions, double probability);

    void initializeCandidates();
    void updateCandidates();
    void addCandidate(size_t i, size_t j, int sign);
    void useCandidate(const Candidate& candidate);

    void addTargetPairs();

    std::optional<size_t> build();
    size_t buildNaive();

    void removeUnused();

    bool isBounded() const;
    bool isTimeout() const;
};

} // namespace leo::vector_covering
