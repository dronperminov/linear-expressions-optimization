#include <leo/optimization/strategy_presets.h>

#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <leo/optimization/selection/greedy_alternative_selector.h>
#include <leo/optimization/solvers/cse/common_subexpression_solver.h>
#include <leo/optimization/solvers/cse/scorers/default_scorer.h>
#include <leo/optimization/solvers/cse/scorers/potential_scorer.h>
#include <leo/optimization/solvers/vector_covering/scorers/default_scorer.h>
#include <leo/optimization/solvers/vector_covering/vector_covering_parameters.h>
#include <leo/optimization/solvers/vector_covering/vector_covering_solver.h>

namespace leo::presets {

StrategyPool vectorCoveringDefault(const ExpressionsSystem& expressionsSystem, bool addTargetPairs) {
    StrategyPool strategies;

    if (expressionsSystem.isHorizontal())
        return strategies;

    vector_covering::VectorCoveringParameters parameters = {expressionsSystem.getMaxAbsValue(), true, true, addTargetPairs};
    auto selector = std::make_shared<GreedyAlternativeSelector>();

    std::vector<std::shared_ptr<const vector_covering::VectorCoveringScorer>> scorers = {
        std::make_shared<vector_covering::DefaultScorer>(),
        std::make_shared<vector_covering::DefaultScorer>(1000,   100,   0,   0, 0,  0),
        std::make_shared<vector_covering::DefaultScorer>(1000,   100,   0,   0, 0,  5),
        std::make_shared<vector_covering::DefaultScorer>(1000,   100,   0, 0.1, 0,  0),
        std::make_shared<vector_covering::DefaultScorer>(10000,  300,   0,   1, 2,  5),
        std::make_shared<vector_covering::DefaultScorer>(10000,  300,   0,   1, 5, 50),
        std::make_shared<vector_covering::DefaultScorer>(10000, 1000,   0, 0.1, 0,  5),
        std::make_shared<vector_covering::DefaultScorer>(10000, 1000, 0.1,   1, 1,  0),
        std::make_shared<vector_covering::DefaultScorer>(10000,  300, 0.1, 0.1, 0,  0)
    };

    for (size_t i = 0; i < scorers.size(); i++) {
        strategies.add("vec/" + std::to_string(i + 1), [scorer = scorers[i], parameters, selector](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
            return std::make_unique<vector_covering::VectorCoveringSolver>(expressions, parameters, scorer, selector, seed);
        });
    }

    strategies.add("vec/rnd", [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
        std::mt19937 generator(seed);
        std::vector<double> coverWeights = {10000, 1000};
        std::vector<double> oneStepWeights = {1000, 500, 300, 100};
        std::vector<double> hammingWeights = {0.0, 0.1, 1.0};
        std::vector<double> matchesWeights = {0.0, 0.1, 1.0};
        std::vector<double> distanceWeights = {0.0, 0.1, 1.0, 2.0, 5.0, 10.0};
        std::vector<double> savingsWeights = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 10.0, 25.0, 50.0};

        double cover = coverWeights[generator() % coverWeights.size()];
        double oneStep = oneStepWeights[generator() % oneStepWeights.size()];
        double hamming = hammingWeights[generator() % hammingWeights.size()];
        double matches = matchesWeights[generator() % matchesWeights.size()];
        double distance = distanceWeights[generator() % distanceWeights.size()];
        double savings = savingsWeights[generator() % savingsWeights.size()];

        auto scorer = std::make_shared<vector_covering::DefaultScorer>(cover, oneStep, hamming, matches, distance, savings);
        return std::make_unique<vector_covering::VectorCoveringSolver>(expressions, parameters, scorer, selector, seed);
    });

    return strategies;
}

StrategyPool cseDefault(const ExpressionsSystem& expressionsSystem) {
    StrategyPool strategies;

    auto selector = std::make_shared<GreedyAlternativeSelector>();

    strategies.add("cse/default", 3, [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
        auto scorer = std::make_shared<cse::DefaultScorer>();
        return std::make_unique<cse::CommonSubexpressionSolver>(expressions, scorer, selector, seed);
    });

    strategies.add("cse/potential", 1, [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
        std::mt19937 generator(seed);
        double alpha = std::uniform_real_distribution<double>(0.0, 0.6)(generator);
        auto scorer = std::make_shared<cse::PotentialScorer>(alpha);
        return std::make_unique<cse::CommonSubexpressionSolver>(expressions, scorer, selector, generator());
    });

    return strategies;
}

} // leo::presets
