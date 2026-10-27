#include <leo/entities/ternary_vector.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>

namespace leo {

TernaryVector::TernaryVector(const std::vector<int>& values) : dimension(values.size()) {
    size_t size = (dimension + 63) / 64;
    digits.assign(size, 0);
    signs.assign(size, 0);
    valid = true;

    for (size_t i = 0; i < dimension; i++)
        set(i, values[i]);
}

TernaryVector::TernaryVector(size_t dimension) : dimension(dimension) {
    size_t size = (dimension + 63) / 64;
    digits.assign(size, 0);
    signs.assign(size, 0);
    valid = true;
}

TernaryVector::TernaryVector(size_t dimension, size_t index) : dimension(dimension) {
    size_t size = (dimension + 63) / 64;
    digits.assign(size, 0);
    signs.assign(size, 0);
    valid = true;

    set(index, 1);
}

TernaryVector TernaryVector::operator+(const TernaryVector& vector) const {
    TernaryVector result(dimension);

    for (size_t i = 0; i < digits.size(); i++) {
        result.digits[i] = digits[i] ^ vector.digits[i];
        result.signs[i] = ((signs[i] & digits[i]) | (vector.signs[i] & vector.digits[i])) & result.digits[i];
        result.valid &= !(digits[i] & vector.digits[i] & ~(signs[i] ^ vector.signs[i]));
    }

    return result;
}

TernaryVector TernaryVector::operator-(const TernaryVector& vector) const {
    TernaryVector result(dimension);

    for (size_t i = 0; i < digits.size(); i++) {
        result.digits[i] = digits[i] ^ vector.digits[i];
        result.signs[i] = ((signs[i] & digits[i]) | (~vector.signs[i] & vector.digits[i])) & result.digits[i];
        result.valid &= !(digits[i] & vector.digits[i] & (signs[i] ^ vector.signs[i]));
    }

    return result;
}

TernaryVector TernaryVector::operator-() const {
    TernaryVector result(dimension);

    for (size_t i = 0; i < digits.size(); i++) {
        result.digits[i] = digits[i];
        result.signs[i] = (~signs[i]) & digits[i];
    }

    result.valid = valid;
    return result;
}

TernaryVector TernaryVector::operator*(int scale) const {
    if (scale == 0)
        return TernaryVector(dimension);

    if (scale == 1)
        return TernaryVector(*this);

    if (scale == -1)
        return -(*this);

    TernaryVector result(dimension);
    result.valid = false;
    return result;
}

TernaryVector& TernaryVector::operator+=(const TernaryVector& vector) {
    for (size_t i = 0; i < digits.size(); i++) {
        uint64_t sv1 = signs[i] & digits[i];
        uint64_t sv2 = vector.signs[i] & vector.digits[i];

        valid &= !((digits[i] & vector.digits[i] & ~(signs[i] ^ vector.signs[i])));
        digits[i] ^= vector.digits[i];
        signs[i] = (sv1 | sv2) & digits[i];
    }

    return *this;
}

TernaryVector& TernaryVector::operator-=(const TernaryVector& vector) {
    for (size_t i = 0; i < digits.size(); i++) {
        uint64_t sv1 = signs[i] & digits[i];
        uint64_t sv2 = ~vector.signs[i] & vector.digits[i];

        valid &= !(digits[i] & vector.digits[i] & (signs[i] ^ vector.signs[i]));
        digits[i] ^= vector.digits[i];
        signs[i] = (sv1 | sv2) & digits[i];
    }

    return *this;
}

TernaryVector& TernaryVector::operator*=(int scale) {
    if (!isZero() && (scale > 1 || scale < -1)) {
        valid = false;
        return *this;
    }

    if (scale == 0) {
        for (size_t i = 0; i < digits.size(); i++) {
            digits[i] = 0;
            signs[i] = 0;
        }
    }
    else if (scale == -1) {
        for (size_t i = 0; i < digits.size(); i++)
            signs[i] = (~signs[i]) & digits[i];
    }

    return *this;
}

TernaryVector TernaryVector::getCanonized() const {
    TernaryVector canonized(*this);
    canonized.canonize();
    return canonized;
}

TernaryVector TernaryVector::addScaled(const TernaryVector& vector, int scale) const {
    return *this + vector * scale;
}

int TernaryVector::operator[](size_t index) const {
    return get(index);
}

size_t TernaryVector::getHash() const {
    size_t hash = 0;

    for (uint64_t digit : digits)
        hash ^= std::hash<uint64_t>{}(digit) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

    return hash;
}

bool TernaryVector::operator==(const TernaryVector& vector) const {
    return digits == vector.digits && signs == vector.signs;
}

bool TernaryVector::operator!=(const TernaryVector& vector) const {
    return !(*this == vector);
}

// TODO
bool TernaryVector::operator<(const TernaryVector& vector) const {
    for (size_t i = 0; i < dimension; i++) {
        int value1 = get(i);
        int value2 = vector.get(i);

        if (value1 != value2)
            return value1 < value2;
    }

    return false;
}

int TernaryVector::compare(const TernaryVector& vector) const {
    if (digits != vector.digits)
        return 0;

    if (signs == vector.signs)
        return 1;

    return -1;
}

size_t TernaryVector::getDimension() const {
    return dimension;
}

size_t TernaryVector::getSupport() const {
    size_t support = 0;

    for (uint64_t value : digits)
        support += __builtin_popcountll(value);

    return support;
}

int TernaryVector::getMaxAbs() const {
    for (uint64_t digit : digits)
        if (digit)
            return 1;

    return 0;
}

// TODO
size_t TernaryVector::getDistance(const TernaryVector& vector) const {
    size_t distance = 0;
    size_t inverse = 0;

    for (size_t i = 0; i < dimension; i++) {
        distance += std::abs(get(i) - vector.get(i));
        inverse += std::abs(get(i) + vector.get(i));
    }

    return std::min(distance, inverse);
}

// TODO
size_t TernaryVector::getHammingDistance(const TernaryVector& vector) const {
    size_t distance = 0;
    size_t inverse = 0;

    for (size_t i = 0; i < dimension; i++) {
        if (get(i) != vector.get(i))
            distance++;

        if (get(i) != -vector.get(i))
            inverse++;
    }

    return std::min(distance, inverse);
}

// TODO
size_t TernaryVector::getMatchesCount(const TernaryVector& vector) const {
    size_t matches = 0;
    size_t inverse = 0;

    for (size_t i = 0; i < dimension; i++) {
        if (get(i) == 0)
            continue;

        matches += get(i) == vector.get(i);
        inverse += get(i) == -vector.get(i);
    }

    return std::max(matches, inverse);
}

std::vector<size_t> TernaryVector::getNonZeroIndices() const {
    std::vector<size_t> indices;

    for (size_t i = 0; i < digits.size(); i++) {
        uint64_t digit = digits[i];

        while (digit > 0) {
            indices.push_back(i * 64 + __builtin_ctzll(digit));
            digit &= (digit - 1);
        }
    }

    return indices;
}

size_t TernaryVector::getNonZeroIndex() const {
    for (size_t i = 0; i < digits.size(); i++) {
        uint64_t digit = digits[i];

        if (digit > 0)
            return i * 64 + __builtin_ctzll(digit);
    }

    throw std::runtime_error("TernaryVector::getNonZeroIndex: vector is zero");
}

uint64_t TernaryVector::getFingerprint() const {
    uint64_t hash = 0;

    for (size_t w = 0; w < digits.size(); w++) {
        uint64_t d = digits[w];

        while (d) {
            unsigned bit = __builtin_ctzll(d);
            d &= d - 1;

            uint64_t weight = mix(w * 64 + bit + 1) | 1;
            hash += ((signs[w] >> bit) & 1) ? (0 - weight) : weight;
        }
    }

    return hash;
}

bool TernaryVector::isValid() const {
    return valid;
}

bool TernaryVector::isZero() const {
    for (uint64_t digit : digits)
        if (digit)
            return false;

    return true;
}

bool TernaryVector::isOneHot() const {
    bool have = false;

    for (uint64_t digit : digits) {
        if (!digit)
            continue;

        if ((digit & (digit - 1)))
            return false;

        if (have)
            return false;

        have = true;
    }

    return have;
}

// TODO
bool TernaryVector::isSubVector(const TernaryVector& vector) const {
    bool positive = true;
    bool negative = true;

    for (size_t i = 0; i < digits.size(); i++) {
        bool eqDigits = digits[i] == (vector.digits[i] & digits[i]);
        positive &= eqDigits && (signs[i] == (vector.signs[i] & digits[i]));
        negative &= eqDigits && (signs[i] == ((~vector.signs[i]) & vector.digits[i] & digits[i]));
    }

    return positive || negative;
}

void TernaryVector::set(size_t index, int value) {
    size_t offset = index / 64;
    uint64_t mask = uint64_t(1) << (index % 64);

    if (value < -1 || value > 1) {
        valid = false;
        return;
    }

    if (value < 0) {
        signs[offset] |= mask;
    }
    else {
        signs[offset] &= ~mask;
    }

    if (value == 0) {
        digits[offset] &= ~mask;
    }
    else {
        digits[offset] |= mask;
    }
}

int TernaryVector::get(size_t index) const {
    size_t offset = index / 64;
    uint64_t mask = uint64_t(1) << (index % 64);

    int value = digits[offset] & mask ? 1 : 0;

    if (signs[offset] & mask)
        value = -value;

    return value;
}

void TernaryVector::canonize() {
    bool isZero = true;

    for (size_t i = 0; i < digits.size(); i++) {
        if (!digits[i])
            continue;

        isZero = false;

        if (digits[i] & -digits[i] & ~signs[i])
            return;
    }

    if (isZero)
        return;

    for (size_t i = 0; i < digits.size(); i++)
        signs[i] = (~signs[i]) & digits[i];
}

uint64_t TernaryVector::mix(uint64_t x) const {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

} // namespace leo
