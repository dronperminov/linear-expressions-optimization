#pragma once

#include <cstdint>
#include <unordered_set>
#include <vector>

#include <leo/entities/vector.h>

namespace leo::vector_covering {

struct Context {
    const std::unordered_set<Vector>& uncovered;
    const std::vector<Vector>& vectors;
    const std::vector<uint64_t>& fingerprints;
};

} // namespace leo::vector_covering
