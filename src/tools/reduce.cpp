#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <omp.h>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <leo/leo.h>

#include "../cli/arg_parser.h"
#include "../io/expressions_reader.h"
#include "../io/expressions_readers/sms_expressions_reader.h"
#include "../io/expressions_readers/txt_expressions_reader.h"
#include "../utils.h"

using namespace leo;

TaskPool initTasks(const ExpressionsSystem& expressionsSystem, const ArgParser& parser, std::mt19937& generator) {
    TaskPool tasks;

    StrategyPool vecStrategies = leo::presets::vectorCoveringDefault(expressionsSystem, parser.isSet("--vec-add-target-pairs"));
    size_t vecIterations = std::stoull(parser["--vec-iterations"]);

    if (parser["--vec-sampling"] == "sample")
        tasks.add(vecStrategies.sample(vecIterations, generator));
    else
        tasks.add(vecStrategies.each(vecIterations, generator));

    StrategyPool cseStrategies = leo::presets::cseDefault(expressionsSystem);
    size_t cseIterations = std::stoull(parser["--cse-iterations"]);

    if (parser["--cse-sampling"] == "sample")
        tasks.add(cseStrategies.sample(cseIterations, generator));
    else
        tasks.add(cseStrategies.each(cseIterations, generator));

    return tasks;
}

Solution reduce(const ExpressionsSystem& expressionsSystem, const ArgParser& parser, std::mt19937& generator) {
    size_t threads = std::stoull(parser["--threads"]);

    Reducer reducer(threads);
    std::vector<TaskPool> pools;

    reducer.addGroup(expressionsSystem);
    pools.push_back(initTasks(expressionsSystem, parser, generator));

    std::optional<ExpressionsSystem> transposedSystem;
    if (parser.isSet("--try-transpose")) {
        transposedSystem.emplace(expressionsSystem.getTransposedExpressions());
        reducer.addGroup(*transposedSystem);
        pools.push_back(initTasks(*transposedSystem, parser, generator));
    }

    reducer.reduce(pools);

    Solution solution = reducer.getSolution(0);

    if (transposedSystem) {
        SolutionTransposer transposer;
        Solution transposed = transposer.transpose(reducer.getSolution(1));

        if (transposed.getAdditions() < solution.getAdditions())
            solution = transposed;
    }

    if (!parser.isSet("--quiet"))
        std::cout << "Optimized solution has " << solution.getAdditions() << " additions" << std::endl;

    return solution;
}

Solution inlineSubstitutions(const Solution& solution, const ArgParser& parser) {
    SolutionSubstitutionInliner inliner;
    Solution optimized = inliner.optimize(solution);

    size_t substitutionsBefore = solution.substitutions.size();
    size_t substitutionsAfter = optimized.substitutions.size();

    size_t additionsBefore = solution.getAdditions();
    size_t additionsAfter = optimized.getAdditions();

    if (!parser.isSet("--quiet")) {
        std::cout << "Number of substitutions ";

        if (substitutionsAfter < substitutionsBefore) {
            std::cout << "reduced from " << substitutionsBefore << " to " << substitutionsAfter;
        }
        else {
            std::cout << "remained unchanged: " << substitutionsBefore;
        }

        if (additionsBefore != additionsAfter)
            std::cout << ", additions changed to " << additionsAfter;

        std::cout << std::endl;
    }

    return optimized;
}

Solution optimizeInversions(const Solution& solution, const ArgParser& parser, std::mt19937& generator) {
    size_t iterations = std::stoull(parser["--optimize-signs-iterations"]);

    SolutionSignOptimizer optimizer;
    Solution optimized = optimizer.optimize(solution, generator, iterations);

    size_t inversionsBefore = solution.getInversions();
    size_t inversionsAfter = optimized.getInversions();

    if (!parser.isSet("--quiet")) {
        std::cout << "Number of sign inversions ";

        if (inversionsAfter < inversionsBefore) {
            std::cout << "reduced from " << inversionsBefore << " to " << inversionsAfter << std::endl;
        }
        else {
            std::cout << "remained unchanged: " << inversionsBefore << std::endl;
        }
    }

    return optimized;
}

std::unique_ptr<SolutionFormatter> getFormatter(const std::string& path, const std::string& format) {
    std::string detectedFormat = format;

    if (format == "auto") {
        std::string extension = std::filesystem::path(path).extension().string();
        detectedFormat = extension.empty() ? "" : extension.substr(1);
    }

    if (detectedFormat == "slp" )
        return std::make_unique<SlpSolutionFormatter>("i", "o", "t", 0);

    if (detectedFormat == "txt")
        return std::make_unique<PlainTextSolutionFormatter>("x", "y", "x", 1);

    if (detectedFormat == "json")
        return std::make_unique<JsonSolutionFormatter>();

    throw std::runtime_error("unsupported output format: \"" + detectedFormat + "\", expected one of: slp, txt, json");
}

std::unique_ptr<ExpressionsReader> getExpressionsReader(const std::string& path) {
    std::string extension = std::filesystem::path(path).extension().string();

    if (extension == ".sms")
        return std::make_unique<SmsExpressionsReader>();

    return std::make_unique<TxtExpressionsReader>();
}

std::string replaceTemplates(const std::string& path, const Solution& solution) {
    std::string additions = std::to_string(solution.getAdditions());
    std::string inversions = std::to_string(solution.getInversions());
    std::string substitutions = std::to_string(solution.substitutions.size());
    std::string dimension = std::to_string(solution.dimension);

    std::string replaced = replace(path, "{additions}", additions);
    replaced = replace(replaced, "{inversions}", inversions);
    replaced = replace(replaced, "{substitutions}", substitutions);
    replaced = replace(replaced, "{dimension}", dimension);

    return replaced;
}

int main(int argc, char** argv) {
    ArgParser parser("reduce", "Minimize the number of additions and subtractions required to evaluate a system of linear expressions.");
    parser.add("--threads", "-t", ArgType::Natural, "Number of OpenMP threads to use", std::to_string(omp_get_max_threads()));
    parser.add("--seed", "-s", ArgType::UInt, "Random seed; 0 uses a time-based seed", "0");
    parser.add("--quiet", "-q", ArgType::Flag, "Suppress all output to stdout");

    parser.addSection("Input / output");
    parser.add("--input-path", "-i", ArgType::Path, "Path to the input file containing linear expressions", "", true);
    parser.add("--output-path", "-o", ArgType::Path, "Path to the output file for the resulting solution, or \"stdout\" for standard output", "output.txt");

    parser.addSection("Optimization");
    parser.add("--vec-add-target-pairs", ArgType::Flag, "Precompute target vectors that can be obtained with one addition or subtraction (xi +/- xj)");
    parser.add("--vec-iterations", ArgType::UInt, "Number of iterations of the vector covering solver", "10");
    parser.addChoices("--vec-sampling", ArgType::String, "Vector covering strategy sampling: \"sample\" (random per iteration) or \"each\" (all strategies)", {"sample", "each"}, "sample");
    parser.add("--cse-iterations", ArgType::UInt, "Number of iterations of the common subexpression solver", "100");
    parser.addChoices("--cse-sampling", ArgType::String, "Common subexpression strategy sampling: \"sample\" (random per iteration) or \"each\" (all strategies)", {"sample", "each"}, "sample");
    parser.add("--try-transpose", "-T", ArgType::Flag, "Additionally try solving the transposed system, then transpose the solution back");

    parser.addSection("Solution");
    parser.add("--validate", ArgType::Flag, "Validate the resulting solution");
    parser.add("--optimize-signs-iterations", ArgType::UInt, "Number of iterations of the sign inversion optimizer; 0 disables it", "0");
    parser.add("--inline-substitutions", ArgType::Flag, "Inline substitutions that occur only once");
    parser.addChoices("--format", "-f", ArgType::String, "Output format for the solution", {"slp", "txt", "json", "auto"}, "auto");

    if (!parser.parse(argc, argv))
        return 0;

    if (parser["--output-path"] == "stdout" && parser["--format"] == "auto") {
        std::cerr << "Format \"auto\" cannot be used with stdout. Specify the format explicitly." << std::endl;
        return -1;
    }

    bool quiet = parser.isSet("--quiet");
    size_t threads = std::stoi(parser["--threads"]);
    uint32_t seed = parser.isSet("--seed") && std::stoul(parser["--seed"]) != 0 ? std::stoul(parser["--seed"]) : time(0);

    std::string inputPath = parser["--input-path"];
    std::string outputPath = parser["--output-path"];

    bool validate = parser.isSet("--validate");
    std::string format = parser["--format"];

    if (!quiet) {
        std::cout << "Parsed parameters:" << std::endl;
        std::cout << "- input path: " << inputPath << std::endl;
        std::cout << "- output path: " << outputPath << std::endl;
        std::cout << "- random seed: " << seed << std::endl;
        std::cout << "- threads: " << threads << std::endl;
        std::cout << "- iterations (vec / cse): " << std::stoi(parser["--vec-iterations"]) << " / " << std::stoi(parser["--cse-iterations"]) << std::endl;
        std::cout << "- sampling strategies (vec / cse): " << parser["--vec-sampling"] << " / " << parser["--cse-sampling"] << std::endl;
        std::cout << "- try transposing: " << (parser.isSet("--try-transpose") ? "yes" : "no") << std::endl;
        std::cout << "- validate solution: " << (validate ? "yes" : "no") << std::endl;
        std::cout << "- output format: " << format << std::endl;
        std::cout << std::endl;
    }

    std::mt19937 generator(seed);

    try {
        std::unique_ptr<SolutionFormatter> formatter = getFormatter(outputPath, format);
        std::unique_ptr<ExpressionsReader> reader = getExpressionsReader(inputPath);

        ExpressionsSystem expressionsSystem = reader->read(inputPath);

        if (!quiet) {
            std::cout << "Read system of expressions:" << std::endl;
            expressionsSystem.describe(std::cout);
            std::cout << std::endl;
        }

        auto t1 = std::chrono::steady_clock::now();
        Solution solution = reduce(expressionsSystem, parser, generator);

        if (parser.isSet("--inline-substitutions"))
            solution = inlineSubstitutions(solution, parser);

        if (parser.isSet("--optimize-signs-iterations"))
            solution = optimizeInversions(solution, parser, generator);

        auto t2 = std::chrono::steady_clock::now();

        if (validate) {
            if (!expressionsSystem.validateSolution(solution))
                throw std::runtime_error("solution is not valid");

            if (!quiet)
                std::cout << "Solution is valid" << std::endl;
        }

        if (!quiet)
            std::cout << "Elapsed " << formatDuration(t2 - t1) << std::endl;

        if (outputPath == "stdout") {
            formatter->format(std::cout, solution);
        }
        else {
            std::string replacedPath = replaceTemplates(outputPath, solution);
            std::ofstream fout(replacedPath);
            formatter->format(fout, solution);
            fout.close();

            if (!quiet)
                std::cout << "Solution saved to \"" << replacedPath << "\"" << std::endl;
        }
    }
    catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << std::endl;
        return -1;
    }

    return 0;
}
