#pragma once

#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "../abstract_solver.h"
#include "../../../entities/substitution.h"
#include "../../../entities/solution.h"
#include "../../selection/score_selector.h"
#include "subexpression.h"
#include "common_subexpression_context.h"
#include "common_subexpression_scorer.h"


class CommonSubexpressionSolver : public AbstractSolver {
    const CommonSubexpressionScorer* scorer;
    const ScoreSelector* selector;

    std::vector<std::vector<int>> matrix;
    std::vector<Substitution> substitutions;
    std::vector<double> scores;
    size_t variables;
public:
    CommonSubexpressionSolver(const std::vector<std::vector<int>>& expressions, const CommonSubexpressionScorer& scorer, const ScoreSelector& selector);

    void setScorer(const CommonSubexpressionScorer &scorer);
    void setSelector(const ScoreSelector &selector);

    size_t solve() override;
    Solution getSolution() const override;
private:
    void initialize();

    std::vector<Subexpression> getSubexpressions() const;
    void eliminate(const Subexpression& subexpression);
    size_t getAdditions() const;
};
