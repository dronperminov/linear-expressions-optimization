#pragma once

#include <random>
#include "candidate_selector.h"


template <typename Candidate>
class GreedyAlternativeSelector : public CandidateSelector<Candidate> {
    std::mt19937& generator;
public:
    GreedyAlternativeSelector(std::mt19937& generator);

    Candidate select(const std::vector<Candidate>& candidates) const override;
};

template <typename Candidate>
GreedyAlternativeSelector<Candidate>::GreedyAlternativeSelector(std::mt19937& generator) : generator(generator) {
    
}

template <typename Candidate>
Candidate GreedyAlternativeSelector<Candidate>::select(const std::vector<Candidate>& candidates) const {
    double maxScore = candidates[0].score;

    for (size_t i = 1; i < candidates.size(); i++)
        maxScore = std::max(maxScore, candidates[i].score);

    std::vector<size_t> indices;
    for (size_t i = 0; i < candidates.size(); i++)
        if (candidates[i].score == maxScore)
            indices.push_back(i);

    return candidates[indices[generator() % indices.size()]];
}
