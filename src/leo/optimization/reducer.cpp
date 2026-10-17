#include <leo/optimization/reducer.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace leo {

Reducer::Group::Group(const ExpressionsSystem& expressionsSystem) :
                      expressionsSystem(&expressionsSystem),
                      lowerBound(expressionsSystem.getAdditionsLowerBound()),
                      solution(expressionsSystem.getNaiveSolution()),
                      additions(expressionsSystem.getNaiveAdditions()),
                      strategyName("naive"),
                      pool(expressionsSystem.getVariablesCount()) {

}

Reducer::Reducer(size_t threads) : threads(threads ? threads : 1) {

}

size_t Reducer::addGroup(const ExpressionsSystem& expressionsSystem) {
    groups.emplace_back(expressionsSystem);
    return groups.size() - 1;
}

bool Reducer::reduce(const std::vector<TaskPool>& pools, const ReduceParameters& parameters) {
    if (pools.size() != groups.size())
        throw std::invalid_argument("Reducer::reduce: expected " + std::to_string(groups.size()) + " task pools (one per group), got " + std::to_string(pools.size()));

    std::vector<Job> jobs = getJobs(pools);
    if (jobs.empty())
        return false;

    std::vector<std::atomic<size_t>> shared(groups.size());
    for (size_t group = 0; group < groups.size(); group++)
        shared[group] = groups[group].additions;

    std::vector<std::optional<ReducerResult>> best(groups.size(), std::nullopt);
    std::vector<std::vector<Solution>> collected(groups.size());

    #pragma omp parallel num_threads(threads)
    {
        std::vector<std::optional<ReducerResult>> localBest(groups.size(), std::nullopt);
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

            std::optional<size_t> result = solve(*solver, parameters, task, group);
            if (!result)
                continue;

            size_t solverAdditions = *result;
            Solution solution = solver->getSolution();
            localSolutions[group].push_back(solution);

            std::optional<ReducerResult>& local = localBest[group];
            bool isBetter = solverAdditions < (local ? local->additions : groups[group].additions);
            if (isBetter)
                local = ReducerResult{solverAdditions, taskId, std::move(solution)};

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

                std::vector<Solution>& solutions = localSolutions[group];
                collected[group].insert(collected[group].end(), std::make_move_iterator(solutions.begin()), std::make_move_iterator(solutions.end()));
            }
        }
    }

    return finalize(pools, best, collected);
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

std::vector<Reducer::Job> Reducer::getJobs(const std::vector<TaskPool>& pools) const {
    std::vector<Job> jobs;
    for (size_t group = 0; group < pools.size(); group++)
        for (size_t taskId = 0; taskId < pools[group].size(); taskId++)
            jobs.push_back({group, taskId});

    return jobs;
}

std::vector<double> Reducer::getWeights(const std::vector<Solution>& solutions, size_t best, double temperature, double eps) const {
    std::vector<size_t> additions;
    for (const Solution& solution : solutions)
        additions.push_back(solution.getAdditions());

    std::vector<double> weights;
    if (additions.empty())
        return weights;

    std::unordered_map<size_t, size_t> counts;
    for (size_t value : additions)
        counts[value]++;

    double sum = 0;
    for (size_t value : additions) {
        double weight = std::exp(-double(value - best) / temperature) / counts[value];
        weights.push_back(weight);
        sum += weight;
    }

    double uniform = 1.0 / additions.size();
    for (double& weight : weights)
        weight = weight / sum * (1 - eps) + uniform * eps;

    return weights;
}

std::vector<Vector> Reducer::getIntermediates(const std::vector<std::vector<int>>& expressions, const Solution& solution) const {
    std::unordered_set<Vector> targets;

    for (const std::vector<int>& expression : expressions) {
        Vector target(expression);

        if (target.getSupport() >= 2)
            targets.insert(target.getCanonized());
    }

    std::vector<Vector> vectors;
    vectors.reserve(solution.dimension + solution.substitutions.size());
    for (size_t i = 0; i < solution.dimension; i++)
        vectors.emplace_back(solution.dimension, i);

    std::unordered_set<Vector> seen;
    std::vector<Vector> intermediates;

    for (const Substitution& s : solution.substitutions) {
        Vector vector = vectors[s.i] * s.ai + vectors[s.j] * s.aj;
        vectors.push_back(vector);

        if (vector.getSupport() < 2)
            continue;

        Vector canonized = vector.getCanonized();
        if (targets.count(canonized) || !seen.insert(canonized).second)
            continue;

        intermediates.push_back(std::move(canonized));
    }

    return intermediates;
}

std::vector<Vector> Reducer::getVectors(const std::vector<Vector>& intermediates, std::mt19937& generator, size_t minRemoved, size_t maxRemoved) const {
    std::uniform_int_distribution<size_t> distribution(minRemoved, std::max(minRemoved, maxRemoved));
    size_t count = std::min(distribution(generator), intermediates.size());

    std::vector<size_t> order(intermediates.size());
    for (size_t i = 0; i < intermediates.size(); i++)
        order[i] = i;

    std::shuffle(order.begin(), order.end(), generator);

    std::vector<bool> removed(intermediates.size(), false);
    for (size_t i = 0; i < count; i++)
        removed[order[i]] = true;

    std::vector<Vector> vectors;
    vectors.reserve(intermediates.size() - count);

    for (size_t i = 0; i < intermediates.size(); i++)
        if (!removed[i])
            vectors.push_back(intermediates[i]);

    return vectors;
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

std::optional<size_t> Reducer::solve(Solver& solver, const ReduceParameters& parameters, const Task& task, size_t group) const {
    std::seed_seq sequence{task.seed, 0x5EEDu};
    std::mt19937 generator(sequence);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    const bool canSubstitutions = solver.canStartFromSubstitutions() && !groups[group].pool.empty();
    const bool canVectors = solver.canStartFromVectors() && !groups[group].solutions.empty();

    const double substitutionsWeight = canSubstitutions ? parameters.substitutionsRatio : 0.0;
    const double vectorsWeight = canVectors ? parameters.vectorsRatio : 0.0;
    const double scratchWeight = 1.0 - parameters.substitutionsRatio - parameters.vectorsRatio;

    StartMode mode = chooseStartMode(scratchWeight, substitutionsWeight, vectorsWeight, uniform(generator));

    if (mode == StartMode::Substitutions)
        return solveFromSubstitutions(solver, parameters, group, generator);

    if (mode == StartMode::Vectors)
        return solveFromVectors(solver, parameters, group, generator);

    return solver.solve();
}

std::optional<size_t> Reducer::solveFromSubstitutions(Solver& solver, const ReduceParameters& parameters, size_t group, std::mt19937& generator) const {
    size_t minAdditions = groups[group].additions * parameters.minAdditionsRatio;
    size_t maxAdditions = groups[group].additions * parameters.maxAdditionsRatio;

    std::uniform_int_distribution<size_t> distribution(minAdditions, maxAdditions);
    size_t count = distribution(generator);
    std::vector<Substitution> substitutions = groups[group].pool.getRandomSubstitutions(generator, count);

    return solver.solve(substitutions);
}

std::optional<size_t> Reducer::solveFromVectors(Solver& solver, const ReduceParameters& parameters, size_t group, std::mt19937& generator) const {
    std::discrete_distribution<size_t> distribution(groups[group].weights.begin(), groups[group].weights.end());
    const Solution& solution = groups[group].solutions[distribution(generator)];
    std::vector<Vector> intermediates = getIntermediates(groups[group].expressionsSystem->getExpressions(), solution);
    std::vector<Vector> vectors = getVectors(intermediates, generator, parameters.minRemoveVectors, parameters.maxRemoveVectors);

    return solver.solve(vectors);
}

bool Reducer::finalize(const std::vector<TaskPool>& pools, const std::vector<std::optional<ReducerResult>>& best, const std::vector<std::vector<Solution>>& collected) {
    bool improved = false;

    for (size_t group = 0; group < groups.size(); group++) {
        Group& g = groups[group];

        if (best[group]) {
            g.additions = best[group]->additions;
            g.solution = best[group]->solution;
            g.strategyName = pools[group][best[group]->taskId].strategy->name;
            improved = true;
        }

        g.solutions.clear();
        if (!best[group])
            g.solutions.push_back(g.solution);

        const size_t offset = g.solutions.size();
        g.solutions.insert(g.solutions.end(), collected[group].begin(), collected[group].end());
        g.weights = getWeights(g.solutions, g.additions);

        for (size_t i = offset; i < g.solutions.size(); i++)
            g.pool.addSolution(g.solutions[i], g.weights[i]);
    }

    return improved;
}

} // namespace leo
