#include <leo/utils/solution_sign_optimizer.h>

#include <cmath>
#include <unordered_set>

#include <leo/entities/term.h>

namespace leo {

SolutionSignOptimizer::SolutionSignOptimizer(double startTemp, double endTemp, size_t stepsPerVariable) : startTemp(startTemp), endTemp(endTemp), stepsPerVariable(stepsPerVariable) {

}

Solution SolutionSignOptimizer::optimize(const Solution& solution, std::mt19937& generator, size_t restarts) const {
    std::vector<SubstitutionUsage> usage = getUsage(solution);

    std::vector<int> bestS(solution.substitutions.size(), 1);
    int bestInversions = getInversions(solution, bestS);

    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    size_t steps = stepsPerVariable * solution.substitutions.size();
    std::vector<int> s(solution.substitutions.size(), 1);

    for (size_t restart = 0; restart < restarts && bestInversions > 0; restart++) {
        if (restart > 0) {
            for (size_t i = 0; i < s.size(); i++)
                s[i] = uniform(generator) < 0.5 ? 1 : -1;
        }

        int inversions = getInversions(solution, s);
        if (inversions < bestInversions) {
            bestInversions = inversions;
            bestS = s;
        }

        for (size_t step = 0; step < steps && bestInversions > 0; step++) {
            double T = startTemp * std::pow(endTemp / startTemp, double(step) / steps);
            size_t index = generator() % solution.substitutions.size();

            int before = getAffectedInversions(solution, index, usage, s);
            s[index] = -s[index];
            int delta = getAffectedInversions(solution, index, usage, s) - before;

            if (delta <= 0 || uniform(generator) < std::exp(-delta / T)) {
                inversions += delta;

                if (inversions < bestInversions) {
                    bestInversions = inversions;
                    bestS = s;
                }
            }
            else {
                s[index] = -s[index];
            }
        }
    }

    return apply(solution, bestS);
}

std::vector<SolutionSignOptimizer::SubstitutionUsage> SolutionSignOptimizer::getUsage(const Solution& solution) const {
    size_t total = solution.substitutions.size();
    std::vector<std::unordered_set<size_t>> substitutionsUsage(total);
    std::vector<std::unordered_set<size_t>> expressionsUsage(total);

    for (size_t index = 0; index < total; index++) {
        substitutionsUsage[index].insert(index);

        if (solution.substitutions[index].i >= solution.dimension)
            substitutionsUsage[solution.substitutions[index].i - solution.dimension].insert(index);

        if (solution.substitutions[index].j >= solution.dimension)
            substitutionsUsage[solution.substitutions[index].j - solution.dimension].insert(index);
    }

    for (size_t i = 0; i < solution.expressions.size(); i++)
        for (const Term& term : solution.expressions[i])
            if (term.index >= solution.dimension)
                expressionsUsage[term.index - solution.dimension].insert(i);

    std::vector<SubstitutionUsage> usage(total);
    for (size_t i = 0; i < total; i++) {
        usage[i].substitutions = std::vector<size_t>(substitutionsUsage[i].begin(), substitutionsUsage[i].end());
        usage[i].expressions = std::vector<size_t>(expressionsUsage[i].begin(), expressionsUsage[i].end());
    }

    return usage;
}

int SolutionSignOptimizer::getSign(size_t index, size_t dimension, const std::vector<int>& signs) const {
    return index < dimension ? 1 : signs[index - dimension];
}

int SolutionSignOptimizer::getInversions(const Solution& solution, const std::vector<int>& signs) const {
    int inversions = 0;

    for (size_t index = 0; index < solution.substitutions.size(); index++)
        inversions += isSubstitutionInverted(solution, index, signs);

    for (size_t index = 0; index < solution.expressions.size(); index++)
        inversions += isExpressionInverted(solution, index, signs);

    return inversions;
}

int SolutionSignOptimizer::getAffectedInversions(const Solution& solution, size_t index, const std::vector<SubstitutionUsage>& usage, const std::vector<int>& signs) const {
    int inversions = 0;

    for (size_t i : usage[index].substitutions)
        inversions += isSubstitutionInverted(solution, i, signs);

    for (size_t i : usage[index].expressions)
        inversions += isExpressionInverted(solution, i, signs);

    return inversions;
}

bool SolutionSignOptimizer::isSubstitutionInverted(const Solution& solution, size_t index, const std::vector<int>& signs) const {
    Substitution substitution = solution.substitutions[index];

    int ai = substitution.ai * signs[index] * getSign(substitution.i, solution.dimension, signs);
    int aj = substitution.aj * signs[index] * getSign(substitution.j, solution.dimension, signs);

    return (ai < 0) && (aj < 0);
}

bool SolutionSignOptimizer::isExpressionInverted(const Solution& solution, size_t index, const std::vector<int>& signs) const {
    if (solution.expressions[index].empty())
        return false;

    for (const Term& term : solution.expressions[index]) {
        int value = term.value * getSign(term.index, solution.dimension, signs);
        if (value > 0)
            return false;
    }

    return true;
}

Solution SolutionSignOptimizer::apply(const Solution& solution, const std::vector<int> &signs) const {
    Solution result = solution;

    for (size_t index = 0; index < result.substitutions.size(); index++) {
        Substitution& substitution = result.substitutions[index];

        substitution.ai *= signs[index] * getSign(substitution.i, result.dimension, signs);
        substitution.aj *= signs[index] * getSign(substitution.j, result.dimension, signs);

        if ((substitution.ai < 0) && (substitution.aj > 0)) {
            std::swap(substitution.i, substitution.j);
            std::swap(substitution.ai, substitution.aj);
        }
    }

    for (auto& expression : result.expressions)
        for (Term& term : expression)
            term.value *= getSign(term.index, result.dimension, signs);

    return result;
}

} // namespace leo
