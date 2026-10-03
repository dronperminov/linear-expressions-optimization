#include <leo/optimization/selection/score_selector.h>

namespace leo {

double ScoreSelector::getMaxScore(const std::vector<double>& scores) const {
    double maxScore = scores[0];

    for (size_t i = 1; i < scores.size(); i++)
        maxScore = std::max(maxScore, scores[i]);

    return maxScore;
}

std::vector<size_t> ScoreSelector::getMaxScoreIndices(const std::vector<double>& scores) const {
    double maxScore = getMaxScore(scores);

    std::vector<size_t> indices;
    for (size_t i = 0; i < scores.size(); i++)
        if (scores[i] == maxScore)
            indices.push_back(i);

    return indices;
}

} // namespace leo
