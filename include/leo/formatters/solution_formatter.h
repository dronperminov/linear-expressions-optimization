#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/term.h>

namespace leo {

class SolutionFormatter {
protected:
    std::string inputVarName;
    std::string outputVarName;
    std::string newVarName;
    size_t start;
public:
    SolutionFormatter(const std::string& inputVarName, const std::string& outputVarName, const std::string& newVarName, size_t start);

    virtual void format(std::ostream& os, const Solution& solution) const = 0;
protected:
    std::string index2variable(size_t index, size_t dimension) const;
};

} // namespace leo
