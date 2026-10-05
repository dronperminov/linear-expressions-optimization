#pragma once

#include <iostream>

#include <leo/entities/solution.h>

namespace leo {

class SolutionFormatter {
public:
    virtual void format(std::ostream& os, const Solution& solution) const = 0;

    virtual ~SolutionFormatter() = default;
};

} // namespace leo
