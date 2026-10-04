#pragma once

#include <atomic>
#include <exception>
#include <optional>
#include <stdexcept>

#include <leo/entities/expressions_system.h>
#include <leo/entities/solution.h>
#include <leo/optimization/strategy_pool.h>

namespace leo {

class Reducer {
    struct Group {
        const ExpressionsSystem* expressionsSystem;
        size_t lowerBound;
        Solution solution;
        size_t additions;
        std::string strategyName;
    };

    std::vector<Group> groups;
    size_t threads;
public:
    Reducer(size_t threads = 1);

    size_t addGroup(const ExpressionsSystem& expressionsSystem);

    bool reduce(const std::vector<TaskPool>& pools);

    size_t getGroupsCount() const;
    size_t getLowerBound(size_t group) const;
    size_t getAdditions(size_t group) const;
    bool isOptimal(size_t group) const;

    const Solution& getSolution(size_t group) const;
    const std::string& getStrategyName(size_t group) const;
};

} // namespace leo
