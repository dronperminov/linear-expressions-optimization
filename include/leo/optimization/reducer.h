#pragma once

#include <cstddef>
#include <random>
#include <string>
#include <vector>

#include <leo/entities/expressions_system.h>
#include <leo/entities/helper_pool.h>
#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/optimization/strategy_pool.h>

namespace leo {

struct ReduceParameters {
    bool boundByBest;
    bool startFromSubstitutions;
    double minStartRatio;
    double maxStartRatio;
};

class Reducer {
    struct Group {
        const ExpressionsSystem* expressionsSystem;
        size_t lowerBound;
        Solution solution;
        size_t additions;
        std::string strategyName;
        HelperPool pool;

        Group(const ExpressionsSystem& expressionsSystem);
    };

    std::vector<Group> groups;
    size_t threads;
public:
    Reducer(size_t threads = 1);

    size_t addGroup(const ExpressionsSystem& expressionsSystem);

    bool reduce(const std::vector<TaskPool>& pools, const ReduceParameters& parameters);

    size_t getGroupsCount() const;
    size_t getLowerBound(size_t group) const;
    size_t getAdditions(size_t group) const;
    bool isOptimal(size_t group) const;

    const Solution& getSolution(size_t group) const;
    const std::string& getStrategyName(size_t group) const;

    std::vector<double> getWeights(const std::vector<Solution>& solutions, size_t best, double temperature = 1.0, double eps = 0.01) const;
    std::vector<Substitution> getRandomSubstitutions(const std::vector<Solution>& solutions, const std::vector<double>& weights, std::mt19937& generator, double probability) const;
};

} // namespace leo
