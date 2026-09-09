#pragma once

#include "candidate_selector.h"


template <typename Candidate>
class GreedySelector : public CandidateSelector<Candidate> {
public:
    Candidate select(const std::vector<Candidate>& candidates) const override;
};

template <typename Candidate>
Candidate GreedySelector<Candidate>::select(const std::vector<Candidate>& candidates) const {
    size_t imax = 0;

    for (size_t i = 1; i < candidates.size(); i++)
        if (candidates[i].score > candidates[imax].score)
            imax = i;

    return candidates[imax];
}
