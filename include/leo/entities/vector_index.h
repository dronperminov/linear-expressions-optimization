#pragma once

#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <leo/entities/substitution.h>
#include <leo/entities/vector.h>

namespace leo {

class VectorIndex {
    std::vector<Vector> vectors;
    std::unordered_map<Vector, size_t> vector2index;
public:
    VectorIndex(size_t dimension);

    size_t size() const;

    void add(const Vector& vector);
    void add(const Substitution& substitution);

    std::pair<size_t, bool> getOrAdd(const Vector& vector);

    size_t getIndex(const Vector& vector) const;
    bool contains(const Vector& vector) const;

    const Vector& operator[](size_t index) const;
};

} // namespace leo
