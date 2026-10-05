#pragma once

#include <cstddef>
#include <random>
#include <vector>

#include <leo/optimization/selection/score_selector.h>

namespace leo {

class WeightedRandomSelector : public ScoreSelector {
public:
    size_t selectIndex(const std::vector<double>& scores, std::mt19937& generator) const override;
};

} // namespace leo
