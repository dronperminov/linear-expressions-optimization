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
    const ExpressionsSystem& expressionsSystem;
    size_t threads;
    size_t lowerBound;

    Solution solution;
    size_t additions;
    std::string strategyName;
public:
    Reducer(const ExpressionsSystem& expressionsSystem, size_t threads = 1);

    bool reduce(const TaskPool& pool);

    size_t getLowerBound() const;
    size_t getAdditions() const;
    bool isOptimal() const;

    const Solution& getSolution() const;
    const std::string& getStrategyName() const;
};

} // namespace leo
