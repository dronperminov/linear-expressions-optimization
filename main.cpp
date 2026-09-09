#include <iostream>
#include <ctime>
#include <random>

#include "src/optimization/selection/greedy_selector.h"
#include "src/optimization/selection/greedy_alternative_selector.h"
#include "src/optimization/selection/greedy_random_selector.h"
#include "src/optimization/solvers/vector_covering/vector_covering_solver.h"


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

    GreedySelector greedy;
    GreedyAlternativeSelector greedyAlternative(generator);
    GreedyRandomSelector greedyRandom(generator, 0.7);

    VectorCoveringSolver solver(expressions, parameters, defaultScorer, greedy);
    int additions = solver.solve();
    std::cout << "Additions (default, greedy): " << additions << std::endl;

    solver.setScorer(scorer1);
    additions = solver.solve();
    std::cout << "Additions (scorer1, greedy): " << additions << std::endl;

    solver.setScorer(scorer2);
    additions = solver.solve();
    std::cout << "Additions (scorer2, greedy): " << additions << std::endl;

    solver.setScorer(defaultScorer);
    solver.setSelector(greedyAlternative);
    additions = solver.solve();
    std::cout << "Additions (default, greedy-alternative): " << additions << std::endl;

    solver.setSelector(greedyRandom);
    additions = solver.solve();
    std::cout << "Additions (default, greedy-random): " << additions << std::endl;

    return 0;
}
