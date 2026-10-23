#include <leo/optimization/reducer.h>

#include <atomic>
#include <chrono>
#include <iterator>
#include <stdexcept>

namespace leo {

Reducer::Reducer(size_t threads) : threads(threads ? threads : 1) {

}

size_t Reducer::addGroup(const ExpressionsSystem& expressionsSystem) {
    groups.emplace_back(expressionsSystem);
    return groups.size() - 1;
}

bool Reducer::reduce(const std::vector<TaskPool>& pools, const ReduceParameters& parameters) {
    if (pools.size() != groups.size())
        throw std::invalid_argument("Reducer::reduce: expected " + std::to_string(groups.size()) + " task pools (one per group), got " + std::to_string(pools.size()));

    std::vector<Job> jobs = getReduceJobs(pools);
    if (jobs.empty())
        return false;

    std::vector<std::atomic<size_t>> shared(groups.size());
    for (size_t group = 0; group < groups.size(); group++)
        shared[group] = groups[group].additions;

    std::vector<std::optional<ReduceResult>> best(groups.size(), std::nullopt);
    std::vector<std::vector<Solution>> collected(groups.size());

    #pragma omp parallel num_threads(threads)
    {
        std::vector<std::optional<ReduceResult>> localBest(groups.size(), std::nullopt);
        std::vector<std::vector<Solution>> localSolutions(groups.size());

        #pragma omp for schedule(dynamic)
        for (size_t jobId = 0; jobId < jobs.size(); jobId++) {
            const size_t group = jobs[jobId].group;
            const size_t taskId = jobs[jobId].taskId;

            if (shared[group] <= groups[group].lowerBound)
                continue;

            const Task& task = pools[group][taskId];
            std::unique_ptr<Solver> solver = task.strategy->create(groups[group].expressionsSystem->getExpressions(), task.seed);

            if (parameters.boundByBest)
                solver->setBound(shared[group].load());

            std::optional<size_t> result = solve(*solver, parameters, task.seed, group);
            if (!result)
                continue;

            size_t solverAdditions = *result;
            Solution solution = solver->getSolution();
            localSolutions[group].push_back(solution);

            std::optional<ReduceResult>& local = localBest[group];
            bool isBetter = solverAdditions < (local ? local->additions : groups[group].additions);
            if (isBetter)
                local = ReduceResult{solverAdditions, taskId, std::move(solution)};

            size_t current = shared[group].load();
            while (solverAdditions < current && !shared[group].compare_exchange_weak(current, solverAdditions))
                ;
        }

        #pragma omp critical
        {
            for (size_t group = 0; group < groups.size(); group++) {
                std::optional<ReduceResult>& local = localBest[group];

                if (local && (!best[group] || std::tie(local->additions, local->taskId) < std::tie(best[group]->additions, best[group]->taskId)))
                    best[group] = std::move(local);

                std::vector<Solution>& solutions = localSolutions[group];
                collected[group].insert(collected[group].end(), std::make_move_iterator(solutions.begin()), std::make_move_iterator(solutions.end()));
            }
        }
    }

    return finalizeReduce(pools, best, collected);
}

bool Reducer::polish(const PolishParameters& parameters) {
    if (parameters.maxRemoved == 0)
        return false;

    #pragma omp parallel for num_threads(threads) schedule(dynamic)
    for (size_t group = 0; group < groups.size(); group++)
        groups[group].pool.addAllProductions();

    std::vector<Job> jobs = getPolishJobs(parameters);
    if (jobs.empty())
        return false;

    std::vector<std::atomic<size_t>> shared(groups.size());
    for (size_t group = 0; group < groups.size(); group++)
        shared[group] = groups[group].additions;

    std::vector<PolishResult> results;

    #pragma omp parallel num_threads(threads)
    {
        std::vector<std::unique_ptr<HelperExchange>> exchanges(groups.size());
        std::vector<PolishResult> local;

        #pragma omp for schedule(dynamic)
        for (size_t jobId = 0; jobId < jobs.size(); jobId++) {
            const size_t group = jobs[jobId].group;
            Group& g = groups[group];

            if (shared[group] <= g.lowerBound)
                continue;

            if (!exchanges[group])
                exchanges[group] = std::make_unique<HelperExchange>(g.pool);

            Solution current = g.solutions[jobs[jobId].taskId];
            bool improved = false;
            const auto start = std::chrono::steady_clock::now();

            while (current.getAdditions() > g.lowerBound) {
                double remaining = parameters.maxTimeInSeconds - std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
                if (remaining <= 0.0)
                    break;

                std::optional<Solution> better = exchanges[group]->improve(current, parameters.maxRemoved, remaining);
                if (!better || !g.expressionsSystem->validateSolution(*better))
                    break;

                current = std::move(*better);
                improved = true;
            }

            g.polished[jobs[jobId].taskId] = true;

            if (!improved)
                continue;

            size_t additions = current.getAdditions();
            size_t seen = shared[group].load();
            while (additions < seen && !shared[group].compare_exchange_weak(seen, additions))
                ;

            local.push_back({group, std::move(current)});
        }

        #pragma omp critical
        results.insert(results.end(), std::make_move_iterator(local.begin()), std::make_move_iterator(local.end()));
    }

    return finalizePolish(results);
}

size_t Reducer::getGroupsCount() const {
    return groups.size();
}

size_t Reducer::getSolutionsCount(size_t group) const {
    return groups.at(group).solutions.size();
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

std::vector<Reducer::Job> Reducer::getReduceJobs(const std::vector<TaskPool>& pools) const {
    std::vector<Job> jobs;
    for (size_t group = 0; group < pools.size(); group++)
        for (size_t taskId = 0; taskId < pools[group].size(); taskId++)
            jobs.push_back({group, taskId});

    return jobs;
}

std::vector<Reducer::Job> Reducer::getPolishJobs(const PolishParameters& parameters) const {
    std::vector<Job> jobs;

    for (size_t group = 0; group < groups.size(); group++)
        for (const auto& candidate : groups[group].getPolishCandidates(parameters.slack))
            jobs.push_back({group, candidate.second});

    return jobs;
}

Reducer::StartMode Reducer::chooseStartMode(double scratch, double substitutions, double vectors, double p) const {
    const double weights[3] = {scratch, substitutions, vectors};
    const StartMode modes[3] = {StartMode::Scratch, StartMode::Substitutions, StartMode::Vectors};

    double total = scratch + substitutions + vectors;
    if (total <= 0.0)
        return StartMode::Scratch;

    double x = p * total;
    StartMode last = StartMode::Scratch;

    for (size_t i = 0; i < 3; i++) {
        if (weights[i] <= 0.0)
            continue;

        last = modes[i];
        if (x < weights[i])
            return last;

        x -= weights[i];
    }

    return last;
}

std::optional<size_t> Reducer::solve(Solver& solver, const ReduceParameters& parameters, uint32_t seed, size_t group) const {
    std::seed_seq sequence{seed, 0x5EEDu};
    std::mt19937 generator(sequence);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    const bool canSubstitutions = solver.canStartFromSubstitutions() && !groups[group].pool.empty();
    const bool canVectors = solver.canStartFromVectors() && !groups[group].solutions.empty();

    const double substitutionsWeight = canSubstitutions ? parameters.substitutionsRatio : 0.0;
    const double vectorsWeight = canVectors ? parameters.vectorsRatio : 0.0;
    const double scratchWeight = 1.0 - parameters.substitutionsRatio - parameters.vectorsRatio;

    StartMode mode = chooseStartMode(scratchWeight, substitutionsWeight, vectorsWeight, uniform(generator));

    if (mode == StartMode::Substitutions) {
        std::vector<Substitution> substitutions = groups[group].getRandomSubstitutions(generator, parameters.minAdditionsRatio, parameters.maxAdditionsRatio);
        return solver.solve(substitutions);
    }

    if (mode == StartMode::Vectors) {
        std::vector<Vector> vectors = groups[group].getVectors(generator, parameters.minRemoveVectors, parameters.maxRemoveVectors);
        return solver.solve(vectors);
    }

    return solver.solve();
}

bool Reducer::finalizeReduce(const std::vector<TaskPool>& pools, const std::vector<std::optional<ReduceResult>>& best, const std::vector<std::vector<Solution>>& collected) {
    bool improved = false;

    for (size_t group = 0; group < groups.size(); group++) {
        Group& g = groups[group];

        if (best[group]) {
            g.additions = best[group]->additions;
            g.solution = best[group]->solution;
            g.strategyName = pools[group][best[group]->taskId].strategy->name;
            improved = true;
        }

        const size_t offset = g.solutions.size();

        g.add(g.solution);
        for (const Solution& solution : collected[group])
            g.add(solution);

        g.update(offset);
    }

    return improved;
}

bool Reducer::finalizePolish(const std::vector<PolishResult>& results) {
    std::vector<size_t> from(groups.size());
    for (size_t group = 0; group < groups.size(); group++)
        from[group] = groups[group].solutions.size();

    bool improved = false;

    for (const PolishResult& result : results) {
        Group& g = groups[result.group];

        size_t additions = result.solution.getAdditions();
        if (additions < g.additions) {
            g.additions = additions;
            g.solution = result.solution;
            g.strategyName = "helper exchange";
            improved = true;
        }

        g.add(result.solution, true);
    }

    for (size_t group = 0; group < groups.size(); group++)
        groups[group].update(from[group]);

    return improved;
}

} // namespace leo
