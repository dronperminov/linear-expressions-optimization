#include <leo/entities/vector_index.h>

namespace leo {

VectorIndex::VectorIndex(size_t dimension) {
    for (size_t i = 0; i < dimension; i++) {
        Vector basis(dimension, i);
        vector2index[basis.getCanonized()] = i;
        vectors.emplace_back(basis);
    }
}

size_t VectorIndex::size() const {
    return vectors.size();
}

void VectorIndex::add(const Vector& vector) {
    Vector canonized = vector.getCanonized();

    if (vector2index.find(canonized) != vector2index.end())
        throw std::runtime_error("VectorIndex::addSubstitution: substitutions produce duplicate vectors");

    vector2index[canonized] = vectors.size();
    vectors.emplace_back(vector);
}

void VectorIndex::add(const Substitution& substitution) {
    if (substitution.i >= vectors.size() || substitution.j >= vectors.size())
        throw std::runtime_error("VectorIndex::addSubstitution: substitution uses future steps");

    add(vectors[substitution.i] * substitution.ai + vectors[substitution.j] * substitution.aj);
}

std::pair<size_t, bool> VectorIndex::getOrAdd(const Vector& vector) {
    Vector canonized = vector.getCanonized();
    auto result = vector2index.find(canonized);
    if (result != vector2index.end())
        return {result->second, false};

    vector2index[canonized] = vectors.size();
    vectors.emplace_back(vector);
    return {vectors.size() - 1, true};
}

size_t VectorIndex::getIndex(const Vector& vector) const {
    Vector canonized = vector.getCanonized();
    auto result = vector2index.find(canonized);
    if (result != vector2index.end())
        return result->second;

    throw std::runtime_error("VectorIndex::getIndex: vector is not presented");
}

bool VectorIndex::contains(const Vector& vector) const {
    return vector2index.find(vector.getCanonized()) != vector2index.end();
}

const Vector& VectorIndex::operator[](size_t index) const {
    return vectors[index];
}

} // namespace leo
