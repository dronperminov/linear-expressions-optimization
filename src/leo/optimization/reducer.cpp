#include <leo/optimization/reducer.h>

#include <atomic>
#include <optional>
#include <stdexcept>

namespace leo {

namespace {
struct ReducerResult {
    size_t additions;
    size_t taskId;
    Solution solution;
};

struct Job {
    size_t group;
    size_t taskId;
};
} // namespace

Reducer::Reducer(size_t threads) : threads(threads ? threads : 1) {

}

size_t Reducer::addGroup(const ExpressionsSystem& expressionsSystem) {
    Group group;
    group.expressionsSystem = &expressionsSystem;
    group.lowerBound = expressionsSystem.getAdditionsLowerBound();
    group.solution = expressionsSystem.getNaiveSolution();
    group.additions = group.solution.getAdditions();
    group.strategyName = "naive";

    groups.push_back(std::move(group));
    return groups.size() - 1;
}

bool Reducer::reduce(const std::vector<TaskPool>& pools, bool boundByBest) {
    if (pools.size() != groups.size())
        throw std::invalid_argument("Reducer::reduce: expected " + std::to_string(groups.size()) + " task pools (one per group), got " + std::to_string(pools.size()));

    std::vector<Job> jobs;
    for (size_t group = 0; group < pools.size(); group++)
        for (size_t taskId = 0; taskId < pools[group].size(); taskId++)
            jobs.push_back({group, taskId});

    if (jobs.empty())
        return false;

    std::vector<std::atomic<size_t>> shared(groups.size());
    for (size_t group = 0; group < groups.size(); group++)
        shared[group] = groups[group].additions;

    std::vector<std::optional<ReducerResult>> best(groups.size(), std::nullopt);

    #pragma omp parallel num_threads(threads)
    {
        std::vector<std::optional<ReducerResult>> localBest(groups.size(), std::nullopt);

        #pragma omp for schedule(dynamic)
        for (size_t jobId = 0; jobId < jobs.size(); jobId++) {
            const size_t group = jobs[jobId].group;
            const size_t taskId = jobs[jobId].taskId;

            if (shared[group] <= groups[group].lowerBound)
                continue;

            const Task& task = pools[group][taskId];
            std::unique_ptr<Solver> solver = task.strategy->create(groups[group].expressionsSystem->getExpressions(), task.seed);

            if (boundByBest && shared[group] < groups[group].additions)
                solver->setBound(shared[group]);

            const size_t solverAdditions = solver->solve();

            std::optional<ReducerResult>& local = localBest[group];
            if (solverAdditions < (local ? local->additions : groups[group].additions))
                local = ReducerResult{solverAdditions, taskId, solver->getSolution()};

            size_t current = shared[group].load();
            while (solverAdditions < current && !shared[group].compare_exchange_weak(current, solverAdditions))
                ;
        }

        #pragma omp critical
        {
            for (size_t group = 0; group < groups.size(); group++) {
                std::optional<ReducerResult>& local = localBest[group];

                if (local && (!best[group] || std::tie(local->additions, local->taskId) < std::tie(best[group]->additions, best[group]->taskId)))
                    best[group] = std::move(local);
            }
        }
    }

    bool improved = false;

    for (size_t group = 0; group < groups.size(); group++) {
        if (!best[group])
            continue;

        groups[group].additions = best[group]->additions;
        groups[group].solution = std::move(best[group]->solution);
        groups[group].strategyName = pools[group][best[group]->taskId].strategy->name;
        improved = true;
    }

    return improved;
}

size_t Reducer::getGroupsCount() const {
    return groups.size();
}

size_t Reducer::getLowerBound(size_t group) const {
    return groups.at(group).lowerBound;
}

size_t Reducer::getAdditions(size_t group) const {
    return groups.at(group).additions;
}

bool Reducer::isOptimal(size_t group) const {
    return groups.at(group).additions <= groups.at(group).lowerBound;
}

const Solution& Reducer::getSolution(size_t group) const {
    return groups.at(group).solution;
}

const std::string& Reducer::getStrategyName(size_t group) const {
    return groups.at(group).strategyName;
}

} // namespace leo
