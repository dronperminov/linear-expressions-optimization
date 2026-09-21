#include <leo/entities/expressions_system.h>

namespace leo {

ExpressionsSystem::ExpressionsSystem(const std::vector<std::vector<int>>& expressions) : expressions(expressions) {
    dimension = expressions.empty() ? 0 : expressions[0].size();
    count = expressions.size();
}

size_t ExpressionsSystem::getVariablesCount() const {
    return dimension;
}

size_t ExpressionsSystem::getExpressionsCount() const {
    return count;
}

size_t ExpressionsSystem::getNaiveAdditions() const {
    size_t additions = 0;

    for (const std::vector<int>& expression : expressions) {
        size_t expressionAdditions = 0;

        for (int value : expression)
            expressionAdditions += value != 0;

        if (expressionAdditions)
            additions += expressionAdditions - 1;
    }

    return additions;
}

size_t ExpressionsSystem::getAdditionsLowerBound() const {
    if (count == 0 || dimension < 2)
        return 0;

    if (count >= dimension)
        return getAdditionsLowerBound(expressions);

    return getAdditionsLowerBound(getTransposedExpressions()) + dimension - count;
}

int ExpressionsSystem::getMaxAbsValue() const {
    int maxAbsValue = 0;

    for (const std::vector<int>& expression : expressions)
        for (int value : expression)
            maxAbsValue = std::max(maxAbsValue, std::abs(value));

    return maxAbsValue;
}

const std::vector<std::vector<int>>& ExpressionsSystem::getExpressions() const {
    return expressions;
}

std::vector<std::vector<int>> ExpressionsSystem::getTransposedExpressions() const {
    std::vector<std::vector<int>> transposed(dimension, std::vector<int>(count));

    for (size_t i = 0; i < count; i++)
        for (size_t j = 0; j < dimension; j++)
            transposed[j][i] = expressions[i][j];

    return transposed;
}

bool ExpressionsSystem::validateVariablesCount() const {
    for (size_t i = 1; i < count; i++)
        if (expressions[i].size() != dimension)
            return false;

    return true;
}

bool ExpressionsSystem::validateSolution(const Solution& solution) const {
    utils::SolutionValidator validator;
    bool valid = validator.validate(expressions, solution);
    return valid;
}

size_t ExpressionsSystem::getAdditionsLowerBound(const std::vector<std::vector<int>>& expressions) const {
    std::unordered_set<Vector> unique;

    for (const std::vector<int>& expression : expressions) {
        Vector vector(expression);
        vector.canonize();

        if (vector.getSupport() > 1)
            unique.insert(vector);
    }

    return unique.size();
}

} // namespace leo
