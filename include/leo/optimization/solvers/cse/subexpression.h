#pragma once

#include <vector>

namespace leo::cse {

struct Subexpression {
    size_t i;
    size_t j;
    int sign;
    std::vector<size_t> rows;
};

} // namespace leo::cse
