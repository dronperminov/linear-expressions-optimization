#pragma once

namespace leo::vector_covering {

struct VectorCoveringParameters {
    int maxAbsValue;
    bool naiveFallback;
    bool removeUnused;
};

} // namespace leo::vector_covering
