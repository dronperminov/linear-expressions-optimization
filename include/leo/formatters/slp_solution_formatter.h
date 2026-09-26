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

class SlpSolutionFormatter : public SolutionFormatter {
public:
    SlpSolutionFormatter(const std::string& inputVarName = "i", const std::string& outputVarName = "o", const std::string& newVarName = "t", size_t start = 0);

    void format(std::ostream& os, const Solution& solution) const;
private:
    void formatTermFirst(std::ostream& os, size_t index, int value, size_t dimension) const;
    void formatTerm(std::ostream& os, size_t index, int value, size_t dimension) const;
};

} // namespace leo
