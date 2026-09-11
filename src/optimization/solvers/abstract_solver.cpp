#include <leo/optimization/solvers/abstract_solver.h>

namespace leo {

AbstractSolver::AbstractSolver(const std::vector<std::vector<int>>& expressions) {
    this->expressions = expressions;
    this->dimension = expressions.empty() ? 0 : expressions[0].size();
}

const std::vector<std::vector<int>>& AbstractSolver::getExpressions() const {
    return expressions;
}

} // namespace leo
