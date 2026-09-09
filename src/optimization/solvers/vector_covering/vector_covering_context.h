#pragma once

#include <vector>
#include <unordered_set>

#include "../../../entities/vector.h"

struct VectorCoveringContext {
    std::unordered_set<Vector> uncovered;
    std::vector<Vector> vectors;
};
