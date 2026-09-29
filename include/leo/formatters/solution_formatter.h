#pragma once

#include <iostream>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/term.h>

namespace leo {

class SolutionFormatter {
public:
    virtual void format(std::ostream& os, const Solution& solution) const = 0;

    virtual ~SolutionFormatter() = default;
};

} // namespace leo
