#pragma once

#pragma once

#include <iostream>
#include <vector>

#include "../../entities/solution.h"

class AbstractSolver {
public:
    virtual size_t solve() = 0;
    virtual Solution getSolution() const = 0;

    virtual ~AbstractSolver() = default;
};
