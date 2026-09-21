#include <leo/optimization/selection/greedy_selector.h>

namespace leo {

size_t GreedySelector::selectIndex(const std::vector<double>& scores) const {
    size_t imax = 0;

    for (size_t i = 1; i < scores.size(); i++)
        if (scores[i] > scores[imax])
            imax = i;

    return imax;
}

} // namespace leo
