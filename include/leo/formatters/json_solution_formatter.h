#pragma once

#include <ostream>

#include <leo/entities/solution.h>
#include <leo/formatters/solution_formatter.h>

namespace leo {

class JsonSolutionFormatter : public SolutionFormatter {
public:
    JsonSolutionFormatter();

    void format(std::ostream& os, const Solution& solution) const override;
};

} // namespace leo
