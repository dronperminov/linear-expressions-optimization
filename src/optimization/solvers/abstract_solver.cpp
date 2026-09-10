#include "abstract_solver.h"

AbstractSolver::AbstractSolver(const std::vector<std::vector<int>>& expressions) {
    this->expressions = expressions;
    this->dimension = expressions[0].size();
}

const std::vector<std::vector<int>>& AbstractSolver::getExpressions() const {
    return expressions;
}
