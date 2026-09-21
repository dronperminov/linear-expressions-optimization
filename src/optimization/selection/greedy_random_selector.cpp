#include <leo/optimization/selection/greedy_random_selector.h>

namespace leo {

GreedyRandomSelector::GreedyRandomSelector(std::mt19937& generator, double randomProbability) : generator(generator) {
    setRandomProbability(randomProbability);
}

void GreedyRandomSelector::setRandomProbability(double randomProbability) {
    if (randomProbability < 0 || randomProbability > 1)
        throw std::invalid_argument("GreedyRandomSelector::setRandomProbability: probability must be in range [0, 1], got " + std::to_string(randomProbability));

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
