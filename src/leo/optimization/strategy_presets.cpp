#include <leo/optimization/strategy_presets.h>

#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <leo/optimization/selection/greedy_alternative_selector.h>
#include <leo/optimization/solvers/cse/common_subexpression_solver.h>
#include <leo/optimization/solvers/cse/scorers/default_scorer.h>
#include <leo/optimization/solvers/cse/scorers/intersections_scorer.h>
#include <leo/optimization/solvers/cse/scorers/potential_scorer.h>
#include <leo/optimization/solvers/vector_covering/scorers/default_scorer.h>
#include <leo/optimization/solvers/vector_covering/scorers/distance_scorer.h>
#include <leo/optimization/solvers/vector_covering/vector_covering_solver.h>

namespace leo::presets {

StrategyPool vectorCoveringDefault(const leo::vector_covering::Parameters& parameters) {
    StrategyPool strategies;

    auto selector = std::make_shared<GreedyAlternativeSelector>();

    std::vector<std::pair<std::string, std::shared_ptr<const vector_covering::VectorCoveringScorer>>> scorers = {
        {"vec/1", std::make_shared<vector_covering::DefaultScorer>()},
        {"vec/2", std::make_shared<vector_covering::DefaultScorer>(1000.0, 100.0,  0.0,  0.0, 0.0, 0.0)},
        {"vec/3", std::make_shared<vector_covering::DefaultScorer>(1000.0, 100.0,  0.0,  0.0, 0.0, 5.0)},
        {"vec/4", std::make_shared<vector_covering::DefaultScorer>(1000.0, 100.0,  0.0,  0.1, 0.0, 0.0)},
        {"vec/5", std::make_shared<vector_covering::DefaultScorer>(1000.0,  30.0,  0.0,  0.1, 0.2, 0.5)},
        {"vec/6", std::make_shared<vector_covering::DefaultScorer>(1000.0,  30.0,  0.0,  0.1, 0.5, 5.0)},
        {"vec/7", std::make_shared<vector_covering::DefaultScorer>(1000.0, 100.0,  0.0, 0.01, 0.0, 0.5)},
        {"vec/8", std::make_shared<vector_covering::DefaultScorer>(1000.0, 100.0, 0.01,  0.1, 0.1, 0.0)},
        {"vec/9", std::make_shared<vector_covering::DefaultScorer>(1000.0,  30.0, 0.01, 0.01, 0.0, 0.0)}
    };

    for (const auto& scorer : scorers) {
        strategies.add(scorer.first, [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
            return std::make_unique<vector_covering::VectorCoveringSolver>(expressions, parameters, scorer.second, selector, seed);
        });
    }

    strategies.add("vec/rnd", [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
        std::mt19937 generator(seed);
        std::vector<double> coverWeights = {10000.0, 1000.0};
        std::vector<double> oneStepWeights = {1000.0, 500.0, 300.0, 100.0};
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

StrategyPool vectorCoveringDistance(const leo::vector_covering::Parameters& parameters) {
    StrategyPool strategies;

    auto selector = std::make_shared<GreedyAlternativeSelector>();

    std::vector<std::pair<std::string, std::shared_ptr<const vector_covering::VectorCoveringScorer>>> scorers = {
        {"vec/dst1", std::make_shared<vector_covering::DistanceScorer>(1000.0, 1.0,  0.0, 10.0, 2)},
        {"vec/dst2", std::make_shared<vector_covering::DistanceScorer>(1000.0, 1.0, 0.01, 10.0, 2)}
    };

    for (const auto& scorer : scorers) {
        strategies.add(scorer.first, [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
            return std::make_unique<vector_covering::VectorCoveringSolver>(expressions, parameters, scorer.second, selector, seed);
        });
    }

    return strategies;
}

StrategyPool vectorCoveringAll(const leo::vector_covering::Parameters& parameters) {
    StrategyPool strategies;
    strategies.add(vectorCoveringDistance(parameters));
    strategies.add(vectorCoveringDefault(parameters));

    return strategies;
}

StrategyPool cseVanilla(const leo::cse::Parameters& parameters) {
    StrategyPool strategies;

    auto selector = std::make_shared<GreedyAlternativeSelector>();
    auto scorer = std::make_shared<cse::DefaultScorer>();

    strategies.add("cse/vanilla", [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
        return std::make_unique<cse::CommonSubexpressionSolver>(expressions, parameters, scorer, selector, seed);
    });

    return strategies;
}

StrategyPool csePotential(const leo::cse::Parameters& parameters) {
    StrategyPool strategies;

    auto selector = std::make_shared<GreedyAlternativeSelector>();

    strategies.add("cse/potential", [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
        std::mt19937 generator(seed);
        double alpha = std::uniform_real_distribution<double>(0.0, 0.6)(generator);
        auto scorer = std::make_shared<cse::PotentialScorer>(alpha);
        return std::make_unique<cse::CommonSubexpressionSolver>(expressions, parameters, scorer, selector, generator());
    });

    return strategies;
}

StrategyPool cseIntersections(const leo::cse::Parameters& parameters) {
    StrategyPool strategies;

    auto selector = std::make_shared<GreedyAlternativeSelector>();

    strategies.add("cse/intersections", [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
        std::mt19937 generator(seed);
        double alpha = std::uniform_real_distribution<double>(0.0, 0.6)(generator);
        double beta = std::uniform_real_distribution<double>(0.5, 1.0)(generator);
        auto scorer = std::make_shared<cse::IntersectionsScorer>(alpha, beta);
        return std::make_unique<cse::CommonSubexpressionSolver>(expressions, parameters, scorer, selector, generator());
    });

    return strategies;
}

StrategyPool cseAll(const leo::cse::Parameters& parameters) {
    StrategyPool strategies;

    auto selector = std::make_shared<GreedyAlternativeSelector>();

    strategies.add("cse/vanilla", 3, [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
        auto scorer = std::make_shared<cse::DefaultScorer>();
        return std::make_unique<cse::CommonSubexpressionSolver>(expressions, parameters, scorer, selector, seed);
    });

    strategies.add("cse/potential", 1, [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
        std::mt19937 generator(seed);
        double alpha = std::uniform_real_distribution<double>(0.0, 0.6)(generator);
        auto scorer = std::make_shared<cse::PotentialScorer>(alpha);
        return std::make_unique<cse::CommonSubexpressionSolver>(expressions, parameters, scorer, selector, generator());
    });

    strategies.add("cse/intersections", 1, [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
        std::mt19937 generator(seed);
        double alpha = std::uniform_real_distribution<double>(0.0, 0.6)(generator);
        double beta = std::uniform_real_distribution<double>(0.5, 1.0)(generator);
        auto scorer = std::make_shared<cse::IntersectionsScorer>(alpha, beta);
        return std::make_unique<cse::CommonSubexpressionSolver>(expressions, parameters, scorer, selector, generator());
    });

    return strategies;
}

} // leo::presets
