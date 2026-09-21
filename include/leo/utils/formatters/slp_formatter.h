#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/term.h>

namespace leo::utils::formatters {

class SlpFormatter {
    std::string inputVarName;
    std::string outputVarName;
    std::string newVarName;
public:
    SlpFormatter(const std::string& inputVarName = "i", const std::string& outputVarName = "o", const std::string& newVarName = "t");

    void format(std::ostream& os, const Solution& solution) const;
private:
    std::string index2variable(size_t index, size_t dimension) const;
    void formatTermFirst(std::ostream& os, size_t index, int value, size_t dimension) const;
    void formatTerm(std::ostream& os, size_t index, int value, size_t dimension) const;
};

} // namespace leo::utils::formatters
