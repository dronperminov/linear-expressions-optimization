#include "greedy_random_selector.h"

GreedyRandomSelector::GreedyRandomSelector(std::mt19937& generator, double probability) : generator(generator) {
    this->probability = probability;
}

size_t GreedyRandomSelector::selectIndex(const std::vector<double>& scores) const {
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    if (uniform(generator) < probability) {
        std::vector<size_t> indices = getMaxScoreIndices(scores);
        std::uniform_int_distribution<size_t> dist(0, indices.size() - 1);
        return indices[dist(generator)];
    }

    std::uniform_int_distribution<size_t> dist(0, scores.size() - 1);
    return dist(generator);
}
