#pragma once

#include <vector>

#include <leo/entities/substitution.h>
#include <leo/entities/term.h>

namespace leo {

struct Solution {
    size_t dimension;
    std::vector<Substitution> substitutions;
    std::vector<std::vector<Term>> expressions;
};

} // namespace leo
