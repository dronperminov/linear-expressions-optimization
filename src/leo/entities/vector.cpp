#include <leo/entities/vector.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>

namespace leo {

Vector::Vector(const std::vector<int>& values) : values(values) {

}

Vector::Vector(size_t dimension) : values(dimension, 0) {

}

Vector::Vector(size_t dimension, size_t index) : values(dimension, 0) {
    values[index] = 1;
}

void Vector::set(size_t index, int value) {
    values[index] = value;
}

int Vector::get(size_t index) const {
    return values[index];
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

Vector& Vector::operator+=(const Vector& vector) {
    for (size_t i = 0; i < values.size(); i++)
        values[i] += vector.values[i];

    return *this;
}

Vector& Vector::operator-=(const Vector& vector) {
    for (size_t i = 0; i < values.size(); i++)
        values[i] -= vector.values[i];

    return *this;
}

Vector& Vector::operator*=(int scale) {
    for (size_t i = 0; i < values.size(); i++)
        values[i] *= scale;

    return *this;
}

Vector Vector::getCanonized() const {
    Vector canonized(values);
    canonized.canonize();
    return canonized;
}

Vector Vector::addScaled(const Vector& vector, int scale) const {
    std::vector<int> result(values.size());

    for (size_t i = 0; i < values.size(); i++)
        result[i] = values[i] + vector.values[i] * scale;

    return Vector(result);
}

int Vector::operator[](size_t index) const {
    return values[index];
}

size_t Vector::getHash() const {
    size_t hash = 0;

    for (int value : values)
        hash ^= std::hash<int>{}(value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

    return hash;
}

bool Vector::operator==(const Vector& vector) const {
    return values == vector.values;
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

    for (size_t i = 0; i < values.size(); i++)
        if (values[i] != -vector.values[i])
            return 0;

    return -1;
}

size_t Vector::getDimension() const {
    return values.size();
}

size_t Vector::getSupport() const {
    size_t support = 0;

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

size_t Vector::getDistance(const Vector& vector) const {
    size_t distance = 0;
    size_t inverse = 0;

    for (size_t i = 0; i < values.size(); i++) {
        distance += std::abs(values[i] - vector.values[i]);
        inverse += std::abs(values[i] + vector.values[i]);
    }

    return std::min(distance, inverse);
}

size_t Vector::getHammingDistance(const Vector& vector) const {
    size_t distance = 0;
    size_t inverse = 0;

    for (size_t i = 0; i < values.size(); i++) {
        if (values[i] != vector.values[i])
            distance++;

        if (values[i] != -vector.values[i])
            inverse++;
    }

    return std::min(distance, inverse);
}

size_t Vector::getMatchesCount(const Vector& vector) const {
    size_t matches = 0;
    size_t inverse = 0;

    for (size_t i = 0; i < values.size(); i++) {
        if (values[i] == 0)
            continue;

        matches += values[i] == vector.values[i];
        inverse += values[i] == -vector.values[i];
    }

    return std::max(matches, inverse);
}

std::vector<size_t> Vector::getNonZeroIndices() const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < values.size(); i++)
        if (values[i])
            indices.push_back(i);

    return indices;
}

size_t Vector::getNonZeroIndex() const {
    for (size_t i = 0; i < values.size(); i++)
        if (values[i])
            return i;

    throw std::runtime_error("Vector::getNonZeroIndex: vector is zero");
}

uint64_t Vector::getFingerprint() const {
    uint64_t hash = 0;

    for (size_t i = 0; i < values.size(); i++)
        hash += values[i] * (mix(i + 1) | 1);

    return hash;
}

bool Vector::isZero() const {
    for (int value : values)
        if (value)
            return false;

    return true;
}

bool Vector::isOneHot() const {
    int sum = 0;

    for (int value : values)
        sum += std::abs(value);

    return sum == 1;
}

bool Vector::isSubVector(const Vector& vector) const {
    int sign = 0;

    for (size_t i = 0; i < values.size(); i++) {
        int value = values[i];
        if (value == 0)
            continue;

        int target = vector.values[i];

        if (sign == 0) {
            sign = (value == target) ? 1 : (value == -target) ? -1 : 0;

            if (sign == 0)
                return false;
        }
        else if (value != sign * target) {
            return false;
        }
    }

    return true;
}

void Vector::canonize() {
    size_t i = 0;
    while (i < values.size() && values[i] == 0)
        i++;

    if (i == values.size() || values[i] > 0)
        return;

    for (size_t j = i; j < values.size(); j++)
        values[j] = -values[j];
}

uint64_t Vector::mix(uint64_t x) const {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

} // namespace leo
