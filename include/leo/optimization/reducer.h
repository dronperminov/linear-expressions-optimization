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

class Reducer {
    enum class StartMode {
        Scratch,
        Substitutions,
        Vectors
    };

    struct Group {
        const ExpressionsSystem* expressionsSystem;
        size_t lowerBound;
        Solution solution;
        size_t additions;
        std::string strategyName;
        HelperPool pool;
        std::vector<Solution> solutions;
        std::vector<double> weights;

        Group(const ExpressionsSystem& expressionsSystem);
    };

    struct ReducerResult {
        size_t additions;
        size_t taskId;
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

    size_t getGroupsCount() const;
    size_t getLowerBound(size_t group) const;
    size_t getAdditions(size_t group) const;
    bool isOptimal(size_t group) const;

    const Solution& getSolution(size_t group) const;
    const std::string& getStrategyName(size_t group) const;
private:
    std::vector<Job> getJobs(const std::vector<TaskPool>& pools) const;

    std::vector<double> getWeights(const std::vector<Solution>& solutions, size_t best, double temperature = 1.0, double eps = 0.01) const;
    std::vector<Vector> getIntermediates(const std::vector<std::vector<int>>& expressions, const Solution& solution) const;
    std::vector<Vector> getVectors(const std::vector<Vector>& intermediates, std::mt19937& generator, size_t minRemoved, size_t maxRemoved) const;

    StartMode chooseStartMode(double scratch, double substitutions, double vectors, double p) const;

    std::optional<size_t> solve(Solver& solver, const ReduceParameters& parameters, const Task& task, size_t group) const;
    std::optional<size_t> solveFromSubstitutions(Solver& solver, const ReduceParameters& parameters, size_t group, std::mt19937& generator) const;
    std::optional<size_t> solveFromVectors(Solver& solver, const ReduceParameters& parameters, size_t group, std::mt19937& generator) const;

    bool finalize(const std::vector<TaskPool>& pools, const std::vector<std::optional<ReducerResult>>& best, const std::vector<std::vector<Solution>>& collected);
};

} // namespace leo
