#include <leo/optimization/group.h>

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace leo {

Group::Group(const ExpressionsSystem& expressionsSystem) :
             expressionsSystem(&expressionsSystem),
             lowerBound(expressionsSystem.getAdditionsLowerBound()),
             solution(expressionsSystem.getNaiveSolution()),
             additions(expressionsSystem.getNaiveAdditions()),
             strategyName("naive"),
             pool(expressionsSystem.getVariablesCount()) {

}

bool Group::add(const Solution& solution, bool wasPolished) {
    auto [it, inserted] = signatures.insert(solution.getSignature());
    if (!inserted)
        return false;

    solutions.push_back(solution);
    polished.push_back(wasPolished);
    return true;
}

void Group::update(size_t from) {
    if (from >= solutions.size())
        return;

    weights = getWeights();

    for (size_t i = from; i < solutions.size(); i++)
        pool.addSolution(solutions[i], weights[i]);
}

std::vector<std::pair<size_t, size_t>> Group::getPolishCandidates(size_t slack) const {
    if (additions <= lowerBound)
        return {};

    std::vector<std::pair<size_t, size_t>> candidates;

    for (size_t i = 0; i < solutions.size(); i++) {
        size_t solutionAdditions = solutions[i].getAdditions();

        if (!polished[i] && solutionAdditions <= additions + slack)
            candidates.push_back({solutionAdditions, i});
    }

    std::sort(candidates.begin(), candidates.end());
    return candidates;
}

std::vector<Substitution> Group::getRandomSubstitutions(std::mt19937& generator, double minRatio, double maxRatio) const {
    std::uniform_int_distribution<size_t> distribution(additions * minRatio, additions * maxRatio);
    return pool.getRandomSubstitutions(generator, distribution(generator));
}

std::vector<Vector> Group::getVectors(std::mt19937& generator, size_t minRemoved, size_t maxRemoved) const {
    std::discrete_distribution<size_t> distribution(weights.begin(), weights.end());
    const Solution& solution = solutions[distribution(generator)];
    std::vector<Vector> intermediates = getIntermediates(expressionsSystem->getExpressions(), solution);
    return removeRandomVectors(intermediates, generator, minRemoved, maxRemoved);
}

std::vector<double> Group::getWeights(double temperature, double eps) const {
    std::vector<size_t> values;
    for (const Solution& solution : solutions)
        values.push_back(solution.getAdditions());

    std::vector<double> weights;
    if (values.empty())
        return weights;

    std::unordered_map<size_t, size_t> counts;
    for (size_t value : values)
        counts[value]++;

    double sum = 0;
    for (size_t value : values) {
        double weight = std::exp(-double(value - additions) / temperature) / counts[value];
        weights.push_back(weight);
        sum += weight;
    }

    double uniform = 1.0 / values.size();
    for (double& weight : weights)
        weight = weight / sum * (1 - eps) + uniform * eps;

    return weights;
}

std::vector<Vector> Group::getIntermediates(const std::vector<std::vector<int>>& expressions, const Solution& solution) const {
    std::unordered_set<Vector> targets;

    for (const std::vector<int>& expression : expressions) {
        Vector target(expression);

        if (target.getSupport() >= 2)
            targets.insert(target.getCanonized());
    }

    std::vector<Vector> vectors;
    vectors.reserve(solution.dimension + solution.substitutions.size());
    for (size_t i = 0; i < solution.dimension; i++)
        vectors.emplace_back(solution.dimension, i);

    std::unordered_set<Vector> seen;
    std::vector<Vector> intermediates;

    for (const Substitution& s : solution.substitutions) {
        Vector vector = vectors[s.i] * s.ai + vectors[s.j] * s.aj;
        vectors.push_back(vector);

        if (vector.getSupport() < 2)
            continue;

        Vector canonized = vector.getCanonized();
        if (targets.count(canonized) || !seen.insert(canonized).second)
            continue;

        intermediates.push_back(std::move(canonized));
    }

    return intermediates;
}

std::vector<Vector> Group::removeRandomVectors(const std::vector<Vector>& intermediates, std::mt19937& generator, size_t minRemoved, size_t maxRemoved) const {
    std::uniform_int_distribution<size_t> distribution(minRemoved, std::max(minRemoved, maxRemoved));
    size_t count = std::min(distribution(generator), intermediates.size());

    std::vector<size_t> order(intermediates.size());
    for (size_t i = 0; i < intermediates.size(); i++)
        order[i] = i;

    std::shuffle(order.begin(), order.end(), generator);

    std::vector<bool> removed(intermediates.size(), false);
    for (size_t i = 0; i < count; i++)
        removed[order[i]] = true;

    std::vector<Vector> vectors;
    vectors.reserve(intermediates.size() - count);

    for (size_t i = 0; i < intermediates.size(); i++)
        if (!removed[i])
            vectors.push_back(intermediates[i]);

    return vectors;
}

} // namespace leo
