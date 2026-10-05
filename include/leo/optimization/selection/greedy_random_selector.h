#pragma once

#include <random>
#include <vector>

#include <leo/optimization/selection/score_selector.h>

namespace leo {

class GreedyRandomSelector : public ScoreSelector {
    double randomProbability;
public:
    GreedyRandomSelector(double randomProbability);

    void setRandomProbability(double randomProbability);

    size_t selectIndex(const std::vector<double>& scores, std::mt19937& generator) const override;
};

} // namespace leo
