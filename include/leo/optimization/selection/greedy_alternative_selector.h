#pragma once

#include <random>

#include <leo/optimization/selection/score_selector.h>

namespace leo {

class GreedyAlternativeSelector : public ScoreSelector {
    std::mt19937& generator;
public:
    GreedyAlternativeSelector(std::mt19937& generator);

    size_t selectIndex(const std::vector<double>& scores) const override;
};

} // namespace leo
