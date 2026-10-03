#include <leo/formatters/json_solution_formatter.h>

namespace leo {

JsonSolutionFormatter::JsonSolutionFormatter() {

}

void JsonSolutionFormatter::format(std::ostream& os, const Solution& solution) const {
    os << "{" << std::endl;
    os << "    \"additions\": " << solution.getAdditions() << "," << std::endl;
    os << "    \"substitutions\": [" << std::endl;

    for (size_t i = 0; i < solution.substitutions.size(); i++) {
        Substitution s = solution.substitutions[i];
        if (i > 0)
            os << "," << std::endl;

        os << "        {\"i\": " << s.i << ", \"ai\": " << s.ai << ", \"j\": " << s.j << ", \"aj\": " << s.aj << "}";
    }

    os << std::endl;
    os << "    ]," << std::endl;
    os << "    \"expressions\": [" << std::endl;

    for (size_t i = 0; i < solution.expressions.size(); i++) {
        const std::vector<Term>& expression = solution.expressions[i];

        if (i > 0)
            os << "," << std::endl;

        os << "        [";
        for (size_t j = 0; j < expression.size(); j++)
            os << (j > 0 ? ", " : "") << "{\"index\": " << expression[j].index << ", \"value\": " << expression[j].value << "}";
        os << "]";
    }

    os << std::endl;
    os << "    ]" << std::endl;
    os << "}" << std::endl;
}

} // namespace leo
