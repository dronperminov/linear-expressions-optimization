#include "common_subexpression_solver.h"

CommonSubexpressionSolver::CommonSubexpressionSolver(const std::vector<std::vector<int>>& expressions, const CommonSubexpressionScorer& scorer, const ScoreSelector& selector) : AbstractSolver(expressions) {
    setScorer(scorer);
    setSelector(selector);
}

void CommonSubexpressionSolver::setScorer(const CommonSubexpressionScorer &scorer) {
    this->scorer = &scorer;
}

void CommonSubexpressionSolver::setSelector(const ScoreSelector &selector) {
    this->selector = &selector;
}

size_t CommonSubexpressionSolver::solve() {
    initialize();

    while (1) {
        std::vector<Subexpression> subexpressions = getSubexpressions();
        if (subexpressions.empty())
            break;

        scorer->score(subexpressions, {matrix}, scores);
        Subexpression subexpression = subexpressions[selector->selectIndex(scores)];
        eliminate(subexpression);
    }

    return getAdditions();
}

Solution CommonSubexpressionSolver::getSolution() const {
    Solution solution;
    solution.dimension = dimension;
    solution.substitutions = substitutions;
    solution.expressions.resize(expressions.size());

    for (size_t i = 0; i < expressions.size(); i++)
        for (size_t j = 0; j < variables; j++)
            if (matrix[i][j])
                solution.expressions[i].push_back({j, matrix[i][j]});

    return solution;
}

void CommonSubexpressionSolver::initialize() {
    matrix = expressions;
    substitutions.clear();
    variables = dimension;
}

std::vector<Subexpression> CommonSubexpressionSolver::getSubexpressions() const {
    std::vector<Subexpression> subexpressions;

    for (size_t i = 0; i < variables; i++) {
        for (size_t j = i + 1; j < variables; j++) {
            std::vector<size_t> positiveIndices;
            std::vector<size_t> negativeIndices;

            for (size_t k = 0; k < matrix.size(); k++) {
                if (matrix[k][i] == 0 || matrix[k][j] == 0)
                    continue;

                if (matrix[k][i] == matrix[k][j]) {
                    positiveIndices.push_back(k);
                }
                else if (matrix[k][i] == -matrix[k][j]) {
                    negativeIndices.push_back(k);
                }
            }

            if (positiveIndices.size() > 1)
                subexpressions.push_back({i, j, 1, positiveIndices});

            if (negativeIndices.size() > 1)
                subexpressions.push_back({i, j, -1, negativeIndices});
        }
    }

    return subexpressions;
}

void CommonSubexpressionSolver::eliminate(const Subexpression& subexpression) {
    for (size_t i = 0; i < matrix.size(); i++)
        matrix[i].push_back(0);

    for (size_t k : subexpression.rows) {
        matrix[k][variables] = matrix[k][subexpression.i];
        matrix[k][subexpression.i] = 0;
        matrix[k][subexpression.j] = 0;
    }

    substitutions.push_back({subexpression.i, subexpression.j, 1, subexpression.sign});
    variables++;
}

size_t CommonSubexpressionSolver::getAdditions() const {
    size_t additions = substitutions.size();

    for (size_t i = 0; i < matrix.size(); i++) {
        for (size_t j = 0; j < variables; j++)
            if (matrix[i][j])
                additions++;

        additions--;
    }

    return additions;
}
