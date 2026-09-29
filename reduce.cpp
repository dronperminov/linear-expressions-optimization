#include <iostream>
#include <filesystem>
#include <memory>
#include <random>
#include <vector>
#include <leo/leo.h>

#include "src/cli/arg_parser.h"
#include "src/io/expressions_reader.h"

using namespace leo;

void solve(Solver& solver, size_t& bestAdditions, Solution& solution) {
    size_t additions = solver.solve();

    if (additions < bestAdditions) {
        bestAdditions = additions;
        solution = solver.getSolution();
    }
}

void reduceVectorCovering(const std::vector<std::vector<int>>& expressions, size_t& bestAdditions, Solution& solution, int maxAbsValue, size_t lowerBound, std::mt19937& generator, int iterations) {
    vector_covering::VectorCoveringParameters parameters = {maxAbsValue, true, true};

    std::vector<vector_covering::DefaultScorer> scorers = {
        vector_covering::DefaultScorer(),
        vector_covering::DefaultScorer(10000, 1000, 0, 1, 0, 10),
        vector_covering::DefaultScorer(10000, 1000, 1, 0, 0, 10),
        vector_covering::DefaultScorer(10000, 1000, 5, 3, 0, 10),
        vector_covering::DefaultScorer(10000, 1000, 0, 0, 0, 10)
    };

    GreedyAlternativeSelector selector(generator);
    vector_covering::VectorCoveringSolver solver(expressions, parameters, scorers[0], selector);

    for (int i = 0; i < iterations && lowerBound < bestAdditions; i++) {
        solver.setScorer(scorers[generator() % scorers.size()]);
        solve(solver, bestAdditions, solution);
    }
}

void reduceCSE(const std::vector<std::vector<int>>& expressions, size_t& bestAdditions, Solution& solution, size_t lowerBound, std::mt19937& generator, int iterations) {
    GreedyAlternativeSelector selector(generator);

    cse::DefaultScorer defaultScorer;
    cse::PotentialScorer potentialScorer(0.3);
    cse::CommonSubexpressionSolver solver(expressions, defaultScorer, selector);

    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    for (int i = 0; i < iterations && lowerBound < bestAdditions; i++) {
        if (uniform(generator) < 0.25) {
            potentialScorer.setAlpha(uniform(generator) * 0.6);
            solver.setScorer(potentialScorer);
        }
        else {
            solver.setScorer(defaultScorer);
        }

        solve(solver, bestAdditions, solution);
    }
}

Solution reduceSystem(const ExpressionsSystem& expressionsSystem, std::mt19937& generator, int vecIterations, int cseIterations) {
    int maxAbsValue = expressionsSystem.getMaxAbsValue();
    size_t lowerBound = expressionsSystem.getAdditionsLowerBound();

    Solution solution = expressionsSystem.getNaiveSolution();
    size_t bestAdditions = solution.getAdditions();

    reduceVectorCovering(expressionsSystem.getExpressions(), bestAdditions, solution, maxAbsValue, lowerBound, generator, vecIterations);
    reduceCSE(expressionsSystem.getExpressions(), bestAdditions, solution, lowerBound, generator, cseIterations);

    return solution;
}

Solution reduce(const ExpressionsSystem& expressionsSystem, std::mt19937& generator, int vecIterations, int cseIterations, bool tryTranspose) {
    Solution solution = reduceSystem(expressionsSystem, generator, vecIterations, cseIterations);
    size_t bestAdditions = solution.getAdditions();

    if (tryTranspose) {
        ExpressionsSystem transposedSystem(expressionsSystem.getTransposedExpressions());
        Solution transposed = reduceSystem(transposedSystem, generator, vecIterations, cseIterations);
        SolutionTransposer transposer;
        Solution reversed = transposer.transpose(transposed);

        if (reversed.getAdditions() < bestAdditions)
            solution = reversed;
    }

    return solution;
}

std::unique_ptr<SolutionFormatter> getFormatter(const std::string& outputPath, const std::string& format) {
    std::string detectedFormat = format;

    if (format == "auto") {
        std::string extension = std::filesystem::path(outputPath).extension().string();
        detectedFormat = extension.empty() ? "" : extension.substr(1);
    }

    if (detectedFormat == "slp" )
        return std::make_unique<SlpSolutionFormatter>("i", "o", "t", 0);

    if (detectedFormat == "txt")
        return std::make_unique<PlainTextSolutionFormatter>("x", "y", "x", 1);

    if (detectedFormat == "json")
        return std::make_unique<JsonSolutionFormatter>();

    throw std::runtime_error("invalid formatter type \"" + detectedFormat + "\"");
}

int main(int argc, char** argv) {
    ArgParser parser("reduce", "Minimize the number of additions and subtractions required to evaluate a system of linear expressions.");
    parser.add("--quiet", "-q", ArgType::Flag, "Suppress all output to stdout, only save the solution to the output file");

    parser.addSection("Input / output");
    parser.add("--input-path", "-i", ArgType::Path, "Path to the input file containing linear expressions", "", true);
    parser.add("--output-path", "-o", ArgType::Path, "Path to the output file for the resulting solution", "output.txt");

    parser.addSection("Optimization");
    parser.add("--seed", ArgType::UInt, "Random seed; 0 uses a time-based seed", "0");
    parser.add("--vec-iterations", ArgType::UInt, "Number of iterations of the vector covering solver", "10");
    parser.add("--cse-iterations", ArgType::UInt, "Number of iterations of the common subexpression solver", "100");
    parser.add("--try-transpose", ArgType::Flag, "Additionally try solving the transposed system, then transpose the solution back");

    parser.addSection("Solution");
    parser.add("--validate", ArgType::Flag, "Validate the resulting solution");
    parser.addChoices("--format", "-f", ArgType::String, "Output format for the solution", {"slp", "txt", "json", "auto"}, "auto");

    if (!parser.parse(argc, argv))
        return 0;

    bool quiet = parser.isSet("--quiet");

    std::string inputPath = parser["--input-path"];
    std::string outputPath = parser["--output-path"];

    int seed = parser.isSet("--seed") && std::stoi(parser["--seed"]) != 0 ? std::stoi(parser["--seed"]) : time(0);
    int vecIterations = std::stoi(parser["--vec-iterations"]);
    int cseIterations = std::stoi(parser["--cse-iterations"]);
    bool tryTranspose = parser.isSet("--try-transpose");

    bool validate = parser.isSet("--validate");
    std::string format = parser["--format"];

    if (!quiet) {
        std::cout << "Parsed parameters:" << std::endl;
        std::cout << "- input path: " << inputPath << std::endl;
        std::cout << "- output path: " << outputPath << std::endl;
        std::cout << "- random seed: " << seed << std::endl;
        std::cout << "- iterations (vec / cse): " << vecIterations << " / " << cseIterations << std::endl;
        std::cout << "- try transposing: " << (tryTranspose ? "yes" : "no") << std::endl;
        std::cout << "- validate solution: " << (validate ? "yes" : "no") << std::endl;
        std::cout << "- format: " << format << std::endl;
        std::cout << std::endl;
    }

    std::mt19937 generator(seed);

    try {
        std::unique_ptr<SolutionFormatter> formatter = getFormatter(outputPath, format);

        ExpressionsReader reader;
        ExpressionsSystem expressionsSystem = reader.read(inputPath);

        if (!quiet) {
            std::cout << "Readed system of expressions:" << std::endl;
            expressionsSystem.describe(std::cout);
            std::cout << std::endl;
        }

        Solution solution = reduce(expressionsSystem, generator, vecIterations, cseIterations, tryTranspose);

        if (validate) {
            if (!expressionsSystem.validateSolution(solution))
                throw std::runtime_error("solution is not valid");

            if (!quiet)
                std::cout << "Solution is valid" << std::endl;
        }

        std::ofstream fout(outputPath);
        formatter->format(fout, solution);
        fout.close();

        if (!quiet) {
            std::cout << "Optimized solution has " << solution.getAdditions() << " additions" << std::endl;
            std::cout << "Solution saved to \"" << outputPath << "\"" << std::endl;
        }
    }
    catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << std::endl;
        return -1;
    }

    return 0;
}
