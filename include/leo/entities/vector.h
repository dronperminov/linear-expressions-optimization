#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace leo {

class Vector {
    std::vector<int> values;
public:
    Vector(const std::vector<int>& values);
    Vector(size_t dimension);
    Vector(size_t dimension, size_t index);

    void set(size_t index, int value);
    int get(size_t index) const;

    Vector operator+(const Vector& vector) const;
    Vector operator-(const Vector& vector) const;
    Vector operator-() const;
    Vector operator*(int scale) const;

    Vector& operator+=(const Vector& vector);
    Vector& operator-=(const Vector& vector);
    Vector& operator*=(int scale);

    Vector getCanonized() const;
    Vector addScaled(const Vector& vector, int scale) const;

    int operator[](size_t index) const;
    size_t getHash() const;

    bool operator==(const Vector& vector) const;
    bool operator!=(const Vector& vector) const;
    bool operator<(const Vector& vector) const;
    int compare(const Vector& vector) const; 

    size_t getDimension() const;
    size_t getSupport() const;
    int getMaxAbs() const;
    size_t getDistance(const Vector& vector) const;
    size_t getHammingDistance(const Vector& vector) const;
    size_t getMatchesCount(const Vector& vector) const;
    std::vector<size_t> getNonZeroIndices() const;
    size_t getNonZeroIndex() const;
    uint64_t getFingerprint() const;

    bool isZero() const;
    bool isOneHot() const;
    bool isSubVector(const Vector& vector) const;

    void canonize();
private:
    uint64_t mix(uint64_t x) const;
};

} // namespace leo

namespace std {
    template <>
    struct hash<leo::Vector> {
        size_t operator()(const leo::Vector& vector) const {
            return vector.getHash();
        }
    };
}
