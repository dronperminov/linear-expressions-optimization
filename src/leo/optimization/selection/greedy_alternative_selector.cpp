#include <leo/optimization/selection/greedy_alternative_selector.h>

namespace leo {

size_t GreedyAlternativeSelector::selectIndex(const std::vector<double>& scores, std::mt19937& generator) const {
    std::vector<size_t> indices = getMaxScoreIndices(scores);
    std::uniform_int_distribution<size_t> dist(0, indices.size() - 1);
    return indices[dist(generator)];
}

} // namespace leo
