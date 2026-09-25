#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/term.h>
#include <leo/entities/vector.h>
#include <leo/entities/vector_index.h>

namespace leo::utils {

class SolutionTransposer {
public:
    Solution transpose(const Solution& solution) const;
private:
    std::vector<Vector> initializeOutputs(const Solution& solution, VectorIndex& vectorIndex, std::vector<Substitution>& substitutions) const;
    std::optional<Term> merge(const std::optional<Term>& accumulator, const Term& src, std::vector<Substitution>& substitutions, VectorIndex& vectorIndex, size_t dimension) const;
    Term getVectorTerm(const Vector& vector, const VectorIndex& vectorIndex) const;
    std::vector<std::vector<Term>> getExpressions(const std::vector<Vector>& outputs, const VectorIndex& vectorIndex, size_t dimension, size_t dimensionT) const;
};

} // namespace leo
