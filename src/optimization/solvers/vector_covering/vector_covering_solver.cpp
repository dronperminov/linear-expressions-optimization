#include "vector_covering_solver.h"

VectorCoveringSolver::VectorCoveringSolver(const std::vector<std::vector<int>>& expressions, const VectorCoveringParameters& parameters, const VectorCoveringScorer& scorer, const ScoreSelector &selector) {
    this->dimension = expressions[0].size();
    this->count = expressions.size();

    setParameters(parameters);
    setScorer(scorer);
    setSelector(selector);

    for (const std::vector<int>& expression : expressions) {
        this->expressions.push_back(Vector(expression));
        this->targets.insert(Vector(expression));
    }
}

void VectorCoveringSolver::setParameters(const VectorCoveringParameters& parameters) {
    this->parameters = parameters;
}

void VectorCoveringSolver::setScorer(const VectorCoveringScorer& scorer) {
    this->scorer = &scorer;
}

void VectorCoveringSolver::setSelector(const ScoreSelector &selector) {
    this->selector = &selector;
}

int VectorCoveringSolver::solve() {
    initialize();

    while (!uncovered.empty()) {
        std::vector<VectorCoveringCandidate> candidates = getCandidates();
        scorer->score(candidates, {uncovered, vectors}, scores);
        VectorCoveringCandidate candidate = candidates[selector->selectIndex(scores)];
        addCandidate(candidate);
    }

    return steps.size();
}

Solution VectorCoveringSolver::getSolution() const {
    Solution solution;
    solution.substitutions = steps;
    solution.expressions = {}; // TODO
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

    for (int i = 0; i < dimension; i++) {
        Vector basis(dimension, i);
        pool.insert(basis);
        vectors.push_back(basis);
    }
}

std::vector<VectorCoveringCandidate> VectorCoveringSolver::getCandidates() const {
    std::vector<VectorCoveringCandidate> candidates;
    std::unordered_set<Vector> unique;

    for (size_t i = 0; i < vectors.size(); i++) {
        for (size_t j = i + 1; j < vectors.size(); j++) {
            std::vector<VectorCoveringCandidate> vs = {
                {{i, j, 1, 1}, vectors[i] + vectors[j]},
                {{i, j, 1, -1}, vectors[i] - vectors[j]}
            };

            for (const VectorCoveringCandidate& candidate : vs) {
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

void VectorCoveringSolver::addCandidate(const VectorCoveringCandidate& candidate) {
    pool.insert(candidate.vector);
    vectors.push_back(candidate.vector);
    steps.push_back(candidate.step);
    uncovered.erase(candidate.vector);
}
