#include <leo/utils/formatters/slp_formatter.h>

namespace leo::utils::formatters {

SlpFormatter::SlpFormatter(const std::string& inputVarName, const std::string& outputVarName, const std::string& newVarName) {
    this->inputVarName = inputVarName;
    this->outputVarName = outputVarName;
    this->newVarName = newVarName;
}

void SlpFormatter::format(std::ostream& os, const Solution& solution) const {
    for (size_t i = 0; i < solution.substitutions.size(); i++) {
        Substitution s = solution.substitutions[i];

        os << newVarName << i << ":=";
        formatTermFirst(os, s.i, s.ai, solution.dimension);
        formatTerm(os, s.j, s.aj, solution.dimension);
        os << ";" << std::endl;
    }

    for (size_t i = 0; i < solution.expressions.size(); i++) {
        const std::vector<Term>& expression = solution.expressions[i];

        os << outputVarName << i << ":=";
        formatTermFirst(os, expression[0].index, expression[0].value, solution.dimension);
        for (size_t j = 1; j < expression.size(); j++)
            formatTerm(os, expression[j].index, expression[j].value, solution.dimension);
        os << ";" << std::endl;
    }
}

std::string SlpFormatter::index2variable(size_t index, size_t dimension) const {
    std::stringstream ss;

    if (index < dimension) {
        ss << inputVarName << index;
    }
    else {
        ss << newVarName << (index - dimension);
    }

    return ss.str();
}

void SlpFormatter::formatTermFirst(std::ostream& os, size_t index, int value, size_t dimension) const {
    if (value == -1) {
        os << "-";
    }
    else if (value != 1) {
        os << value;
    }

    os << index2variable(index, dimension);
}

void SlpFormatter::formatTerm(std::ostream& os, size_t index, int value, size_t dimension) const {
    os << (value > 0 ? "+" : "-");

    if (value > 1 || value < -1)
        os << std::abs(value);

    os << index2variable(index, dimension);
}

} // namespace leo::utils::formatters
