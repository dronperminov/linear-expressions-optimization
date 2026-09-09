#pragma once

#include <vector>


template <typename Candidate>
class CandidateSelector {
public:
    virtual Candidate select(const std::vector<Candidate>& candidates) const = 0;

    virtual ~CandidateSelector() = default;
};
