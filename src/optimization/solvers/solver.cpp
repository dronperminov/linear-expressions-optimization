#include <leo/optimization/solvers/solver.h>

namespace leo {

Solver::Solver(const std::vector<std::vector<int>>& expressions) {
    this->expressions = expressions;
    this->dimension = expressions.empty() ? 0 : expressions[0].size();
}

const std::vector<std::vector<int>>& Solver::getExpressions() const {
    return expressions;
}

} // namespace leo
