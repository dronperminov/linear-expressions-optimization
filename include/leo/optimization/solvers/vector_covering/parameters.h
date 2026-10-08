#pragma once

namespace leo::vector_covering {

struct Parameters {
    int maxAbsValue;
    bool naiveFallback = true;
    bool removeUnused = true;
    bool addTargetPairs = false;
};

} // namespace leo::vector_covering
