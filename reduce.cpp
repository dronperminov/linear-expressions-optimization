#include <iostream>
#include <random>
#include <vector>
#include <leo/leo.h>

#include "src/cli/arg_parser.h"
#include "src/io/expressions_reader.h"

using namespace leo;

Solution reduce(const ExpressionsSystem& expressionsSystem, std::mt19937& generator, int vecIterations, int cseIterations) {
    int maxAbsValue = expressionsSystem.getMaxAbsValue();
    size_t lowerBound = expressionsSystem.getAdditionsLowerBound();

    vector_covering::VectorCoveringParameters parameters = {maxAbsValue, true, true};

    std::vector<vector_covering::DefaultScorer> vecScorers = {
        vector_covering::DefaultScorer(1000, 100, 0, 1, 0),
        vector_covering::DefaultScorer(1000, 100, 1, 0, 0),
        vector_covering::DefaultScorer(1000, 100, 5, 3, 0),
        vector_covering::DefaultScorer(1000, 100, 0, 0, 0)
    };

    GreedyAlternativeSelector selector(generator);

    vector_covering::VectorCoveringSolver vec(expressionsSystem.getExpressions(), parameters, vecScorers[0], selector);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    size_t bestAdditions = vec.solve();
    Solution solution = vec.getSolution();

    for (int i = 1; i < vecIterations && lowerBound < bestAdditions; i++) {
        vec.setScorer(vecScorers[generator() % vecScorers.size()]);
        size_t additions = vec.solve();

        if (additions < bestAdditions) {
            bestAdditions = additions;
            solution = vec.getSolution();
        }
    }

    cse::DefaultScorer cseDefaultScorer;
    cse::PotentialScorer csePotentialScorer(0.3);
    cse::CommonSubexpressionSolver cse(expressionsSystem.getExpressions(), cseDefaultScorer, selector);

    for (int i = 0; i < cseIterations && lowerBound < bestAdditions; i++) {
        if (uniform(generator) < 0.25) {
            csePotentialScorer.setAlpha(0.1 + uniform(generator));
            cse.setScorer(csePotentialScorer);
        }
        else {
            cse.setScorer(cseDefaultScorer);
        }

        size_t additions = cse.solve();
        if (additions < bestAdditions) {
            bestAdditions = additions;
            solution = cse.getSolution();
        }
    }

    return solution;
}

int main(int argc, char** argv) {
    ArgParser parser("reduce", "");

    parser.addSection("Input / output");
    parser.add("--input-path", "-i", ArgType::Path, "Path to input file with expressions", "", true);
    parser.add("--output-path", "-o", ArgType::Path, "Path to output file", "output.slp");

    parser.addSection("Other parameters");
    parser.add("--seed", ArgType::Natural, "Random seed, 0 uses time-based seed", "0");
    parser.add("--vec-iterations", ArgType::Natural, "Iterations of vector covering based solver", "10");
    parser.add("--cse-iterations", ArgType::Natural, "Iterations of common subexpression solver", "100");
    parser.add("--validate", ArgType::Flag, "Validate solution");

    if (!parser.parse(argc, argv))
        return 0;

    std::string inputPath = parser["--input-path"];
    std::string outputPath = parser["--output-path"];

    int seed = parser.isSet("--seed") ? std::stoi(parser["--seed"]) : time(0);
    std::mt19937 generator(seed);

    int vecIterations = std::stoi(parser["--vec-iterations"]);
    int cseIterations = std::stoi(parser["--cse-iterations"]);
    bool validate = parser.isSet("--validate");

    try {
        ExpressionsSystem expressionsSystem = ExpressionsReader::read(inputPath);

        std::cout << "Readed system from \"" << inputPath << "\"" << std::endl;
        expressionsSystem.describe(std::cout);
        std::cout << std::endl;

        Solution solution = reduce(expressionsSystem, generator, vecIterations, cseIterations);

        if (validate) {
            if (!expressionsSystem.validateSolution(solution))
                throw std::runtime_error("solution is not valid");

            std::cout << "Solution is valid" << std::endl;
        }

        std::cout << "Optimized solution has " << solution.getAdditions() << " additions" << std::endl;

        utils::formatters::SlpFormatter formatter;
        std::ofstream fout(outputPath);
        formatter.format(fout, solution);
        fout.close();

        std::cout << "Solution saved to \"" << outputPath << "\"" << std::endl;
    }
    catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << std::endl;
        return 1;
    }

    return 0;
}
