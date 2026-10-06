#pragma once

#include <cstddef>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/term.h>

namespace leo {

class SolutionSubstitutionInliner {
    struct Usage {
        size_t substitutionsCount;
        size_t expressionsCount;
        size_t row;
        size_t column;
    };
public:
    Solution optimize(const Solution& solution) const;
private:
    std::vector<Usage> getUsage(const Solution& solution) const;
    std::vector<bool> getInlinable(const std::vector<Usage>& usage) const;
    std::vector<std::vector<Term>> getSubstitutionExpressions(const Solution& solution) const;

    void updateUsage(std::vector<Usage>& usage, size_t index, size_t dimension, size_t row, size_t column, bool byExpressions) const;
    void inlineTerm(std::vector<Term>& expression, size_t column, const std::vector<Term>& replacement) const;
    void normalize(std::vector<Term>& expression, const std::vector<size_t>& indices, size_t dimension) const;

    size_t remap(size_t index, size_t dimension, const std::vector<size_t>& indices) const;
};

} // namespace leo
