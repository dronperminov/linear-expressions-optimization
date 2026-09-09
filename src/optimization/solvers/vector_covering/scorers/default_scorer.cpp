#include "default_scorer.h"


VectorCoveringDefaultScorer::VectorCoveringDefaultScorer() {
    coverWeight = 1000.0;
    oneStepWeight = 100.0;
    hammingWeight = 0.0;
    matchesWeight = 1.0;
}

VectorCoveringDefaultScorer::VectorCoveringDefaultScorer(double coverWeight, double oneStepWeight, double hammingWeight, double matchesWeight) {
    this->coverWeight = coverWeight;
    this->oneStepWeight = oneStepWeight;
    this->hammingWeight = hammingWeight;
    this->matchesWeight = matchesWeight;
}

void VectorCoveringDefaultScorer::score(const std::vector<VectorCoveringCandidate>& candidates, const VectorCoveringContext& context, std::vector<double>& scores) const {
    scores.resize(candidates.size());

    for (size_t i = 0; i < candidates.size(); i++) {
        double score = 0.0;

        for (const Vector& target : context.uncovered) {
            if (candidates[i].vector.compare(target)) {
                score += coverWeight;
                continue;
            }

            int hamming = candidates[i].vector.getDimension() - candidates[i].vector.getHammingDistance(target);
            int matches = candidates[i].vector.getMatchesCount(target);

            score += hamming * hammingWeight;
            score += matches * matchesWeight;
        }

        scores[i] = score;
    }

    if (oneStepWeight != 0)
        addOneStepScores(candidates, context, scores);
}

void VectorCoveringDefaultScorer::addOneStepScores(const std::vector<VectorCoveringCandidate>& candidates, const VectorCoveringContext& context, std::vector<double>& scores) const {
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
            scores[index] += oneStepWeight;
    }
}
