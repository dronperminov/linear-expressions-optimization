#pragma once

#include <cstddef>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include <leo/entities/expressions_system.h>
#include <leo/entities/helper_pool.h>
#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/optimization/group.h>
#include <leo/optimization/helper_exchange.h>
#include <leo/optimization/strategy_pool.h>

namespace leo {

struct ReduceParameters {
    bool boundByBest;

    double substitutionsRatio;
    double vectorsRatio;

    double minAdditionsRatio;
    double maxAdditionsRatio;
    size_t minRemoveVectors;
    size_t maxRemoveVectors;
};

struct PolishParameters {
    size_t maxRemoved;
    size_t slack;
    double maxTimeInSeconds;
};

class Reducer {
    enum class StartMode {
        Scratch,
        Substitutions,
        Vectors
    };

    struct ReduceResult {
        size_t additions;
        size_t taskId;
        Solution solution;
    };

    struct PolishResult {
        size_t group;
        Solution solution;
    };

    struct Job {
        size_t group;
        size_t taskId;
    };

    std::vector<Group> groups;
    size_t threads;
public:
    Reducer(size_t threads = 1);

    size_t addGroup(const ExpressionsSystem& expressionsSystem);

    bool reduce(const std::vector<TaskPool>& pools, const ReduceParameters& parameters);
    bool polish(const PolishParameters& parameters);

    size_t getGroupsCount() const;
    size_t getSolutionsCount(size_t group) const;
    size_t getLowerBound(size_t group) const;
    size_t getAdditions(size_t group) const;
    bool isOptimal(size_t group) const;

    const Solution& getSolution(size_t group) const;
    const std::string& getStrategyName(size_t group) const;
private:
    std::vector<Job> getReduceJobs(const std::vector<TaskPool>& pools) const;
    std::vector<Job> getPolishJobs(const PolishParameters& parameters) const;

    StartMode chooseStartMode(double scratch, double substitutions, double vectors, double p) const;

    std::optional<size_t> solve(Solver& solver, const ReduceParameters& parameters, uint32_t seed, size_t group) const;

    bool finalizeReduce(const std::vector<TaskPool>& pools, const std::vector<std::optional<ReduceResult>>& best, const std::vector<std::vector<Solution>>& collected);
    bool finalizePolish(const std::vector<PolishResult>& results);
};

} // namespace leo
