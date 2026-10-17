#include <leo/optimization/helper_exchange.h>

#include <algorithm>
#include <limits>

namespace leo {

HelperExchange::HelperExchange(const HelperPool& pool) : pool(pool), dimension(pool.getDimension()), count(pool.getVectorsCount()) {
    byOut.resize(count);
    byParent.resize(count);

    for (size_t out = dimension; out < count; out++) {
        for (const Substitution& s : pool.getSubstitutions(out)) {
            size_t id = edges.size();
            edges.push_back({out, s.i, s.j, s.ai, s.aj});
            byOut[out].push_back(id);
            byParent[s.i].push_back(id);
            if (s.j != s.i)
                byParent[s.j].push_back(id);
        }
    }
}

std::optional<Solution> HelperExchange::improve(const Solution& solution, size_t maxRemoved, double seconds) {
    if (solution.dimension != dimension)
        return std::nullopt;

    std::vector<Vector> vectors;
    for (size_t i = 0; i < dimension; i++)
        vectors.emplace_back(dimension, i);
    for (const Substitution& s : solution.substitutions)
        vectors.push_back(vectors[s.i] * s.ai + vectors[s.j] * s.aj);

    if (!load(solution, vectors))
        return std::nullopt;

    reset({});
    if (!done())
        return std::nullopt;

    std::vector<size_t> all(count - dimension);
    std::iota(all.begin(), all.end(), dimension);
    candidates = collect(all, order.size());

    deadline = std::chrono::steady_clock::now() + std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(seconds));

    if (!search(maxRemoved))
        return std::nullopt;

    return build(solution, vectors);
}

bool HelperExchange::load(const Solution& solution, const std::vector<Vector>& vectors) {
    required.clear();
    helpers.clear();

    base.assign(count, 0);
    isHelper.assign(count, 0);
    seen.assign(count, 0);
    stamp = 0;

    for (size_t i = 0; i < dimension; i++)
        base[i] = 1;

    for (const std::vector<Term>& expression : solution.expressions) {
        for (const Term& term : expression) {
            Vector canonized = vectors[term.index].getCanonized();
            if (canonized.getSupport() < 2)
                continue;

            std::optional<size_t> index = pool.find(canonized);
            if (!index)
                return false;

            if (!base[*index]) {
                base[*index] = 1;
                required.push_back(*index);
            }
        }
    }

    for (size_t k = dimension; k < vectors.size(); k++) {
        Vector canonized = vectors[k].getCanonized();
        if (canonized.getSupport() < 2)
            continue;

        std::optional<size_t> index = pool.find(canonized);
        if (!index)
            return false;

        if (!base[*index]) {
            base[*index] = 1;
            isHelper[*index] = 1;
            helpers.push_back(*index);
        }
    }

    return true;
}

bool HelperExchange::isReady(size_t o) const {
    return producedBy[o] != NOT_READY;
}

bool HelperExchange::expired() const {
    return std::chrono::steady_clock::now() > deadline;
}

bool HelperExchange::done() const {
    for (size_t r : required)
        if (!isReady(r))
            return false;

    return true;
}

void HelperExchange::reset(const std::vector<size_t>& removed) {
    allowed = base;
    for (size_t h : removed)
        allowed[h] = 0;

    producedBy.assign(count, NOT_READY);
    order.clear();

    for (size_t i = 0; i < dimension; i++) {
        producedBy[i] = BASIS;
        order.push_back(i);
    }

    grow(0);
}

void HelperExchange::grow(size_t from) {
    for (size_t cursor = from; cursor < order.size(); cursor++) {
        for (size_t id : byParent[order[cursor]]) {
            const Edge& e = edges[id];
            if (!allowed[e.out] || isReady(e.out) || !isReady(e.a) || !isReady(e.b))
                continue;

            producedBy[e.out] = int(id);
            order.push_back(e.out);
        }
    }
}

void HelperExchange::add(size_t h) {
    allowed[h] = 1;

    for (size_t id : byOut[h]) {
        const Edge& e = edges[id];
        if (isReady(e.a) && isReady(e.b)) {
            producedBy[h] = int(id);
            order.push_back(h);
            grow(order.size() - 1);
            return;
        }
    }
}

void HelperExchange::undo(size_t mark, size_t h) {
    while (order.size() > mark) {
        producedBy[order.back()] = NOT_READY;
        order.pop_back();
    }

    allowed[h] = 0;
}

std::vector<size_t> HelperExchange::collect(const std::vector<size_t>& previous, size_t from) {
    std::vector<size_t> result;
    stamp++;

    auto consider = [&](size_t o) {
        if (allowed[o] || isHelper[o] || seen[o] == stamp)
            return;

        seen[o] = stamp;
        for (size_t id : byOut[o]) {
            if (isReady(edges[id].a) && isReady(edges[id].b)) {
                result.push_back(o);
                return;
            }
        }
    };

    for (size_t o : previous)
        consider(o);

    for (size_t k = from; k < order.size(); k++)
        for (size_t id : byParent[order[k]])
            consider(edges[id].out);

    return result;
}

bool HelperExchange::search(size_t maxRemoved) {
    for (size_t r = 1; r <= maxRemoved; r++) {
        std::vector<size_t> removed;
        if (chooseRemoved(r, 0, removed))
            return true;
    }

    return false;
}

bool HelperExchange::chooseRemoved(size_t r, size_t start, std::vector<size_t>& removed) {
    if (removed.size() == r)
        return tryRemoved(removed);

    for (size_t i = start; i < helpers.size(); i++) {
        if (expired())
            return false;

        removed.push_back(helpers[i]);
        if (chooseRemoved(r, i + 1, removed))
            return true;
        removed.pop_back();
    }

    return false;
}

bool HelperExchange::tryRemoved(const std::vector<size_t>& removed) {
    reset(removed);
    if (done())
        return true;

    size_t cap = std::min(removed.size() - 1, MAX_ADDED);
    if (cap == 0)
        return false;

    std::vector<size_t> viable = collect(candidates, order.size());

    for (size_t budget = 1; budget <= cap; budget++)
        if (insert(budget, viable))
            return true;

    return false;
}

bool HelperExchange::insert(size_t budget, const std::vector<size_t>& viable) {
    if (budget == 0)
        return done();

    for (size_t h : viable) {
        if (expired())
            return false;

        size_t mark = order.size();
        add(h);

        std::vector<size_t> next;
        if (budget > 1)
            next = collect(viable, mark);

        if (insert(budget - 1, next))
            return true;

        undo(mark, h);
    }

    return false;
}

std::optional<Solution> HelperExchange::build(const Solution& old, const std::vector<Vector>& oldVectors) const {
    std::vector<char> needed(count, 0);
    std::vector<size_t> stack = required;

    while (!stack.empty()) {
        size_t o = stack.back();
        stack.pop_back();

        if (o < dimension || needed[o])
            continue;

        needed[o] = 1;
        const Edge& e = edges[producedBy[o]];
        stack.push_back(e.a);
        stack.push_back(e.b);
    }

    const size_t NONE = std::numeric_limits<size_t>::max();
    std::vector<size_t> index(count, NONE);
    for (size_t i = 0; i < dimension; i++)
        index[i] = i;

    Solution result{dimension, {}, {}};

    for (size_t o : order) {
        if (o < dimension || !needed[o])
            continue;

        const Edge& e = edges[producedBy[o]];
        index[o] = dimension + result.substitutions.size();
        result.substitutions.push_back({index[e.a], index[e.b], e.sa, e.sb});
    }

    for (const std::vector<Term>& expression : old.expressions) {
        std::vector<Term> terms;

        for (const Term& term : expression) {
            const Vector& vector = oldVectors[term.index];
            Vector canonized = vector.getCanonized();
            int sign = vector == canonized ? 1 : -1;

            std::optional<size_t> poolIndex = pool.find(canonized);
            if (!poolIndex || index[*poolIndex] == NONE)
                return std::nullopt;

            terms.push_back({index[*poolIndex], term.value * sign});
        }

        result.expressions.push_back(std::move(terms));
    }

    if (result.getAdditions() >= old.getAdditions())
        return std::nullopt;

    return result;
}

} // namespace leo
