#include "greedy_alternative_selector.h"

GreedyAlternativeSelector::GreedyAlternativeSelector(std::mt19937& generator) : generator(generator) {

}

size_t GreedyAlternativeSelector::selectIndex(const std::vector<double>& scores) const {
    std::vector<size_t> indices = getMaxScoreIndices(scores);
    std::uniform_int_distribution<size_t> dist(0, indices.size() - 1);
    return indices[dist(generator)];
}
