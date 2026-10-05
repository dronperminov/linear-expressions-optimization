#include <leo/optimization/strategy_pool.h>

#include <stdexcept>

namespace leo {

void TaskPool::add(Task task) {
    tasks.push_back(std::move(task));
}

void TaskPool::add(const TaskPool& pool) {
    tasks.insert(tasks.end(), pool.tasks.begin(), pool.tasks.end());
}

size_t TaskPool::size() const {
    return tasks.size();
}

bool TaskPool::empty() const {
    return tasks.empty();
}

const Task& TaskPool::operator[](size_t index) const {
    return tasks[index];
}

void StrategyPool::add(std::string name, double weight, SolverFactory create) {
    if (weight <= 0)
        throw std::invalid_argument("StrategyPool::add: weight of \"" + name + "\" must be positive, got " + std::to_string(weight));

    if (!create)
        throw std::invalid_argument("StrategyPool::add: factory of \"" + name + "\" must not be empty");

    strategies.push_back(std::make_shared<const Strategy>(Strategy{std::move(name), weight, std::move(create)}));
}

void StrategyPool::add(std::string name, SolverFactory create) {
    add(std::move(name), 1.0, std::move(create));
}

TaskPool StrategyPool::sample(size_t count, std::mt19937& generator) const {
    TaskPool pool;
    if (strategies.empty())
        return pool;

    std::vector<double> weights;
    for (const auto& strategy : strategies)
        weights.push_back(strategy->weight);

    std::discrete_distribution<size_t> distribution(weights.begin(), weights.end());

    for (size_t i = 0; i < count; i++) {
        size_t index = distribution(generator);
        uint32_t seed = generator();
        pool.add({strategies[index], seed});
    }

    return pool;
}

TaskPool StrategyPool::each(size_t repeats, std::mt19937& generator) const {
    TaskPool pool;

    for (const auto& strategy : strategies) {
        for (size_t i = 0; i < repeats; i++) {
            uint32_t seed = generator();
            pool.add({strategy, seed});
        }
    }

    return pool;
}

} // namespace leo
