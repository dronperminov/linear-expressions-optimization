#include <leo/entities/solution.h>

namespace leo {

size_t Solution::getAdditions() const {
    size_t additions = substitutions.size();

    for (const std::vector<Term>& expression : expressions)
        if (!expression.empty())
            additions += expression.size() - 1;

    return additions;
}

size_t Solution::getInversions() const {
    size_t inversions = 0;

    for (const Substitution& s : substitutions)
        inversions += (s.ai < 0) && (s.aj < 0);

    for (const std::vector<Term>& expression : expressions) {
        if (expression.empty())
            continue;

        bool hasPositive = false;
        for (const Term& term : expression) {
            if (term.value > 0) {
                hasPositive = true;
                break;
            }
        }

        inversions += !hasPositive;
    }

    return inversions;
}

}
