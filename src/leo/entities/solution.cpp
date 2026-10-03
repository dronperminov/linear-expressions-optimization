#include <leo/entities/solution.h>

namespace leo {

size_t Solution::getAdditions() const {
    size_t additions = substitutions.size();

    for (const std::vector<Term>& expression : expressions)
        if (!expression.empty())
            additions += expression.size() - 1;

    return additions;
}

}
