#pragma once

namespace leo::vector_covering {

struct Parameters {
    int maxAbsValue;
    bool naiveFallback = true;
    bool removeUnused = true;
    bool addTargetPairs = false;
    double maxTimeInSeconds = 0.0;
};

} // namespace leo::vector_covering
