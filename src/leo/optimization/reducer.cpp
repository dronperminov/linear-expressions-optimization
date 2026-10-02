#include <leo/optimization/reducer.h>

namespace leo {

namespace {

struct ReducerResult {
    size_t additions;
    size_t taskId;
    Solution solution;
};

} // namespace

Reducer::Reducer(const ExpressionsSystem& expressionsSystem, size_t threads) : expressionsSystem(expressionsSystem), threads(threads ? threads : 1) {
    lowerBound = expressionsSystem.getAdditionsLowerBound();
    solution = expressionsSystem.getNaiveSolution();
    additions = solution.getAdditions();
    strategyName = "naive";
}

bool Reducer::reduce(const TaskPool& pool) {
    if (pool.empty())
        return false;

    std::atomic<size_t> shared(additions);
    std::optional<ReducerResult> best = std::nullopt;

    #pragma omp parallel num_threads(threads)
    {
        std::optional<ReducerResult> localBest = std::nullopt;

        #pragma omp for schedule(dynamic)
        for (size_t taskId = 0; taskId < pool.size(); taskId++) {
            if (shared <= lowerBound)
                continue;

            const Task& task = pool[taskId];
            std::unique_ptr<Solver> solver = task.strategy->create(expressionsSystem.getExpressions(), task.seed);

            const size_t solverAdditions = solver->solve();

            if (!localBest || solverAdditions < localBest->additions)
                localBest = ReducerResult{solverAdditions, taskId, solver->getSolution()};

            size_t current = shared.load();
            while (solverAdditions < current && !shared.compare_exchange_weak(current, solverAdditions));
        }

        #pragma omp critical
        {
            if (localBest && (!best || std::tie(localBest->additions, localBest->taskId) < std::tie(best->additions, best->taskId)))
                best = std::move(localBest);
        }
    }

    if (!best)
        return false;

    additions = best->additions;
    solution = std::move(best->solution);
    strategyName = pool[best->taskId].strategy->name;
    return true;
}

size_t Reducer::getLowerBound() const {
    return lowerBound;
}

size_t Reducer::getAdditions() const {
    return additions;
}

bool Reducer::isOptimal() const {
    return additions <= lowerBound;
}

const Solution& Reducer::getSolution() const {
    return solution;
}

const std::string& Reducer::getStrategyName() const {
    return strategyName;
}

} // namespace leo
