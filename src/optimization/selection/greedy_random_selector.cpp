#include <leo/optimization/selection/greedy_random_selector.h>

namespace leo {

GreedyRandomSelector::GreedyRandomSelector(std::mt19937& generator, double randomProbability) : generator(generator) {
    setRandomProbability(randomProbability);
}

void GreedyRandomSelector::setRandomProbability(double randomProbability) {
    this->randomProbability = randomProbability;
}

size_t GreedyRandomSelector::selectIndex(const std::vector<double>& scores) const {
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    if (uniform(generator) < randomProbability) {
        std::uniform_int_distribution<size_t> dist(0, scores.size() - 1);
        return dist(generator);
    }

    std::vector<size_t> indices = getMaxScoreIndices(scores);
    std::uniform_int_distribution<size_t> dist(0, indices.size() - 1);
    return indices[dist(generator)];
}

} // namespace leo
