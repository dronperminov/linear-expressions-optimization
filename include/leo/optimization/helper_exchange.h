#pragma once

#include <chrono>
#include <cstddef>
#include <optional>
#include <vector>

#include <leo/entities/helper_pool.h>
#include <leo/entities/solution.h>
#include <leo/entities/vector.h>

namespace leo {

class HelperExchange {
    const size_t MAX_ADDED = 2;
    const int NOT_READY = -1;
    const int BASIS = -2;

    struct Edge {
        size_t out;
        size_t i;
        size_t j;
        int ai;
        int aj;
    };

    const HelperPool& pool;
    size_t dimension;
    size_t count;
    std::vector<Edge> edges;
    std::vector<std::vector<size_t>> byOut;
    std::vector<std::vector<size_t>> byParent;

    std::vector<size_t> required;
    std::vector<size_t> helpers;
    std::vector<char> base;
    std::vector<char> isHelper;
    std::vector<size_t> candidates;
    std::chrono::steady_clock::time_point deadline;

    std::vector<char> allowed;
    std::vector<int> producedBy;
    std::vector<size_t> order;
    std::vector<size_t> seen;
    size_t stamp = 0;
public:
    HelperExchange(const HelperPool& pool);

    std::optional<Solution> improve(const Solution& solution, size_t maxRemoved, double seconds);
private:
    bool load(const Solution& solution, const std::vector<Vector>& vectors);

    bool isReady(size_t o) const;
    bool isTimeout() const;
    bool done() const;

    void reset(const std::vector<size_t>& removed);
    void grow(size_t from);
    void add(size_t h);
    void undo(size_t mark, size_t h);
    std::vector<size_t> collect(const std::vector<size_t>& previous, size_t from);

    bool search(size_t maxRemoved);
    bool chooseRemoved(size_t r, size_t start, std::vector<size_t>& removed);
    bool tryRemoved(const std::vector<size_t>& removed);
    bool insert(size_t budget, const std::vector<size_t>& viable);

    std::optional<Solution> build(const Solution& old, const std::vector<Vector>& oldVectors) const;
};

} // namespace leo
