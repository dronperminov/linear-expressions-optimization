#include "leo/utils/solution_transposer.h"

namespace leo::utils {

Solution SolutionTransposer::transpose(const Solution& solution) const {
    size_t dimensionT = solution.expressions.size();

    VectorIndex vectors(solution.dimension);
    VectorIndex vectorsT(dimensionT);

    for (const Substitution& substitution : solution.substitutions)
        vectors.add(substitution);

    std::vector<Substitution> substitutions = solution.substitutions;
    std::vector<Substitution> substitutionsT;
    std::vector<Vector> outputs = initializeOutputs(solution, vectors, substitutions);
    std::vector<std::optional<Term>> accumulators(vectors.size(), std::nullopt);

    for (size_t i = 0; i < outputs.size(); i++) {
        if (outputs[i].isZero())
            continue;

        Term term = getVectorTerm(outputs[i], vectors);
        accumulators[term.index] = merge(accumulators[term.index], {i, term.value}, substitutionsT, vectorsT, dimensionT);
    }

    for (size_t i = 0; i < substitutions.size(); i++) {
        if (!accumulators[accumulators.size() - 1 - i])
            continue;

        Substitution s = substitutions[substitutions.size() - 1 - i];
        Term src = *accumulators[accumulators.size() - 1 - i];

        accumulators[s.i] = merge(accumulators[s.i], {src.index, src.value * s.ai}, substitutionsT, vectorsT, dimensionT);
        accumulators[s.j] = merge(accumulators[s.j], {src.index, src.value * s.aj}, substitutionsT, vectorsT, dimensionT);
    }

    std::vector<std::vector<Term>> expressionsT = getExpressions(outputs, vectorsT, solution.dimension, dimensionT);
    return {dimensionT, substitutionsT, expressionsT};
}

std::vector<Vector> SolutionTransposer::initializeOutputs(const Solution& solution, VectorIndex& vectors, std::vector<Substitution>& substitutions) const {
    std::vector<Vector> outputs;

    for (const std::vector<Term>& expression : solution.expressions) {
        if (expression.empty()) {
            outputs.emplace_back(Vector(solution.dimension));
            continue;
        }

        Term prev = expression[0];

        for (size_t index = 1; index < expression.size(); index++) {
            Term curr = expression[index];

            Vector vector = vectors[prev.index] * prev.value + vectors[curr.index] * curr.value;
            std::pair<size_t, bool> result = vectors.getOrAdd(vector);

            if (result.second) {
                substitutions.push_back({prev.index, curr.index, prev.value, curr.value});
                prev.index = result.first;
                prev.value = 1;
            }
            else {
                prev.index = result.first;
                prev.value = vectors[prev.index].compare(vector);
            }
        }

        outputs.emplace_back(vectors[prev.index] * prev.value);
    }

    return outputs;
}

std::optional<Term> SolutionTransposer::merge(const std::optional<Term>& accumulator, const Term& src, std::vector<Substitution>& substitutions, VectorIndex& vectors, size_t dimension) const {
    if (!accumulator)
        return Term{src.index, src.value};

    Term dst = *accumulator;

    size_t i = dst.index;
    size_t j = src.index;
    int ai = dst.value;
    int aj = src.value;

    if (dst.value < 0) {
        std::swap(i, j);
        std::swap(ai, aj);
    }

    Vector vector = vectors[i] * ai + vectors[j] * aj;
    if (vector.isZero())
        return std::nullopt;

    if (vector.getSupport() == 1 || vectors.contains(vector))
        return getVectorTerm(vector, vectors);

    substitutions.push_back({i, j, ai, aj});
    vectors.add(vector);

    return Term{dimension + substitutions.size() - 1, 1};
}

Term SolutionTransposer::getVectorTerm(const Vector& vector, const VectorIndex& vectors) const {
    if (vector.getSupport() == 1) {
        size_t index = vector.getNonZeroIndex();
        return {index, vector[index]};
    }

    size_t index = vectors.getIndex(vector);
    int value = vector.compare(vectors[index]);
    return {index, value};
}

std::vector<std::vector<Term>> SolutionTransposer::getExpressions(const std::vector<Vector>& outputs, const VectorIndex& vectors, size_t dimension, size_t dimensionT) const {
    std::vector<std::vector<Term>> expressions;
    for (size_t i = 0; i < dimension; i++) {
        Vector target(dimensionT);

        for (size_t j = 0; j < dimensionT; j++)
            target[j] = outputs[j][i];

        if (target.isZero()) {
            expressions.push_back({});
            continue;
        }

        Term term = getVectorTerm(target, vectors);
        expressions.push_back({term});
    }

    return expressions;
}

} // namespace leo
