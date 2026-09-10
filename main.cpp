#include <iostream>
#include <ctime>
#include <random>

#include "src/entities/solution.h"

#include "src/optimization/selection/greedy_selector.h"
#include "src/optimization/selection/greedy_alternative_selector.h"
#include "src/optimization/selection/greedy_random_selector.h"

#include "src/optimization/solvers/abstract_solver.h"
#include "src/optimization/solvers/vector_covering/scorers/default_scorer.h"
#include "src/optimization/solvers/vector_covering/vector_covering_solver.h"

#include "src/validation/solution_validator.h"


void printTermFirst(size_t index, int value) {
    if (value == -1)
        std::cout << "-";
    else if (value != 1)
        std::cout << value;

    std::cout << "x" << index;
}

void printTerm(size_t index, int value) {
    std::cout << (value > 0 ? " + " : " - ");

    if (value > 1 || value < -1)
        std::cout << std::abs(value);

    std::cout << "x" << index;
}

void printSolution(const Solution& solution) {
    std::cout << "+ substitutions (" << solution.substitutions.size() << "):" << std::endl;
    for (size_t i = 0; i < solution.substitutions.size(); i++) {
        Substitution s = solution.substitutions[i];
        std::cout << "| x" << (solution.dimension + i) << " = ";
        printTermFirst(s.i, s.ai);
        printTerm(s.j, s.aj);
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "+ expressions:" << std::endl;
    for (const std::vector<Term>& terms : solution.expressions) {
        std::cout << "| ";
        printTermFirst(terms[0].index, terms[0].value);

        for (size_t i = 1; i < terms.size(); i++)
            printTerm(terms[i].index, terms[i].value);

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

void solve(AbstractSolver& solver, const std::string& label, const std::vector<std::vector<int>>& expressions, bool showSolution) {
    int additions = solver.solve();
    std::cout << "Additions (" << label << "): " << additions << std::endl;

    SolutionValidator validator;
    Solution solution = solver.getSolution();
    bool valid = validator.validate(expressions, solution);
    std::cout << "Valid: " << (valid ? "yes" : "no") << std::endl;

    if (showSolution)
        printSolution(solution);

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
    solve(solver, "default, greedy", expressions, true);

    solver.setScorer(scorer1);
    solve(solver, "scorer1, greedy", expressions, true);

    solver.setScorer(scorer2);
    solve(solver, "scorer2, greedy", expressions, true);

    solver.setScorer(defaultScorer);
    solver.setSelector(greedyAlternative);
    solve(solver, "default, greedy-alternative", expressions, true);

    solver.setSelector(greedyRandom);
    solve(solver, "default, greedy-random", expressions, true);

    return 0;
}
