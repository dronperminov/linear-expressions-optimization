#pragma once

#include <random>
#include <stdexcept>

#include <leo/optimization/selection/score_selector.h>

namespace leo {

class GreedyRandomSelector : public ScoreSelector {
    std::mt19937& generator;
    double randomProbability;
public:
    GreedyRandomSelector(std::mt19937& generator, double randomProbability);

    void setRandomProbability(double randomProbability);

    size_t selectIndex(const std::vector<double>& scores) const override;
};

} // namespace leo
