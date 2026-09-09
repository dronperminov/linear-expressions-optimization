#pragma once

#include <iostream>
#include <vector>

class Vector {
    std::vector<int> values;
    std::vector<int> canonized;
public:
    Vector(const std::vector<int>& values);
    Vector(size_t dimension, size_t index);

    Vector operator+(const Vector& vector) const;
    Vector operator-(const Vector& vector) const;
    Vector operator-() const;
    Vector operator*(int scale) const;

    int operator[](size_t index) const;
    size_t getHash() const;

    bool operator==(const Vector& vector) const;
    bool operator!=(const Vector& vector) const;
    bool operator<(const Vector& vector) const;
    int compare(const Vector& vector) const; 

    size_t getDimension() const;
    size_t getSupport() const;
    int getMaxAbs() const;
    size_t getHammingDistance(const Vector& vector) const;
    size_t getMatchesCount(const Vector& vector) const;
private:
    void canonize();
};

namespace std {
    template <>
    struct hash<Vector> {
        size_t operator()(const Vector& vector) const {
            return vector.getHash();
        }
    };
}
