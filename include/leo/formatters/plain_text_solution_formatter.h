#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/term.h>
#include <leo/formatters/solution_formatter.h>

namespace leo {

class PlainTextSolutionFormatter : public SolutionFormatter {
public:
    PlainTextSolutionFormatter(const std::string& inputVarName = "x", const std::string& outputVarName = "y", const std::string& newVarName = "x", size_t start = 1);

    void format(std::ostream& os, const Solution& solution) const;
private:
    void formatTermFirst(std::ostream& os, size_t index, int value, size_t dimension) const;
    void formatTerm(std::ostream& os, size_t index, int value, size_t dimension) const;
};

} // namespace leo
