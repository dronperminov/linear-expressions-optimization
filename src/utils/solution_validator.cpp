#include <leo/utils/solution_validator.h>

namespace leo::utils {

bool SolutionValidator::validate(const std::vector<std::vector<int>>& expressions, const Solution& solution) const {
    if (expressions.size() != solution.expressions.size())
        return false;

    size_t dimension = solution.dimension;
    std::vector<std::vector<int>> steps(dimension, std::vector<int>(dimension, 0));

    for (size_t i = 0; i < dimension; i++)
        steps[i][i] = 1;

    for (const Substitution& s : solution.substitutions) {
        std::vector<int> step(dimension);

        for (size_t index = 0; index < dimension; index++)
            step[index] = s.ai * steps[s.i][index] + s.aj * steps[s.j][index];

        steps.emplace_back(step);
    }

    for (size_t i = 0; i < expressions.size(); i++) {
        std::vector<int> expression(dimension, 0);

        for (const Term& term : solution.expressions[i])
            for (size_t index = 0; index < dimension; index++)
                expression[index] += steps[term.index][index] * term.value;

        if (expressions[i] != expression)
            return false;
    }

    return true;
}

} // namespace leo::utils
