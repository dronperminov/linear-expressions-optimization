#include <leo/optimization/selection/weighted_random_selector.h>

namespace leo {

size_t WeightedRandomSelector::selectIndex(const std::vector<double>& scores, std::mt19937& generator) const {
    std::discrete_distribution<size_t> distribution(scores.begin(), scores.end());
    return distribution(generator);
}

} // namespace leo
