#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/term.h>
#include <leo/entities/vector.h>

namespace leo::utils {

class SolutionTransposer {
public:
    Solution transpose(const Solution& solution) const;
private:
    void initializeBasisVectors(size_t dimension, std::vector<Vector>& vectors, std::unordered_map<Vector, size_t>& vector2index) const;
    void initializeVectors(const std::vector<Substitution>& substitutions, std::vector<Vector>& vectors, std::unordered_map<Vector, size_t>& vector2index) const;
    std::vector<Vector> initializeOutputs(const Solution& solution, std::vector<Vector>& vectors, std::unordered_map<Vector, size_t>& vector2index, std::vector<Substitution>& substitutions) const;
    std::optional<Term> merge(const std::optional<Term>& accumulator, const Term& src, std::vector<Substitution>& substitutions, std::vector<Vector>& vectors, std::unordered_map<Vector, size_t>& vector2index, size_t dimension) const;
    Term getVectorTerm(const Vector& vector, const std::vector<Vector>& vectors, const std::unordered_map<Vector, size_t>& vector2index) const;
    std::vector<std::vector<Term>> getExpressions(const std::vector<Vector>& outputs, const std::vector<Vector>& vectors, const std::unordered_map<Vector, size_t>& vector2index, size_t dimension, size_t dimensionT) const;
};

}
