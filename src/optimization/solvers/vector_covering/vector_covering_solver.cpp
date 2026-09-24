#include <leo/optimization/solvers/vector_covering/vector_covering_solver.h>

namespace leo::vector_covering {

VectorCoveringSolver::VectorCoveringSolver(const std::vector<std::vector<int>>& expressions, const VectorCoveringParameters& parameters, const VectorCoveringScorer& scorer, const ScoreSelector& selector) : Solver(expressions) {
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

void VectorCoveringSolver::setScorer(const VectorCoveringScorer& scorer) {
    this->scorer = &scorer;
}

void VectorCoveringSolver::setSelector(const ScoreSelector& selector) {
    this->selector = &selector;
}

size_t VectorCoveringSolver::solve() {
    initialize();

    while (!uncovered.empty() && (!parameters.naiveFallback || steps.size() <= naiveComplexity)) {
        std::vector<Candidate> candidates = getCandidates();
        scorer->score(candidates, {uncovered, vectors}, scores);
        Candidate candidate = candidates[selector->selectIndex(scores)];
        addCandidate(candidate);
    }

    if (!uncovered.empty()) {
        fallbackToNaive();
    }
    else if (parameters.removeUnused) {
        removeUnused();
    }

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
}

std::vector<Candidate> VectorCoveringSolver::getCandidates() const {
    std::vector<Candidate> candidates;
    std::unordered_set<Vector> unique;

    for (size_t i = 0; i < vectors.size(); i++) {
        for (size_t j = i + 1; j < vectors.size(); j++) {
            for (int sign : {1, -1}) {
                Vector vector = vectors[i].addScaled(vectors[j], sign);
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

    return candidates;
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
