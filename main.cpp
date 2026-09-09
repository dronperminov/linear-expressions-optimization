#include <iostream>
#include <ctime>
#include <random>

#include "src/optimization/selection/greedy_selector.h"
#include "src/optimization/selection/greedy_alternative_selector.h"
#include "src/optimization/selection/greedy_random_selector.h"
#include "src/optimization/optimizers/vector_covering_optimizer.h"


int main() {
    std::vector<std::vector<int>> expressions = {
        {1, 1, 1, 1},
        {1, 1, 1, 0},
        {1, 1, 0, 1},
        {1, 0, 1, 1},
        {0, 1, 1, 1},
    };

    int seed = time(0);
    std::mt19937 generator(seed);

    VectorCoveringParameters parameters = {1};

    VectorCoveringScorer defaultScorer;
    VectorCoveringScorer scorer1(1000, 100, 5, 3);
    VectorCoveringScorer scorer2(1, 1, 0, 0);

    GreedySelector<Candidate> greedy;
    GreedyAlternativeSelector<Candidate> greedyAlternative(generator);
    GreedyRandomSelector<Candidate> greedyRandom(generator, 0.7);

    VectorCoveringOptimizer optimizer(expressions, parameters, defaultScorer, greedy);
    int additions = optimizer.optimize();
    std::cout << "Additions (default, greedy): " << additions << std::endl;

    optimizer.setScorer(scorer1);
    additions = optimizer.optimize();
    std::cout << "Additions (scorer1, greedy): " << additions << std::endl;

    optimizer.setScorer(scorer2);
    additions = optimizer.optimize();
    std::cout << "Additions (scorer2, greedy): " << additions << std::endl;

    optimizer.setScorer(defaultScorer);
    optimizer.setSelector(greedyAlternative);
    additions = optimizer.optimize();
    std::cout << "Additions (default, greedy-alternative): " << additions << std::endl;

    optimizer.setSelector(greedyRandom);
    additions = optimizer.optimize();
    std::cout << "Additions (default, greedy-random): " << additions << std::endl;

    return 0;
}
