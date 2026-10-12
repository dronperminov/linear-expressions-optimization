#include <leo/entities/helper_pool.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace leo {

HelperPool::HelperPool(size_t dimension) : dimension(dimension) {
    reset();
}

bool HelperPool::empty() const {
    return vectors.size() == dimension;
}

size_t HelperPool::size() const {
    return vectors.size() - dimension;
}

void HelperPool::reset() {
    vectors.clear();
    vectors.reserve(dimension);
    vector2index.clear();

    for (size_t i = 0; i < dimension; i++) {
        Vector basis(dimension, i);
        vector2index.emplace(basis, i);
        vectors.push_back(basis);
    }

    index2substitutions.resize(dimension);
    index2weights.resize(dimension);
}

void HelperPool::addSolution(const Solution& solution, double weight) {
    if (solution.dimension != dimension)
        throw std::invalid_argument("HelperPool::addSolution: solution dimension " + std::to_string(solution.dimension) + " does not match pool dimension " + std::to_string(dimension));

    if (weight == 0.0)
        return;

    size_t total = dimension + solution.substitutions.size();
    std::vector<size_t> indices(total);
    std::vector<int> signs(total, 1);

    for (size_t i = 0; i < dimension; i++)
        indices[i] = i;

    for (size_t step = 0; step < solution.substitutions.size(); step++) {
        const Substitution& s = solution.substitutions[step];
        size_t current = dimension + step;

        if (s.i >= current || s.j >= current)
            throw std::invalid_argument("HelperPool::addSolution: substitution uses future steps");

        size_t i = indices[s.i];
        size_t j = indices[s.j];
        int ai = s.ai * signs[s.i];
        int aj = s.aj * signs[s.j];

        Vector vector = vectors[i] * ai + vectors[j] * aj;
        if (vector.isZero())
            throw std::invalid_argument("HelperPool::addSolution: substitution produces zero vector");

        Vector canonized = vector.getCanonized();
        int sign = vector == canonized ? 1 : -1;

        size_t index = getOrAdd(canonized);
        indices[current] = index;
        signs[current] = sign;

        if (index < dimension || i == index || j == index)
            continue;

        ai *= sign;
        aj *= sign;

        if (i > j) {
            std::swap(i, j);
            std::swap(ai, aj);
        }

        addSubstitution(index, {i, j, ai, aj}, weight);
    }
}

void HelperPool::print() const {
    std::cout << "HelperPool(" << dimension << "): " << size() << " helpers" << std::endl;

    for (size_t i = dimension; i < vectors.size(); i++) {
        std::cout << "- v" << i << ": [";

        for (size_t j = 0; j < dimension; j++)
            std::cout << (j > 0 ? ", " : "") << vectors[i][j];
        std::cout << "]:" << std::endl;

        for (size_t j = 0; j < index2substitutions[i].size(); j++) {
            const Substitution& s = index2substitutions[i][j];
            double weight = index2weights[i][j];

            std::cout << "  (v" << s.i << " * " << s.ai << ") + (v" << s.j << " * " << s.aj << "): " << weight << std::endl;
        }
        std::cout << std::endl;
    }
}

std::vector<Substitution> HelperPool::getRandomSubstitutions(std::mt19937& generator, size_t limit) const {
    std::vector<size_t> selected(vectors.size(), NOT_SELECTED);
    std::vector<bool> inProgress(vectors.size(), 0);

    for (size_t i = 0; i < dimension; i++)
        selected[i] = i;

    SamplingState state = {0, selected, inProgress, {}, {}};

    std::vector<size_t> targets;
    std::vector<double> targetWeights;

    for (size_t index = dimension; index < vectors.size(); index++) {
        const std::vector<double>& weights = index2weights[index];
        double total = std::accumulate(weights.begin(), weights.end(), 0.0);

        if (total > 0.0) {
            targets.push_back(index);
            targetWeights.push_back(total);
        }
    }

    for (size_t i : getWeightedOrder(targetWeights, generator)) {
        if (state.substitutions.size() >= limit)
            break;

        state.steps = 0;
        build(targets[i], generator, limit, state);
    }

    return state.substitutions;
}

size_t HelperPool::getOrAdd(const Vector& canonized) {
    auto result = vector2index.find(canonized);
    if (result != vector2index.end())
        return result->second;

    size_t index = vectors.size();

    vector2index.emplace(canonized, index);
    vectors.push_back(canonized);
    index2substitutions.emplace_back();
    index2weights.emplace_back();

    return index;
}

void HelperPool::addSubstitution(size_t index, const Substitution& substitution, double weight) {
    std::vector<Substitution>& substitutions = index2substitutions[index];
    std::vector<double>& weights = index2weights[index];

    for (size_t i = 0; i < substitutions.size(); i++) {
        if (substitutions[i] == substitution) {
            weights[i] += weight;
            return;
        }
    }

    substitutions.push_back(substitution);
    weights.push_back(weight);
}

bool HelperPool::build(size_t index, std::mt19937& generator, size_t limit, SamplingState& state) const {
    if (state.selected[index] != NOT_SELECTED)
        return true;

    if (state.inProgress[index] || state.substitutions.size() >= limit)
        return false;

    const std::vector<Substitution>& candidates = index2substitutions[index];
    std::vector<size_t> order = getWeightedOrder(index2weights[index], generator);

    state.inProgress[index] = true;

    for (size_t i : order) {
        if (++state.steps > MAX_STEPS_PER_TARGET)
            break;

        const Substitution& s = candidates[i];
        size_t size = state.substitutions.size();

        if (build(s.i, generator, limit, state) && build(s.j, generator, limit, state) && state.substitutions.size() < limit) {
            state.selected[index] = dimension + state.substitutions.size();
            state.substitutions.push_back({state.selected[s.i], state.selected[s.j], s.ai, s.aj});
            state.added.push_back(index);
            state.inProgress[index] = false;
            return true;
        }

        while (state.substitutions.size() > size) {
            state.selected[state.added.back()] = NOT_SELECTED;
            state.added.pop_back();
            state.substitutions.pop_back();
        }
    }

    state.inProgress[index] = false;
    return false;
}

std::vector<size_t> HelperPool::getWeightedOrder(const std::vector<double>& weights, std::mt19937& generator) const {
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    std::vector<std::pair<double, size_t>> keys;
    keys.reserve(weights.size());

    for (size_t i = 0; i < weights.size(); i++)
        if (weights[i] > 0.0)
            keys.emplace_back(-std::log(1.0 - uniform(generator)) / weights[i], i);

    std::sort(keys.begin(), keys.end());

    std::vector<size_t> order;
    order.reserve(keys.size());

    for (const auto& key : keys)
        order.push_back(key.second);

    return order;
}

} // namespace leo
