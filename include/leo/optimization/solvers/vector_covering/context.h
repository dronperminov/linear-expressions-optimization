#pragma once

#include <unordered_set>
#include <vector>

#include <leo/entities/vector.h>

namespace leo::vector_covering {

struct Context {
    const std::unordered_set<Vector>& uncovered;
    const std::vector<Vector>& vectors;
};

} // namespace leo::vector_covering
