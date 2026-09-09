#pragma once

#include <iostream>
#include <vector>

class Vector {
    std::vector<int> values;
    std::vector<int> canonized;
public:
    Vector(const std::vector<int>& values);
    Vector(int dimension, int index);

    Vector operator+(const Vector& vector) const;
    Vector operator-(const Vector& vector) const;
    Vector operator-() const;
    Vector operator*(int scale) const;

    int operator[](int index) const;
    size_t getHash() const;

    bool operator==(const Vector& vector) const;
    bool operator!=(const Vector& vector) const;
    bool operator<(const Vector& vector) const;
    int compare(const Vector& vector) const; 

    int getDimension() const;
    int getSupport() const;
    int getMaxAbs() const;
    int getHammingDistance(const Vector& vector) const;
    int getMatchesCount(const Vector& vector) const;
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
