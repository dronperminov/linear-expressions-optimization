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

#include "src/optimization/solvers/cse/scorers/default_scorer.h"
#include "src/optimization/solvers/cse/scorers/potential_scorer.h"
#include "src/optimization/solvers/cse/common_subexpression_solver.h"

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

void solve(AbstractSolver& solver, const std::string& label, bool showSolution) {
    int additions = solver.solve();
    std::cout << "Additions (" << label << "): " << additions << std::endl;

    SolutionValidator validator;
    Solution solution = solver.getSolution();
    bool valid = validator.validate(solver.getExpressions(), solution);
    std::cout << "Valid: " << (valid ? "yes" : "no") << std::endl;

    if (showSolution)
        printSolution(solution);

    std::cout << std::endl;
}

int main() {
    std::vector<std::vector<int>> expressions = {
        {1, 1, 1, -1, -1, 0, 0, -1, -1},
        {1, 0, 0, -1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0, 0},
        {-1, 0, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 1, 1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 0, 0},
        {-1, 0, 0, 0, 0, 0, 1, 1, 0},
        {-1, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 1, 0},
        {1, 1, 1, 0, -1, -1, -1, -1, 0},
        {0, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, -1, 0, 0, 0, 0, 1, 1},
        {0, 0, 1, 0, 0, 0, 0, 0, -1},
        {0, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 1, 1},
        {0, 0, -1, 0, 1, 1, 0, 0, 0},
        {0, 0, 1, 0, 0, -1, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 1}
    };

    int seed = time(0);
    std::mt19937 generator(seed);

    VectorCoveringParameters parameters = {1};

    VectorCoveringDefaultScorer defaultScorer;
    VectorCoveringDefaultScorer customScorer(1000, 100, 5, 3);

    GreedySelector greedy;
    GreedyAlternativeSelector greedyAlternative(generator);
    GreedyRandomSelector greedyRandom(generator, 0.7);

    VectorCoveringSolver vec(expressions, parameters, defaultScorer, greedy);
    solve(vec, "vec: default, greedy", true);

    vec.setScorer(customScorer);
    solve(vec, "vec: custom, greedy", true);

    vec.setScorer(defaultScorer);
    vec.setSelector(greedyAlternative);
    solve(vec, "vec: default, greedy-alternative", true);

    CommonSubexpressionDefaultScorer cseDefaultScorer;
    CommonSubexpressionPotentialScorer csePotentialScorer(0.3);
    CommonSubexpressionSolver cse(expressions, cseDefaultScorer, greedyAlternative);
    solve(cse, "cse: default, greedy", true);

    cse.setScorer(csePotentialScorer);
    solve(cse, "cse: potential (0.3), greedy", true);

    cse.setScorer(cseDefaultScorer);
    cse.setSelector(greedyAlternative);
    solve(cse, "cse: default, greedy-alternative", true);

    cse.setSelector(greedyRandom);
    solve(cse, "cse: default, greedy-random", true);

    return 0;
}
