#include <iostream>
#include <ctime>
#include <random>

#include <leo/leo.h>

using namespace leo;

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

void solve(const ExpressionsSystem& expressionsSystem, Solver& solver, const std::string& label, bool showSolution) {
    int additions = solver.solve();
    std::cout << "Additions (" << label << "): " << additions << std::endl;

    Solution solution = solver.getSolution();
    bool valid = expressionsSystem.validateSolution(solution);
    std::cout << "Valid: " << (valid ? "yes" : "no") << std::endl;

    if (!valid)
        throw std::runtime_error("invalid solution");

    if (showSolution)
        printSolution(solution);

    std::cout << std::endl;
}

int main() {
    ExpressionsSystem expressionsSystem({
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
    });

    std::cout << "Initial system:" << std::endl;
    std::cout << "- variables: " << expressionsSystem.getVariablesCount() << std::endl;
    std::cout << "- expressions: " << expressionsSystem.getExpressionsCount() << std::endl;
    std::cout << "- lower bound: " << expressionsSystem.getAdditionsLowerBound() << std::endl;
    std::cout << std::endl;

    int seed = time(0);
    std::mt19937 generator(seed);

    vector_covering::VectorCoveringParameters parameters = {1, true};

    vector_covering::DefaultScorer defaultScorer;
    vector_covering::DefaultScorer customScorer(1000, 100, 5, 3);

    GreedySelector greedy;
    GreedyAlternativeSelector greedyAlternative(generator);
    GreedyRandomSelector greedyRandom(generator, 0.3);

    vector_covering::VectorCoveringSolver vec(expressionsSystem.getExpressions(), parameters, defaultScorer, greedy);
    solve(expressionsSystem, vec, "vec: default, greedy", true);

    vec.setScorer(customScorer);
    solve(expressionsSystem, vec, "vec: custom, greedy", true);

    vec.setScorer(defaultScorer);
    vec.setSelector(greedyAlternative);
    solve(expressionsSystem, vec, "vec: default, greedy-alternative", true);

    cse::DefaultScorer cseDefaultScorer;
    cse::PotentialScorer csePotentialScorer(0.3);
    cse::CommonSubexpressionSolver cse(expressionsSystem.getExpressions(), cseDefaultScorer, greedyAlternative);
    solve(expressionsSystem, cse, "cse: default, greedy", true);

    cse.setScorer(csePotentialScorer);
    solve(expressionsSystem, cse, "cse: potential (0.3), greedy", true);

    cse.setScorer(cseDefaultScorer);
    cse.setSelector(greedyAlternative);
    solve(expressionsSystem, cse, "cse: default, greedy-alternative", true);

    cse.setSelector(greedyRandom);
    solve(expressionsSystem, cse, "cse: default, greedy-random", true);

    return 0;
}
