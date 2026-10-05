#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <leo/optimization/solvers/solver.h>

namespace leo {

using SolverFactory = std::function<std::unique_ptr<Solver>(const std::vector<std::vector<int>>& expressions, uint32_t seed)>;

struct Strategy {
    std::string name;
    double weight;
    SolverFactory create;
};

struct Task {
    std::shared_ptr<const Strategy> strategy;
    uint32_t seed;
};

class TaskPool {
    std::vector<Task> tasks;
public:
    void add(Task task);
    void add(const TaskPool& pool);

    size_t size() const;
    bool empty() const;

    const Task& operator[](size_t index) const;
};

class StrategyPool {
    std::vector<std::shared_ptr<const Strategy>> strategies;
public:
    void add(std::string name, double weight, SolverFactory create);
    void add(std::string name, SolverFactory create);

    TaskPool sample(size_t count, std::mt19937& generator) const;
    TaskPool each(size_t repeats, std::mt19937& generator) const;
};

} // namespace leo
