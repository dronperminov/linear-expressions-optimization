#include <leo/optimization/solvers/vector_covering/vector_covering_solver.h>

namespace leo::vector_covering {

VectorCoveringSolver::VectorCoveringSolver(const std::vector<std::vector<int>>& expressions, const VectorCoveringParameters& parameters, const VectorCoveringScorer& scorer, const ScoreSelector& selector) : Solver(expressions) {
    setParameters(parameters);
    setScorer(scorer);
    setSelector(selector);

    for (const std::vector<int>& expression : expressions)
        targets.insert(Vector(expression));

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

    std::unordered_map<Vector, size_t> vector2index;
    for (size_t i = 0; i < vectors.size(); i++)
        vector2index[vectors[i]] = i;

    for (size_t i = 0; i < expressions.size(); i++) {
        Vector expression(expressions[i]);

        size_t index = vector2index.at(expression);
        int value = vectors[index].compare(expression);
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
        if (target.getSupport() > 1)
            uncovered.insert(target);

    for (size_t i = 0; i < dimension; i++) {
        Vector basis(dimension, i);
        pool.insert(basis);
        vectors.push_back(basis);
    }
}

std::vector<Candidate> VectorCoveringSolver::getCandidates() const {
    std::vector<Candidate> candidates;
    std::unordered_set<Vector> unique;

    for (size_t i = 0; i < vectors.size(); i++) {
        for (size_t j = i + 1; j < vectors.size(); j++) {
            std::vector<Candidate> vs = {
                {{i, j, 1, 1}, vectors[i] + vectors[j]},
                {{i, j, 1, -1}, vectors[i] - vectors[j]}
            };

            for (const Candidate& candidate : vs) {
                if (pool.find(candidate.vector) != pool.end())
                    continue;

                if (unique.find(candidate.vector) != unique.end())
                    continue;

                if (parameters.maxAbsValue > 0 && candidate.vector.getMaxAbs() > parameters.maxAbsValue)
                    continue;

                unique.insert(candidate.vector);
                candidates.emplace_back(candidate);
            }
        }
    }

    return candidates;
}

void VectorCoveringSolver::addCandidate(const Candidate& candidate) {
    pool.insert(candidate.vector);
    vectors.push_back(candidate.vector);
    steps.push_back(candidate.step);
    uncovered.erase(candidate.vector);
}

void VectorCoveringSolver::fallbackToNaive() {
    initialize();

    while (!uncovered.empty()) {
        const Vector& target = *uncovered.begin();
        std::vector<size_t> indices;
        for (size_t i = 0; i < dimension; i++)
            if (target[i])
                indices.push_back(i);

        size_t i = indices[0];
        size_t j = indices[1];

        vectors.push_back(vectors[i] * target[i] + vectors[j] * target[j]);
        steps.push_back({i, j, target[i], target[j]});
        uncovered.erase(vectors.back());

        for (size_t k = 2; k < indices.size(); k++) {
            steps.push_back({vectors.size() - 1, indices[k], 1, target[indices[k]]});
            vectors.push_back(vectors.back() + vectors[indices[k]] * target[indices[k]]);
            uncovered.erase(vectors.back());
        }
    }
}

void VectorCoveringSolver::removeUnused() {
    std::vector<bool> used(vectors.size(), false);

    for (size_t i = 0; i < steps.size(); i++) {
        size_t index = steps.size() - 1 - i;

        if (targets.find(vectors[dimension + index]) != targets.end())
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
}

} // namespace leo::vector_covering
