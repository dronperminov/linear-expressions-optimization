#include "leo/utils/solution_transposer.h"

namespace leo::utils {

Solution SolutionTransposer::transpose(const Solution& solution) const {
    size_t dimensionT = solution.expressions.size();

    std::vector<Vector> vectors, vectorsT;
    std::unordered_map<Vector, size_t> vector2index, vector2indexT;

    initializeBasisVectors(solution.dimension, vectors, vector2index);
    initializeBasisVectors(dimensionT, vectorsT, vector2indexT);
    initializeVectors(solution.substitutions, vectors, vector2index);

    std::vector<Substitution> substitutions = solution.substitutions;
    std::vector<Substitution> substitutionsT;
    std::vector<Vector> outputs = initializeOutputs(solution, vectors, vector2index, substitutions);
    std::vector<std::optional<Term>> accumulators(vectors.size(), std::nullopt);

    for (size_t i = 0; i < outputs.size(); i++) {
        if (outputs[i].isZero())
            continue;

        Term term = getVectorTerm(outputs[i], vectors, vector2index);
        accumulators[term.index] = merge(accumulators[term.index], {i, term.value}, substitutionsT, vectorsT, vector2indexT, dimensionT);
    }

    for (size_t i = 0; i < substitutions.size(); i++) {
        if (!accumulators[accumulators.size() - 1 - i])
            continue;

        Substitution s = substitutions[substitutions.size() - 1 - i];
        Term src = *accumulators[accumulators.size() - 1 - i];

        accumulators[s.i] = merge(accumulators[s.i], {src.index, src.value * s.ai}, substitutionsT, vectorsT, vector2indexT, dimensionT);
        accumulators[s.j] = merge(accumulators[s.j], {src.index, src.value * s.aj}, substitutionsT, vectorsT, vector2indexT, dimensionT);
    }

    std::vector<std::vector<Term>> expressionsT = getExpressions(outputs, vectorsT, vector2indexT, solution.dimension, dimensionT);
    return {dimensionT, substitutionsT, expressionsT};
}

void SolutionTransposer::initializeBasisVectors(size_t dimension, std::vector<Vector>& vectors, std::unordered_map<Vector, size_t>& vector2index) const {
    for (size_t i = 0; i < dimension; i++) {
        Vector basis(dimension, i);
        vector2index[basis.getCanonized()] = i;
        vectors.emplace_back(basis);
    }
}

void SolutionTransposer::initializeVectors(const std::vector<Substitution>& substitutions, std::vector<Vector>& vectors, std::unordered_map<Vector, size_t>& vector2index) const {
    for (const Substitution& s : substitutions) {
        if (s.i >= vectors.size() || s.j >= vectors.size())
            throw std::runtime_error("SolutionTransposer::initializeVectors: substitution uses future steps");

        Vector vector = vectors[s.i] * s.ai + vectors[s.j] * s.aj;
        Vector canonized = vector.getCanonized();

        if (vector2index.find(canonized) != vector2index.end())
            throw std::runtime_error("SolutionTransposer::initializeVectors: substitutions produce duplicate vectors");

        vector2index[canonized] = vector2index.size();
        vectors.emplace_back(vector);
    }
}

std::vector<Vector> SolutionTransposer::initializeOutputs(const Solution& solution, std::vector<Vector>& vectors, std::unordered_map<Vector, size_t>& vector2index, std::vector<Substitution>& substitutions) const {
    std::vector<Vector> outputs;

    for (const std::vector<Term>& expression : solution.expressions) {
        if (expression.size() == 0) {
            outputs.emplace_back(Vector(solution.dimension));
            continue;
        }

        Term prev = expression[0];

        for (size_t index = 1; index < expression.size(); index++) {
            Term curr = expression[index];

            Vector vector = vectors[prev.index] * prev.value + vectors[curr.index] * curr.value;
            Vector canonized = vector.getCanonized();

            auto result = vector2index.find(canonized);
            if (result == vector2index.end()) {
                substitutions.push_back({prev.index, curr.index, prev.value, curr.value});
                vectors.emplace_back(vector);
                vector2index[canonized] = vector2index.size();
                prev.index = vectors.size() - 1;
            }
            else {
                prev.index = result->second;
            }

            prev.value = 1;
        }

        outputs.emplace_back(vectors[prev.index] * prev.value);
    }

    return outputs;
}

std::optional<Term> SolutionTransposer::merge(const std::optional<Term>& accumulator, const Term& src, std::vector<Substitution>& substitutions, std::vector<Vector>& vectors, std::unordered_map<Vector, size_t>& vector2index, size_t dimension) const {
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

    Vector canonized = vector.getCanonized();
    if (vector.getSupport() == 1 || vector2index.find(canonized) != vector2index.end())
        return getVectorTerm(vector, vectors, vector2index);

    substitutions.push_back({i, j, ai, aj});
    vector2index[canonized] = vector2index.size();
    vectors.emplace_back(vector);

    return Term{dimension + substitutions.size() - 1, 1};
}

Term SolutionTransposer::getVectorTerm(const Vector& vector, const std::vector<Vector>& vectors, const std::unordered_map<Vector, size_t>& vector2index) const {
    if (vector.getSupport() == 1) {
        size_t index = vector.getNonZeroIndex();
        return {index, vector[index]};
    }

    Vector canonized = vector.getCanonized();
    size_t index = vector2index.at(canonized);
    int value = vector.compare(vectors[index]);
    return {index, value};
}

std::vector<std::vector<Term>> SolutionTransposer::getExpressions(const std::vector<Vector>& outputs, const std::vector<Vector>& vectors, const std::unordered_map<Vector, size_t>& vector2index, size_t dimension, size_t dimensionT) const {
    std::vector<std::vector<Term>> expressions;
    for (size_t i = 0; i < dimension; i++) {
        Vector target(dimensionT);

        for (size_t j = 0; j < dimensionT; j++)
            target[j] = outputs[j][i];

        if (target.isZero()) {
            expressions.push_back({});
            continue;
        }

        Term term = getVectorTerm(target, vectors, vector2index);
        expressions.push_back({term});
    }

    return expressions;
}

}
