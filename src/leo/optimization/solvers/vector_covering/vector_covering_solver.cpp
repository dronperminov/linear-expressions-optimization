#include <leo/optimization/solvers/vector_covering/vector_covering_solver.h>

#include <stdexcept>

namespace leo::vector_covering {

VectorCoveringSolver::VectorCoveringSolver(const std::vector<std::vector<int>>& expressions, const VectorCoveringParameters& parameters, std::shared_ptr<const VectorCoveringScorer> scorer, std::shared_ptr<const ScoreSelector> selector, uint32_t seed) : Solver(expressions), generator(seed) {
    setParameters(parameters);
    setScorer(scorer);
    setSelector(selector);

    for (const std::vector<int>& expression : expressions) {
        Vector target(expression);
        if (target.getSupport() < 2)
            continue;

        target.canonize();
        targets.insert(target);
    }

    naiveComplexity = 0;
    for (const Vector& target : targets)
        naiveComplexity += target.getSupport() - 1;
}

void VectorCoveringSolver::setParameters(const VectorCoveringParameters& parameters) {
    this->parameters = parameters;
}

void VectorCoveringSolver::setScorer(std::shared_ptr<const VectorCoveringScorer> scorer) {
    if (!scorer)
        throw std::invalid_argument("VectorCoveringSolver::setScorer: scorer must not be null");

    this->scorer = std::move(scorer);
}

void VectorCoveringSolver::setSelector(std::shared_ptr<const ScoreSelector> selector) {
    if (!selector)
        throw std::invalid_argument("VectorCoveringSolver::setSelector: selector must not be null");

    this->selector = std::move(selector);
}

size_t VectorCoveringSolver::solve() {
    initialize();

    while (!uncovered.empty() && (!parameters.naiveFallback || steps.size() <= naiveComplexity)) {
        scorer->score(candidates, {uncovered, vectors}, scores);
        size_t index = selector->selectIndex(scores, generator);
        addCandidate(candidates[index]);

        if (!uncovered.empty()) {
            updateCandidates();
            candidates[index] = candidates.back();
            candidates.pop_back();
        }
    }

    if (!uncovered.empty()) {
        fallbackToNaive();
    }
    else if (parameters.removeUnused) {
        removeUnused();
    }

    candidates.clear();
    unique.clear();
    solved = true;
    return steps.size();
}

Solution VectorCoveringSolver::getSolution() const {
    if (!solved)
        throw std::runtime_error("VectorCoveringSolver::getSolution: solution is not available yet, call solve() first");

    Solution solution;
    solution.dimension = dimension;
    solution.substitutions = steps;

    for (const std::vector<int>& expression : expressions) {
        Vector vector(expression);

        if (vector.isZero()) {
            solution.expressions.push_back({});
            continue;
        }

        if (vector.getSupport() == 1) {
            size_t index = vector.getNonZeroIndex();
            solution.expressions.push_back({{index, vector[index]}});
            continue;
        }

        size_t index = pool.at(vector.getCanonized());
        int value = vectors[index].compare(vector);
        solution.expressions.push_back({{index, value}});
    }

    return solution;
}

void VectorCoveringSolver::initialize() {
    uncovered.clear();
    pool.clear();
    vectors.clear();
    steps.clear();

    for (const Vector& target : targets)
        uncovered.insert(target);

    for (size_t i = 0; i < dimension; i++) {
        Vector basis(dimension, i);
        pool[basis] = i;
        vectors.push_back(basis);
    }

    initializeCandidates();
}

void VectorCoveringSolver::initializeCandidates() {
    candidates.clear();
    unique.clear();

    for (size_t i = 0; i < vectors.size(); i++) {
        for (size_t j = i + 1; j < vectors.size(); j++) {
            for (int sign : {1, -1}) {
                Vector vector = vectors[i].addScaled(vectors[j], sign);
                Vector canonized = vector.getCanonized();

                unique.insert(canonized);
                candidates.push_back({{i, j, 1, sign}, vector, canonized});
            }
        }
    }
}

void VectorCoveringSolver::updateCandidates() {
    size_t j = vectors.size() - 1;

    for (size_t i = 0; i < j; i++) {
        for (int sign : {1, -1}) {
            Vector vector = vectors[i].addScaled(vectors.back(), sign);
            Vector canonized = vector.getCanonized();

            if (pool.find(canonized) != pool.end())
                continue;

            if (unique.find(canonized) != unique.end())
                continue;

            if (parameters.maxAbsValue > 0 && vector.getMaxAbs() > parameters.maxAbsValue)
                continue;

            unique.insert(canonized);
            candidates.push_back({{i, j, 1, sign}, vector, canonized});
        }
    }
}

void VectorCoveringSolver::addCandidate(const Candidate& candidate) {
    pool[candidate.canonized] = pool.size();
    vectors.push_back(candidate.vector);
    steps.push_back(candidate.step);
    uncovered.erase(candidate.canonized);
}

void VectorCoveringSolver::fallbackToNaive() {
    initialize();

    while (!uncovered.empty()) {
        const Vector& target = *uncovered.begin();
        std::vector<size_t> indices = target.getNonZeroIndices();

        size_t i = indices[0];
        int ai = target[indices[0]];

        for (size_t index = 1; index < indices.size(); index++) {
            size_t j = indices[index];
            int aj = target[j];

            Vector vector = vectors[i] * ai + vectors[j] * aj;
            Vector canonized = vector.getCanonized();

            auto result = pool.find(canonized);
            if (result == pool.end()) {
                addCandidate({{i, j, ai, aj}, vector, canonized});
                i = vectors.size() - 1;
            }
            else {
                i = result->second;
            }

            ai = 1;
        }
    }
}

void VectorCoveringSolver::removeUnused() {
    std::vector<bool> used(vectors.size(), false);

    for (size_t i = 0; i < steps.size(); i++) {
        size_t index = steps.size() - 1 - i;

        if (targets.find(vectors[dimension + index].getCanonized()) != targets.end())
            used[dimension + index] = true;

        if (used[dimension + index]) {
            used[steps[index].i] = true;
            used[steps[index].j] = true;
        }
    }

    std::vector<size_t> indices(vectors.size());
    for (size_t i = 0; i < indices.size(); i++)
        indices[i] = i;

    size_t j = 0;
    for (size_t i = 0; i < steps.size(); i++) {
        if (!used[dimension + i])
            continue;

        indices[dimension + i] = dimension + j;
        steps[i].i = indices[steps[i].i];
        steps[i].j = indices[steps[i].j];

        if (i != j) {
            steps[j] = steps[i];
            vectors[dimension + j] = vectors[dimension + i];
        }

        j++;
    }

    steps.erase(steps.begin() + j, steps.end());
    vectors.erase(vectors.begin() + dimension + j, vectors.end());
    pool.clear();

    for (size_t i = 0; i < vectors.size(); i++)
        pool[vectors[i].getCanonized()] = i;
}

} // namespace leo::vector_covering
