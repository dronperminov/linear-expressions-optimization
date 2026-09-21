#include <leo/optimization/solvers/vector_covering/scorers/default_scorer.h>

namespace leo::vector_covering {

DefaultScorer::DefaultScorer() {
    coverWeight = 1000.0;
    oneStepWeight = 100.0;
    hammingWeight = 0.0;
    matchesWeight = 1.0;
    distanceWeight = 0.0;
}

DefaultScorer::DefaultScorer(double coverWeight, double oneStepWeight, double hammingWeight, double matchesWeight, double distanceWeight) {
    this->coverWeight = coverWeight;
    this->oneStepWeight = oneStepWeight;
    this->hammingWeight = hammingWeight;
    this->matchesWeight = matchesWeight;
    this->distanceWeight = distanceWeight;
}

void DefaultScorer::score(const std::vector<Candidate>& candidates, const Context& context, std::vector<double>& scores) const {
    scores.resize(candidates.size());

    for (size_t i = 0; i < candidates.size(); i++) {
        double score = 0.0;

        const Vector& canonized = candidates[i].canonized;
        for (const Vector& target : context.uncovered) {
            if (canonized == target) {
                score += coverWeight;
                continue;
            }

            size_t hamming = canonized.getDimension() - canonized.getHammingDistance(target);
            size_t matches = canonized.getMatchesCount(target);
            int distance = canonized.getDistance(target);

            score += hamming * hammingWeight;
            score += matches * matchesWeight;
            score += distance * distanceWeight;
        }

        scores[i] = score;
    }

    if (oneStepWeight != 0)
        addOneStepScores(candidates, context, scores);
}

void DefaultScorer::addOneStepScores(const std::vector<Candidate>& candidates, const Context& context, std::vector<double>& scores) const {
    std::unordered_map<Vector, size_t> vector2index;
    for (size_t i = 0; i < candidates.size(); i++)
        vector2index[candidates[i].canonized] = i;

    for (const Vector& target : context.uncovered) {
        std::unordered_set<size_t> indices;

        for (const Vector& vector : context.vectors) {
            auto sub = vector2index.find((target - vector).getCanonized());
            if (sub != vector2index.end() && candidates[sub->second].canonized != target)
                indices.insert(sub->second);

            auto add = vector2index.find((target + vector).getCanonized());
            if (add != vector2index.end() && candidates[add->second].canonized != target)
                indices.insert(add->second);
        }

        for (size_t index : indices)
            scores[index] += oneStepWeight;
    }
}

} // namespace leo::vector_covering
