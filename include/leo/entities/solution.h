#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <leo/entities/substitution.h>
#include <leo/entities/term.h>

namespace leo {

struct Solution {
    size_t dimension;
    std::vector<Substitution> substitutions;
    std::vector<std::vector<Term>> expressions;

    size_t getAdditions() const;
    size_t getInversions() const;
    uint64_t getSignature() const;
};

} // namespace leo
