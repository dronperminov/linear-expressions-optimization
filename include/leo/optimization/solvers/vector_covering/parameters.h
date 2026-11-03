#pragma once

namespace leo::vec {

struct Parameters {
    int maxAbsValue;
    bool naiveFallback = true;
    bool removeUnused = true;
    bool ignoreUnusedInBound = true;
    bool addTargetPairs = false;
    double maxTimeInSeconds = 0.0;
};

} // namespace leo::vec
