#pragma once

#include <random>
#include "score_selector.h"

class GreedyRandomSelector : public ScoreSelector {
    std::mt19937& generator;
    double probability;
public:
    GreedyRandomSelector(std::mt19937& generator, double probability);

    size_t selectIndex(const std::vector<double>& scores) const override;
};
