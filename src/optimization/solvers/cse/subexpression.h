#pragma once

#include <vector>

struct Subexpression {
    size_t i;
    size_t j;
    int sign;
    std::vector<size_t> rows;
};
