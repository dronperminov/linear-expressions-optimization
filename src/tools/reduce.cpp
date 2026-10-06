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

StrategyPool getVectorCoveringStrategies(const ExpressionsSystem& expressionsSystem, const std::string& preset, bool addTargetPairs) {
    if (preset == "default")
        return presets::vectorCoveringDefault(expressionsSystem, addTargetPairs);

    if (preset == "distance")
        return presets::vectorCoveringDistance(expressionsSystem, addTargetPairs);

    return presets::vectorCoveringAll(expressionsSystem, addTargetPairs);
}

StrategyPool getCommonSubexpressionStrategies(const std::string& preset) {
    if (preset == "vanilla")
        return presets::cseVanilla();

    if (preset == "potential")
        return presets::csePotential();

    if (preset == "intersections")
        return presets::cseIntersections();

    return presets::cseAll();
}

TaskPool initTasks(const ExpressionsSystem& expressionsSystem, const ArgParser& parser, std::mt19937& generator) {
    TaskPool tasks;

    size_t vecIterations = std::stoull(parser["--vec-iterations"]);
    StrategyPool vecStrategies = getVectorCoveringStrategies(expressionsSystem, parser["--vec-preset"], parser.isSet("--vec-add-target-pairs"));

    if (parser["--vec-sampling"] == "sample")
        tasks.add(vecStrategies.sample(vecIterations, generator));
    else
        tasks.add(vecStrategies.each(vecIterations, generator));

    size_t cseIterations = std::stoull(parser["--cse-iterations"]);
    StrategyPool cseStrategies = getCommonSubexpressionStrategies(parser["--cse-preset"]);

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

    reducer.reduce(pools, parser.isSet("--bound-by-best"));

    Solution solution = reducer.getSolution(0);

    if (transposedSystem) {
        SolutionTransposer transposer;
        Solution transposed = transposer.transpose(reducer.getSolution(1));

        if (transposed.getAdditions() < solution.getAdditions())
            solution = transposed;
    }

    if (!parser.isSet("--quiet")) {
        std::cout << "Solution:" << std::endl;
        std::cout << "- solution has " << solution.getAdditions() << " additions" << std::endl;
    }

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
        std::cout << "- number of substitutions ";

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
        std::cout << "- number of sign inversions ";

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
    parser.add("--quiet", "-q", ArgType::Flag, "Suppress all output to stdout (explicit --print-* options still print)");

    parser.addSection("Input / output");
    parser.add("--input-path", "-i", ArgType::Path, "Input file with linear expressions", "", true);
    parser.add("--output-path", "-o", ArgType::Path, "Output file path, or \"stdout\" to print the solution", "output.txt");
    parser.addChoices("--format", "-f", ArgType::String, "Output format for the solution", {"slp", "txt", "json", "auto"}, "auto");

    parser.addSection("Solving strategy");
    parser.add("--bound-by-best", "-b", ArgType::Flag, "Use the best solution found so far as an upper bound for subsequent solvers");
    parser.add("--try-transpose", "-T", ArgType::Flag, "Additionally try solving the transposed system, then transpose the solution back");

    parser.addSection("Vector covering solver");
    parser.add("--vec-add-target-pairs", ArgType::Flag, "Precompute target vectors reachable with one addition or subtraction (xi +/- xj)");
    parser.addChoices("--vec-preset", ArgType::String, "Strategues preset", {"default", "distance", "all"}, "all");
    parser.add("--vec-iterations", ArgType::UInt, "Number of iterations", "10");
    parser.addChoices("--vec-sampling", ArgType::String, "\"sample\": random strategy per iteration; \"each\": every strategy, all iterations", {"sample", "each"}, "sample");

    parser.addSection("Common subexpression (CSE) solver");
    parser.addChoices("--cse-preset", ArgType::String, "Strategues preset", {"vanilla", "potential", "intersections", "all"}, "all");
    parser.add("--cse-iterations", ArgType::UInt, "Number of iterations", "100");
    parser.addChoices("--cse-sampling", ArgType::String, "\"sample\": random strategy per iteration; \"each\": every strategy, all iterations", {"sample", "each"}, "sample");

    parser.addSection("Post-processing");
    parser.add("--inline-substitutions", ArgType::Flag, "Inline substitutions that occur only once");
    parser.add("--optimize-signs-iterations", ArgType::UInt, "Number of iterations of the sign inversion optimizer; 0 disables it", "0");
    parser.add("--validate", ArgType::Flag, "Validate the resulting solution");

    parser.addSection("Diagnostics");
    parser.add("--print-args", ArgType::Flag, "Print parsed command-line arguments to stdout");
    parser.add("--print-system-stats", ArgType::Flag, "Print statistics of the parsed system of expressions");

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
    std::string format = parser["--format"];

    size_t cseIterations = std::stoull(parser["--cse-iterations"]);
    size_t vecIterations = std::stoull(parser["--vec-iterations"]);

    if (parser.isSet("--print-args")) {
        std::cout << "Parsed parameters:" << std::endl;
        std::cout << "- threads: " << threads << std::endl;
        std::cout << "- random seed: " << seed << std::endl;
        std::cout << std::endl;
        std::cout << "Input / output:" << std::endl;
        std::cout << "- input path: " << inputPath << std::endl;
        std::cout << "- output path: " << outputPath << std::endl;
        std::cout << "- output format: " << format << std::endl;
        std::cout << std::endl;
        std::cout << "Solving strategy:" << std::endl;
        std::cout << "- try transposing: " << (parser.isSet("--try-transpose") ? "yes" : "no") << std::endl;
        std::cout << "- bound by best: " << (parser.isSet("--bound-by-best") ? "yes" : "no") << std::endl;
        std::cout << std::endl;
        std::cout << "- Vector covering solver:";
        if (vecIterations > 0) {
            std::cout << std::endl;
            std::cout << "  - add target pairs: " << (parser.isSet("--vec-add-target-pairs") ? "yes" : "no") << std::endl;
            std::cout << "  - preset: " << parser["--vec-preset"] << std::endl;
            std::cout << "  - iterations: " << vecIterations << std::endl;
            std::cout << "  - sampling strategy: " << parser["--vec-sampling"] << std::endl;
        }
        else {
            std::cout << " not used" << std::endl;
        }

        std::cout << std::endl;
        std::cout << "- Common subexpression (CSE) solver:";
        if (cseIterations) {
            std::cout << std::endl;
            std::cout << "  - preset: " << parser["--cse-preset"] << std::endl;
            std::cout << "  - iterations: " << cseIterations << std::endl;
            std::cout << "  - sampling strategy: " << parser["--cse-sampling"] << std::endl;
        }
        else {
            std::cout << " not used" << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Postprocessing:" << std::endl;
        std::cout << "- inline substitutions: " << (parser.isSet("--inline-substitutions") ? "yes" : "no") << std::endl;
        std::cout << "- optimize inversions: " << (std::stoul(parser["--optimize-signs-iterations"]) > 0 ? "yes (" + parser["--optimize-signs-iterations"] + " iterations)" : "no") << std::endl;
        std::cout << "- validate solution: " << (parser.isSet("--validate") ? "yes" : "no") << std::endl;
        std::cout << std::endl;
    }

    std::mt19937 generator(seed);

    try {
        std::unique_ptr<SolutionFormatter> formatter = getFormatter(outputPath, format);
        std::unique_ptr<ExpressionsReader> reader = getExpressionsReader(inputPath);

        ExpressionsSystem expressionsSystem = reader->read(inputPath);

        if (parser.isSet("--print-system-stats")) {
            std::cout << "Read system of expressions:" << std::endl;
            expressionsSystem.describe(std::cout);
            std::cout << std::endl;
        }

        auto t1 = std::chrono::steady_clock::now();
        Solution solution = reduce(expressionsSystem, parser, generator);

        if (parser.isSet("--inline-substitutions"))
            solution = inlineSubstitutions(solution, parser);

        if (std::stoull(parser["--optimize-signs-iterations"]) > 0)
            solution = optimizeInversions(solution, parser, generator);

        auto t2 = std::chrono::steady_clock::now();

        if (parser.isSet("--validate")) {
            if (!expressionsSystem.validateSolution(solution))
                throw std::runtime_error("solution is not valid");

            if (!quiet)
                std::cout << "- solution is valid" << std::endl;
        }

        if (!quiet)
            std::cout << "- elapsed " << formatDuration(t2 - t1) << std::endl;

        if (outputPath == "stdout") {
            formatter->format(std::cout, solution);
        }
        else {
            std::string replacedPath = replaceTemplates(outputPath, solution);
            std::ofstream fout(replacedPath);
            formatter->format(fout, solution);
            fout.close();

            if (!quiet)
                std::cout << "- saved to \"" << replacedPath << "\"" << std::endl;
        }
    }
    catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << std::endl;
        return -1;
    }

    return 0;
}
