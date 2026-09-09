#include <iostream>
#include <ctime>
#include <random>

#include "src/entities/solution.h"

#include "src/optimization/selection/greedy_selector.h"
#include "src/optimization/selection/greedy_alternative_selector.h"
#include "src/optimization/selection/greedy_random_selector.h"

#include "src/optimization/solvers/vector_covering/scorers/default_scorer.h"
#include "src/optimization/solvers/vector_covering/vector_covering_solver.h"


void printSolution(const Solution& solution) {
    std::cout << "- substitutions:" << std::endl;
    for (size_t i = 0; i < solution.substitutions.size(); i++) {
        Substitution s = solution.substitutions[i];
        std::cout << "  x" << (solution.dimension + i) << " = (" << s.ai << "x" << s.i << ", " << s.aj << "x" << s.j << ")" << std::endl;
    }

    std::cout << "- expressions:" << std::endl;
    for (const std::vector<Term>& terms : solution.expressions) {
        std::cout << "  " << terms[0].value << "x" << terms[0].index;

        for (size_t i = 1; i < terms.size(); i++)
            std::cout << terms[i].value << "x" << terms[i].index;

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

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

    VectorCoveringDefaultScorer defaultScorer;
    VectorCoveringDefaultScorer scorer1(1000, 100, 5, 3);
    VectorCoveringDefaultScorer scorer2(1, 1, 0, 0);

    GreedySelector greedy;
    GreedyAlternativeSelector greedyAlternative(generator);
    GreedyRandomSelector greedyRandom(generator, 0.7);

    VectorCoveringSolver solver(expressions, parameters, defaultScorer, greedy);
    int additions = solver.solve();
    std::cout << "Additions (default, greedy): " << additions << std::endl;
    Solution solution = solver.getSolution();

    std::cout << "Solution: " << std::endl;
    printSolution(solution);

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
