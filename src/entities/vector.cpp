#include "vector.h"

Vector::Vector(const std::vector<int>& values) : canonized(values.size()) {
    this->values = values;
    canonize();
}

Vector::Vector(int dimension, int index) : values(dimension, 0), canonized(dimension, 0) {
    values[index] = 1;
    canonized[index] = 1;
}

Vector Vector::operator+(const Vector& vector) const {
    std::vector<int> result(values.size());

    for (size_t i = 0; i < values.size(); i++)
        result[i] = values[i] + vector.values[i];

    return Vector(result);
}

Vector Vector::operator-(const Vector& vector) const {
    std::vector<int> result(values.size());

    for (size_t i = 0; i < values.size(); i++)
        result[i] = values[i] - vector.values[i];

    return Vector(result);
}

Vector Vector::operator-() const {
    std::vector<int> result(values.size());

    for (size_t i = 0; i < values.size(); i++)
        result[i] = -values[i];

    return Vector(result);
}

Vector Vector::operator*(int scale) const {
    std::vector<int> result(values.size());

    for (size_t i = 0; i < values.size(); i++)
        result[i] = values[i] * scale;

    return Vector(result);
}

int Vector::operator[](int index) const {
    return values[index];
}

size_t Vector::getHash() const {
    size_t hash = 0;

    for (int value : canonized)
        hash ^= std::hash<int>{}(value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

    return hash;
}

bool Vector::operator==(const Vector& vector) const {
    return canonized == vector.canonized;
}

bool Vector::operator!=(const Vector& vector) const {
    return !(*this == vector);
}

bool Vector::operator<(const Vector& vector) const {
    for (size_t i = 0; i < values.size(); i++)
        if (values[i] != vector.values[i])
            return values[i] < vector.values[i];

    return false;
}

int Vector::compare(const Vector& vector) const {
    if (values == vector.values)
        return 1;
    
    if (canonized == vector.canonized)
        return -1;

    return 0;
}

int Vector::getDimension() const {
    return (int) values.size();
}

int Vector::getSupport() const {
    int support = 0;

    for (int value : values)
        support += value != 0;

    return support;
}

int Vector::getMaxAbs() const {
    int max = 0;

    for (int value : values)
        max = std::max(max, std::abs(value));

    return max;
}

int Vector::getHammingDistance(const Vector& vector) const {
    int distance = 0;
    int inverse = 0;

    for (size_t i = 0; i < values.size(); i++) {
        if (values[i] != vector.values[i])
            distance++;

        if (values[i] != -vector.values[i])
            inverse++;
    }

    return std::min(distance, inverse);
}

int Vector::getMatchesCount(const Vector& vector) const {
    int matches = 0;

    for (size_t i = 0; i < values.size(); i++)
        matches += values[i] != 0 && values[i] == vector.values[i];

    return matches;
}

void Vector::canonize() {
    int sign = 1;
    bool first = false;

    for (size_t i = 0; i < values.size(); i++) {
        int value = values[i];

        if (!first && value != 0) {
            first = true;
            sign = value > 0 ? 1 : -1;
        }

        canonized[i] = value * sign;
    }
}
