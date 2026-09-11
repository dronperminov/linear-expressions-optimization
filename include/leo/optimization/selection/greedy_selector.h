#pragma once

#include <leo/optimization/selection/score_selector.h>

namespace leo {

class GreedySelector : public ScoreSelector {
public:
    size_t selectIndex(const std::vector<double>& scores) const override;
};

} // namespace leo
