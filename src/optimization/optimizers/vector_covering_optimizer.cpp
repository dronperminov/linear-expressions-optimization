#include "vector_covering_optimizer.h"

VectorCoveringScorer::VectorCoveringScorer() {
    coverWeight = 1000.0;
    oneStepWeight = 100.0;
    hammingWeight = 0.0;
    matchesWeight = 1.0;
}

VectorCoveringScorer::VectorCoveringScorer(double coverWeight, double oneStepWeight, double hammingWeight, double matchesWeight) {
    this->coverWeight = coverWeight;
    this->oneStepWeight = oneStepWeight;
    this->hammingWeight = hammingWeight;
    this->matchesWeight = matchesWeight;
}

void VectorCoveringScorer::score(std::vector<Candidate>& candidates, const VectorCoveringContext& context) const {
    for (Candidate& candidate : candidates) {
        candidate.score = 0.0;

        for (const Vector& target : context.uncovered) {
            if (candidate.vector.compare(target)) {
                candidate.score += coverWeight;
                continue;
            }

            int hamming = candidate.vector.getDimension() - candidate.vector.getHammingDistance(target);
            int matches = candidate.vector.getMatchesCount(target);

            candidate.score += hamming * hammingWeight;
            candidate.score += matches * matchesWeight;
        }
    }

    if (oneStepWeight != 0)
        addOneStepScores(candidates, context);
}

void VectorCoveringScorer::addOneStepScores(std::vector<Candidate>& candidates, const VectorCoveringContext& context) const {
    std::unordered_map<Vector, size_t> vector2index;

    for (size_t i = 0; i < candidates.size(); i++)
        vector2index[candidates[i].vector] = i;

    for (const Vector& target : context.uncovered) {
        std::unordered_set<int> indices;

        for (const Vector& vector : context.vectors) {
            auto sub = vector2index.find(target - vector);
            if (sub != vector2index.end() && candidates[sub->second].vector != target)
                indices.insert(sub->second);

            auto add = vector2index.find(target + vector);
            if (add != vector2index.end() && candidates[add->second].vector != target)
                indices.insert(add->second);
        }

        for (int index : indices)
            candidates[index].score += oneStepWeight;
    }
}

VectorCoveringOptimizer::VectorCoveringOptimizer(const std::vector<std::vector<int>>& expressions, const VectorCoveringParameters& parameters, const VectorCoveringScorer& scorer, const CandidateSelector<Candidate> &selector) {
    this->dimension = expressions[0].size();
    this->count = expressions.size();

    setParameters(parameters);
    setScorer(scorer);
    setSelector(selector);

    for (const std::vector<int> expression : expressions) {
        this->expressions.push_back(Vector(expression));
        this->targets.insert(Vector(expression));
    }
}

void VectorCoveringOptimizer::setParameters(const VectorCoveringParameters& parameters) {
    this->parameters = parameters;
}

void VectorCoveringOptimizer::setScorer(const VectorCoveringScorer& scorer) {
    this->scorer = scorer;
}

void VectorCoveringOptimizer::setSelector(const CandidateSelector<Candidate> &selector) {
    this->selector = &selector;
}

int VectorCoveringOptimizer::optimize() {
    initialize();

    while (!uncovered.empty()) {
        std::vector<Candidate> candidates = getCandidates();
        scorer.score(candidates, {uncovered, vectors});
        Candidate candidate = selector->select(candidates);
        addCandidate(candidate);
    }

    return steps.size();
}

Solution VectorCoveringOptimizer::getSolution() const {
    Solution solution;
    solution.substitutions = steps;
    solution.expressions = {}; // TODO
    return solution;
}

void VectorCoveringOptimizer::initialize() {
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

std::vector<Candidate> VectorCoveringOptimizer::getCandidates() const {
    std::vector<Candidate> candidates;
    std::unordered_set<Vector> unique;

    for (size_t i = 0; i < vectors.size(); i++) {
        for (size_t j = i + 1; j < vectors.size(); j++) {
            std::vector<Candidate> vs = {
                {{i, j, 1, 1}, vectors[i] + vectors[j], 0.0},
                {{i, j, 1, -1}, vectors[i] - vectors[j], 0.0}
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

void VectorCoveringOptimizer::addCandidate(const Candidate& candidate) {
    pool.insert(candidate.vector);
    vectors.push_back(candidate.vector);
    steps.push_back(candidate.step);
    uncovered.erase(candidate.vector);
}
