#pragma once

#include <cstddef>
#include <ostream>
#include <string>

#include <leo/entities/solution.h>
#include <leo/formatters/solution_formatter.h>

namespace leo {

class PlainTextSolutionFormatter : public SolutionFormatter {
    std::string inputVarName;
    std::string newVarName;
    std::string outputVarName;
    size_t start;
public:
    PlainTextSolutionFormatter(const std::string& inputVarName = "x", const std::string& outputVarName = "y", const std::string& newVarName = "x", size_t start = 1);

    void format(std::ostream& os, const Solution& solution) const override;
private:
    std::string index2variable(size_t index, size_t dimension) const;
    void formatTermFirst(std::ostream& os, size_t index, int value, size_t dimension) const;
    void formatTerm(std::ostream& os, size_t index, int value, size_t dimension) const;
};

} // namespace leo
