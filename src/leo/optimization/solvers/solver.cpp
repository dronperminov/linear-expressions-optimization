#include <leo/optimization/solvers/solver.h>

namespace leo {

Solver::Solver(const std::vector<std::vector<int>>& expressions) : expressions(expressions) {
    this->dimension = expressions.empty() ? 0 : expressions[0].size();
    this->solved = false;
    this->bound = -1;
}

const std::vector<std::vector<int>>& Solver::getExpressions() const {
    return expressions;
}

void Solver::setBound(size_t bound) {
    this->bound = bound;
}

bool Solver::canStartFromSubstitutions() const {
    return false;
}

std::optional<size_t> Solver::solve(const std::vector<Substitution>& substitutions) {
    return std::nullopt;
}

} // namespace leo
