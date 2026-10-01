#pragma once

#include <iostream>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/term.h>
#include <leo/formatters/solution_formatter.h>

namespace leo {

class JsonSolutionFormatter : public SolutionFormatter {
public:
    JsonSolutionFormatter();

    void format(std::ostream& os, const Solution& solution) const;
};

} // namespace leo
