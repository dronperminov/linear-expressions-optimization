#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <leo/entities/expressions_system.h>
#include <leo/entities/helper_pool.h>
#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>

namespace leo {

struct Group {
    const ExpressionsSystem* expressionsSystem;
    size_t lowerBound;
    Solution solution;
    size_t additions;
    std::string strategyName;
    HelperPool pool;
    std::vector<Solution> solutions;
    std::vector<double> weights;
    std::vector<bool> polished;
    std::unordered_set<uint64_t> signatures;

    Group(const ExpressionsSystem& expressionsSystem);

    bool add(const Solution& solution, bool wasPolished = false);
    void update(size_t from);

    std::vector<std::pair<size_t, size_t>> getPolishCandidates(size_t slack) const;
    std::vector<Substitution> getRandomSubstitutions(std::mt19937& generator, double minRatio, double maxRatio) const;
    std::vector<Vector> getVectors(std::mt19937& generator, size_t minRemoved, size_t maxRemoved) const;
private:
    std::vector<double> getWeights(double temperature = 1.0, double eps = 0.01) const;
    std::vector<Vector> getIntermediates(const std::vector<std::vector<int>>& expressions, const Solution& solution) const;
    std::vector<Vector> removeRandomVectors(const std::vector<Vector>& intermediates, std::mt19937& generator, size_t minRemoved, size_t maxRemoved) const;
};

} // namespace leo
