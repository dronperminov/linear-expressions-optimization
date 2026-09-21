#pragma once

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/optimization/selection/score_selector.h>
#include <leo/optimization/solvers/cse/common_subexpression_scorer.h>
#include <leo/optimization/solvers/cse/context.h>
#include <leo/optimization/solvers/cse/subexpression.h>
#include <leo/optimization/solvers/solver.h>

namespace leo::cse {

class CommonSubexpressionSolver : public Solver {
    const CommonSubexpressionScorer* scorer;
    const ScoreSelector* selector;

    std::vector<std::vector<int>> matrix;
    std::vector<Substitution> substitutions;
    std::vector<double> scores;
    size_t variables;
public:
    CommonSubexpressionSolver(const std::vector<std::vector<int>>& expressions, const CommonSubexpressionScorer& scorer, const ScoreSelector& selector);

    void setScorer(const CommonSubexpressionScorer& scorer);
    void setSelector(const ScoreSelector& selector);

    size_t solve() override;
    Solution getSolution() const override;
private:
    void initialize();

    std::vector<Subexpression> getSubexpressions() const;
    void eliminate(const Subexpression& subexpression);
    size_t getAdditions() const;
};

} // namespace leo::cse
