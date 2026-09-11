#pragma once

#include <random>

#include <leo/optimization/selection/score_selector.h>

namespace leo {

class GreedyRandomSelector : public ScoreSelector {
    std::mt19937& generator;
    double probability;
public:
    GreedyRandomSelector(std::mt19937& generator, double probability);

    size_t selectIndex(const std::vector<double>& scores) const override;
};

} // namespace leo
