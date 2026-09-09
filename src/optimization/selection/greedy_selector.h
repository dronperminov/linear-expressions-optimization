#pragma once

#include "score_selector.h"

class GreedySelector : public ScoreSelector {
public:
    size_t selectIndex(const std::vector<double>& scores) const override;
};
