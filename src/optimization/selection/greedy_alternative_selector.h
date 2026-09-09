#pragma once

#include <random>
#include "score_selector.h"

class GreedyAlternativeSelector : public ScoreSelector {
    std::mt19937& generator;
public:
    GreedyAlternativeSelector(std::mt19937& generator);

    size_t selectIndex(const std::vector<double>& scores) const override;
};
