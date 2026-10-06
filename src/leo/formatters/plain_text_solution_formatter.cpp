#include <leo/formatters/plain_text_solution_formatter.h>

#include <cstddef>
#include <sstream>
#include <vector>

#include <leo/entities/substitution.h>
#include <leo/entities/term.h>

namespace leo {

PlainTextSolutionFormatter::PlainTextSolutionFormatter(const std::string& inputVarName, const std::string& outputVarName, const std::string& newVarName, size_t start) {
    this->inputVarName = inputVarName;
    this->outputVarName = outputVarName;
    this->newVarName = newVarName;
    this->start = start;
}

void PlainTextSolutionFormatter::format(std::ostream& os, const Solution& solution) const {
    os << "# " << solution.getAdditions() << " additions" << std::endl;

    if (!solution.substitutions.empty())
        os << "# substitutions" << std::endl;

    for (size_t i = 0; i < solution.substitutions.size(); i++) {
        Substitution s = solution.substitutions[i];

        os << index2variable(solution.dimension + i, solution.dimension) << " = ";
        formatTermFirst(os, s.i, s.ai, solution.dimension);
        formatTerm(os, s.j, s.aj, solution.dimension);
        os << std::endl;
    }

    os << std::endl;
    os << "# expressions" << std::endl;

    for (size_t i = 0; i < solution.expressions.size(); i++) {
        const std::vector<Term>& expression = solution.expressions[i];

        os << outputVarName << (i + start) << " = ";

        if (expression.empty()) {
            os << "0";
        }
        else {
            formatTermFirst(os, expression[0].index, expression[0].value, solution.dimension);
            for (size_t j = 1; j < expression.size(); j++)
                formatTerm(os, expression[j].index, expression[j].value, solution.dimension);
        }

        os << std::endl;
    }
}

std::string PlainTextSolutionFormatter::index2variable(size_t index, size_t dimension) const {
    std::ostringstream ss;

    if (index < dimension || inputVarName == newVarName) {
        ss << inputVarName << (index + start);
    }
    else {
        ss << newVarName << (index - dimension + start);
    }

    return ss.str();
}

void PlainTextSolutionFormatter::formatTermFirst(std::ostream& os, size_t index, int value, size_t dimension) const {
    if (value == -1) {
        os << "-";
    }
    else if (value != 1) {
        os << value;
    }

    os << index2variable(index, dimension);
}

void PlainTextSolutionFormatter::formatTerm(std::ostream& os, size_t index, int value, size_t dimension) const {
    os << " " << (value > 0 ? "+" : "-") << " ";

    if (value > 1 || value < -1)
        os << std::abs(value);

    os << index2variable(index, dimension);
}

} // namespace leo
