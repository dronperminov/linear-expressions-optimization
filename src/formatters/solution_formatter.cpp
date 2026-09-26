#include <leo/formatters/solution_formatter.h>

namespace leo {

SolutionFormatter::SolutionFormatter(const std::string& inputVarName, const std::string& outputVarName, const std::string& newVarName, size_t start) {
    this->inputVarName = inputVarName;
    this->outputVarName = outputVarName;
    this->newVarName = newVarName;
    this->start = start;
}

std::string SolutionFormatter::index2variable(size_t index, size_t dimension) const {
    std::stringstream ss;

    if (index < dimension || inputVarName == newVarName) {
        ss << inputVarName << (index + start);
    }
    else {
        ss << newVarName << (index - dimension + start);
    }

    return ss.str();
}

} // namespace leo
