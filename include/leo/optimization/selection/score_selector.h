#pragma once

#include <vector>

namespace leo {

class ScoreSelector {
public:
    virtual size_t selectIndex(const std::vector<double>& scores) const = 0;

    virtual ~ScoreSelector() = default;
protected:
    double getMaxScore(const std::vector<double>& scores) const;
    std::vector<size_t> getMaxScoreIndices(const std::vector<double>& scores) const;
};

} // namespace leo
