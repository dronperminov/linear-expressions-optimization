#pragma once

#include <cstddef>
#include <random>
#include <vector>
#include <unordered_map>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/vector.h>

namespace leo {

class HelperPool {
    const size_t NOT_SELECTED = -1;
    const size_t MAX_STEPS_PER_TARGET = 1000;

    struct SamplingState {
        size_t steps;
        std::vector<size_t> selected;
        std::vector<bool> inProgress;
        std::vector<size_t> added;
        std::vector<Substitution> substitutions;
    };

    size_t dimension;
    std::vector<Vector> vectors;
    std::unordered_map<Vector, size_t> vector2index;
    std::vector<std::vector<Substitution>> index2substitutions;
    std::vector<std::vector<double>> index2weights;
public:
    HelperPool(size_t dimension);

    bool empty() const;
    size_t size() const;

    void reset();
    void addSolution(const Solution& solution, double weight = 1.0);
    void print() const;

    std::vector<Substitution> getRandomSubstitutions(std::mt19937& generator, size_t limit) const;
private:
    size_t getOrAdd(const Vector& canonized);
    void addSubstitution(size_t index, const Substitution& substitution, double weight);

    bool build(size_t index, std::mt19937& generator, size_t limit, SamplingState& state) const;
    std::vector<size_t> getWeightedOrder(const std::vector<double>& weights, std::mt19937& generator) const;
};

} // namespace leo
