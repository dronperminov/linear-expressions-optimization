#include <leo/entities/expressions_system.h>

#include <algorithm>
#include <cstdlib>

#include <leo/utils/solution_validator.h>

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

bool ExpressionsSystem::isVertical() const {
    return count > dimension;
}

bool ExpressionsSystem::isHorizontal() const {
    return count < dimension;
}

bool ExpressionsSystem::isSquare() const {
    return count == dimension;
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

Solution ExpressionsSystem::getNaiveSolution() const {
    std::vector<std::vector<Term>> naiveExpressions(count);

    for (size_t i = 0; i < expressions.size(); i++)
        for (size_t j = 0; j < expressions[i].size(); j++)
            if (expressions[i][j])
                naiveExpressions[i].push_back({j, expressions[i][j]});

    return {dimension, {}, naiveExpressions};
}

bool ExpressionsSystem::validateVariablesCount() const {
    for (size_t i = 1; i < count; i++)
        if (expressions[i].size() != dimension)
            return false;

    return true;
}

bool ExpressionsSystem::validateSolution(const Solution& solution) const {
    SolutionValidator validator;
    bool valid = validator.validate(expressions, solution);
    return valid;
}

void ExpressionsSystem::describe(std::ostream& os) const {
    os << "- variables: " << getVariablesCount() << std::endl;
    os << "- expressions: " << getExpressionsCount() << std::endl;
    os << "- max abs value: " << getMaxAbsValue() << std::endl;
    os << "- naive additions: " << getNaiveAdditions() << std::endl;
    os << "- lower bound: " << getAdditionsLowerBound() << std::endl;
}

size_t ExpressionsSystem::getAdditionsLowerBound(const std::vector<std::vector<int>>& matrix) const {
    if (matrix.empty())
        return 0;

    std::unordered_set<Vector> targets = getUniqueTargets(matrix);
    size_t maxSupport = getMaxSupport(targets);

    size_t bound = targets.size();
    if (!coverTargets(targets, matrix[0].size()))
        bound++;

    if (maxSupport > 0)
        bound = std::max(bound, maxSupport - 1);

    return bound;
}

std::unordered_set<Vector> ExpressionsSystem::getUniqueTargets(const std::vector<std::vector<int>>& matrix) const {
    std::unordered_set<Vector> unique;

    for (const std::vector<int>& row : matrix) {
        Vector vector(row);

        if (vector.getSupport() > 1) {
            vector.canonize();
            unique.insert(vector);
        }
    }

    return unique;
}

size_t ExpressionsSystem::getMaxSupport(const std::unordered_set<Vector>& vectors) const {
    size_t maxSupport = 0;
    for (const Vector& vector : vectors)
        maxSupport = std::max(maxSupport, vector.getSupport());

    return maxSupport;
}

bool ExpressionsSystem::coverTargets(std::unordered_set<Vector>& targets, size_t variables) const {
    std::vector<Vector> vectors;
    for (size_t i = 0; i < variables; i++)
        vectors.emplace_back(variables, i);

    for (size_t i = 0; i < vectors.size() && !targets.empty(); i++) {
        for (size_t j = 0; j <= i; j++) {
            for (int sign : {1, -1}) {
                if (i == j && sign == -1)
                    continue;

                Vector vector = vectors[i].addScaled(vectors[j], sign);

                auto it = targets.find(vector.getCanonized());
                if (it == targets.end())
                    continue;

                vectors.push_back(*it);
                targets.erase(it);
            }
        }
    }

    return targets.empty();
}

} // namespace leo
