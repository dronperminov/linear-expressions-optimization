#include <leo/entities/solution.h>

#include <algorithm>

#include <leo/entities/vector.h>

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

uint64_t Solution::getSignature() const {
    std::vector<Vector> vectors;
    for (size_t i = 0; i < dimension; i++)
        vectors.emplace_back(dimension, i);

    std::vector<uint64_t> fingerprints;
    for (const Substitution& s : substitutions) {
        vectors.push_back(vectors[s.i] * s.ai + vectors[s.j] * s.aj);
        fingerprints.push_back(vectors.back().getCanonized().getFingerprint());
    }

    std::sort(fingerprints.begin(), fingerprints.end());

    uint64_t hash = 1469598103934665603ULL;
    for (uint64_t f : fingerprints)
        hash = (hash ^ f) * 1099511628211ULL;

    return hash;
}

} // namepsace leo
