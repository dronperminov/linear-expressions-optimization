#pragma once

#include <vector>

#include <leo/optimization/solvers/vector_covering/candidate.h>
#include <leo/optimization/solvers/vector_covering/context.h>

namespace leo::vector_covering {

class VectorCoveringScorer {
public:
    virtual void score(const std::vector<Candidate>& candidates, const Context& context, std::vector<double>& scores) const = 0;

    virtual ~VectorCoveringScorer() = default;
};

} // namespace leo::vector_covering
