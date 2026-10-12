#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <random>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/optimization/selection/score_selector.h>
#include <leo/optimization/solvers/cse/common_subexpression_scorer.h>
#include <leo/optimization/solvers/cse/parameters.h>
#include <leo/optimization/solvers/cse/subexpression.h>
#include <leo/optimization/solvers/solver.h>

namespace leo::cse {

class CommonSubexpressionSolver : public Solver {
    Parameters parameters;
    std::shared_ptr<const CommonSubexpressionScorer> scorer;
    std::shared_ptr<const ScoreSelector> selector;
    std::mt19937 generator;
    std::chrono::steady_clock::time_point deadline;

    std::vector<std::vector<int>> matrix;
    std::vector<Substitution> substitutions;
    std::vector<double> scores;
    size_t variables;
public:
    CommonSubexpressionSolver(const std::vector<std::vector<int>>& expressions, const Parameters& parameters, std::shared_ptr<const CommonSubexpressionScorer> scorer, std::shared_ptr<const ScoreSelector> selector, uint32_t seed);

    void setParameters(const Parameters& parameters);
    void setScorer(std::shared_ptr<const CommonSubexpressionScorer> scorer);
    void setSelector(std::shared_ptr<const ScoreSelector> selector);

    std::optional<size_t> solve() override;
    Solution getSolution() const override;
private:
    void initialize();
    void initializeDeadline();

    std::vector<Subexpression> getSubexpressions() const;
    void eliminate(const Subexpression& subexpression);
    size_t getAdditions() const;

    bool isTimeout() const;
};

} // namespace leo::cse
