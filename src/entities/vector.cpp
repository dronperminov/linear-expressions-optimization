#include <leo/entities/vector.h>

namespace leo {

Vector::Vector(const std::vector<int>& values) : values(values), canonized(values.size()) {
    canonize();
}

Vector::Vector(size_t dimension, size_t index) : values(dimension, 0), canonized(dimension, 0) {
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

int Vector::operator[](size_t index) const {
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

} // namespace leo
