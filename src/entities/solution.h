#pragma once

#include <vector>

#include "substitution.h"
#include "term.h"

struct Solution {
    size_t dimension;
    std::vector<Substitution> substitutions;
    std::vector<std::vector<Term>> expressions;
};
