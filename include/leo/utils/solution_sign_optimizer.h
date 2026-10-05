#pragma once

#include <cstddef>
#include <random>
#include <vector>

#include <leo/entities/solution.h>

namespace leo {

class SolutionSignOptimizer {
    struct SubstitutionUsage {
        std::vector<size_t> substitutions;
        std::vector<size_t> expressions;
    };

    double startTemp;
    double endTemp;
    size_t stepsPerVariable;
public:
    SolutionSignOptimizer(double startTemp = 1.0, double endTemp = 0.05, size_t stepsPerVariable = 100);

    Solution optimize(const Solution& solution, std::mt19937& generator, size_t restarts) const;
private:
    std::vector<SubstitutionUsage> getUsage(const Solution& solution) const;

    int getSign(size_t index, size_t dimension, const std::vector<int>& signs) const;
    int getInversions(const Solution& solution, const std::vector<int>& signs) const;
    int getAffectedInversions(const Solution& solution, size_t index, const std::vector<SubstitutionUsage>& usage, const std::vector<int>& signs) const;

    bool isSubstitutionInverted(const Solution& solution, size_t index, const std::vector<int>& signs) const;
    bool isExpressionInverted(const Solution& solution, size_t index, const std::vector<int>& signs) const;

    Solution apply(const Solution& solution, const std::vector<int> &signs) const;
};

} // namespace leo
