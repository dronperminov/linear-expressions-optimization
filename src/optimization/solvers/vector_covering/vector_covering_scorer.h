#pragma once

#include <vector>

#include "vector_covering_context.h"
#include "vector_covering_candidate.h"


class VectorCoveringScorer {
public:
    virtual void score(const std::vector<VectorCoveringCandidate>& candidates, const VectorCoveringContext& context, std::vector<double>& scores) const = 0;

    virtual ~VectorCoveringScorer() = default;
};
