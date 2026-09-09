#pragma once

#include <random>
#include "candidate_selector.h"


template <typename Candidate>
class GreedyRandomSelector : public CandidateSelector<Candidate> {
    std::mt19937& generator;
    double probability;
public:
    GreedyRandomSelector(std::mt19937& generator, double probability);

    Candidate select(const std::vector<Candidate>& candidates) const override;
};

template <typename Candidate>
GreedyRandomSelector<Candidate>::GreedyRandomSelector(std::mt19937& generator, double probability) : generator(generator) {
    this->probability = probability;
}

template <typename Candidate>
Candidate GreedyRandomSelector<Candidate>::select(const std::vector<Candidate>& candidates) const {
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    if (uniform(generator) < probability) {
        std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
        return candidates[dist(generator)];
    }

    double maxScore = candidates[0].score;
    for (size_t i = 1; i < candidates.size(); i++)
        maxScore = std::max(maxScore, candidates[i].score);

    std::vector<size_t> indices;
    for (size_t i = 0; i < candidates.size(); i++)
        if (candidates[i].score == maxScore)
            indices.push_back(i);

    std::uniform_int_distribution<size_t> dist(0, indices.size() - 1);
    return candidates[indices[dist(generator)]];
}
