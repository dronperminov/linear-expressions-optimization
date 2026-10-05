#include <leo/utils/solution_substitution_inliner.h>

#include <algorithm>

namespace leo {

Solution SolutionSubstitutionInliner::optimize(const Solution& solution) const {
    size_t dimension = solution.dimension;

    std::vector<Usage> usage = getUsage(solution);
    std::vector<bool> inlinable = getInlinable(usage);
    std::vector<size_t> indices(solution.substitutions.size());
    std::vector<std::vector<Term>> substitutionExpressions = getSubstitutionExpressions(solution);

    Solution optimized = {dimension, {}, solution.expressions};

    for (size_t index = 0; index < solution.substitutions.size(); index++) {
        if (!inlinable[index]) {
            indices[index] = dimension + optimized.substitutions.size();

            const Substitution& s = solution.substitutions[index];
            size_t i = remap(s.i, dimension, indices);
            size_t j = remap(s.j, dimension, indices);
            optimized.substitutions.push_back({i, j, s.ai, s.aj});
            continue;
        }

        if (usage[index].substitutionsCount == 1) {
            inlineTerm(substitutionExpressions[usage[index].row], usage[index].column, substitutionExpressions[index]);
        }
        else {
            inlineTerm(optimized.expressions[usage[index].row], usage[index].column, substitutionExpressions[index]);
        }
    }

    for (std::vector<Term>& expression : optimized.expressions)
        normalize(expression, indices, dimension);

    return optimized;
}

std::vector<SolutionSubstitutionInliner::Usage> SolutionSubstitutionInliner::getUsage(const Solution& solution) const {
    std::vector<Usage> usage(solution.substitutions.size());

    for (size_t index = 0; index < solution.substitutions.size(); index++) {
        updateUsage(usage, solution.substitutions[index].i, solution.dimension, index, 0, false);
        updateUsage(usage, solution.substitutions[index].j, solution.dimension, index, 1, false);
    }

    for (size_t i = 0; i < solution.expressions.size(); i++)
        for (size_t j = 0; j < solution.expressions[i].size(); j++)
            updateUsage(usage, solution.expressions[i][j].index, solution.dimension, i, j, true);

    return usage;
}

std::vector<bool> SolutionSubstitutionInliner::getInlinable(const std::vector<Usage>& usage) const {
    std::vector<bool> inlinable(usage.size(), false);

    for (size_t index = usage.size(); index-- > 0;) {
        if (usage[index].substitutionsCount == 0 && usage[index].expressionsCount == 1) {
            inlinable[index] = true;
        }
        else if (usage[index].substitutionsCount == 1 && usage[index].expressionsCount == 0) {
            inlinable[index] = inlinable[usage[index].row];
        }
    }

    return inlinable;
}

std::vector<std::vector<Term>> SolutionSubstitutionInliner::getSubstitutionExpressions(const Solution& solution) const {
    std::vector<std::vector<Term>> expressions;
    expressions.reserve(solution.substitutions.size());

    for (const Substitution& s : solution.substitutions)
        expressions.push_back({{s.i, s.ai}, {s.j, s.aj}});

    return expressions;
}

void SolutionSubstitutionInliner::updateUsage(std::vector<Usage>& usage, size_t index, size_t dimension, size_t row, size_t column, bool byExpressions) const {
    if (index < dimension)
        return;

    if (byExpressions) {
        usage[index - dimension].expressionsCount++;
    }
    else {
        usage[index - dimension].substitutionsCount++;
    }

    usage[index - dimension].row = row;
    usage[index - dimension].column = column;
}

void SolutionSubstitutionInliner::inlineTerm(std::vector<Term>& expression, size_t column, const std::vector<Term>& replacement) const {
    int scale = expression[column].value;

    for (const Term& term : replacement)
        if (term.value != 0)
            expression.push_back({term.index, term.value * scale});

    expression[column].value = 0;
}

void SolutionSubstitutionInliner::normalize(std::vector<Term>& expression, const std::vector<size_t>& indices, size_t dimension) const {
    std::sort(expression.begin(), expression.end(), [](const Term& a, const Term& b) { return a.index < b.index; });

    size_t size = 0;

    for (const Term& term : expression) {
        if (term.value == 0)
            continue;

        size_t index = remap(term.index, dimension, indices);

        if (size == 0 || expression[size - 1].index != index) {
            expression[size++] = {index, term.value};
            continue;
        }

        expression[size - 1].value += term.value;

        if (expression[size - 1].value == 0)
            size--;
    }

    expression.erase(expression.begin() + size, expression.end());
}

size_t SolutionSubstitutionInliner::remap(size_t index, size_t dimension, const std::vector<size_t>& indices) const {
    return index < dimension ? index : indices[index - dimension];
}

} // namespace leo
