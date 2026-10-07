#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <leo/entities/hash_set.h>
#include <leo/optimization/solvers/vector_covering/candidate.h>
#include <leo/optimization/solvers/vector_covering/context.h>
#include <leo/optimization/solvers/vector_covering/vector_covering_scorer.h>

namespace leo::vector_covering {

class DistanceScorer : public VectorCoveringScorer {
    struct Target {
        const Vector* vector;
        uint64_t hash;
    };

    double gainWeight;
    double tieWeight;
    double savingsWeight;
    double coverWeight;
    int supportSlack;
public:
    DistanceScorer(double gainWeight = 1000.0, double tieWeight = 1.0, double savingsWeight = 0.0, double coverWeight = 10.0, int supportSlack = 2);

    void score(const std::vector<Candidate>& candidates, const Context& context, std::vector<double>& scores) const override;
private:
    std::vector<uint64_t> getFingerptints(const std::vector<Vector>& vectors) const;
    void fillOnesAndTwos(const std::vector<uint64_t>& fingerprints, HashSet& ones, HashSet& twos) const;
    bool isNear(uint64_t hash, const std::vector<uint64_t>& fingerprints, const HashSet& set) const;

    size_t getGain(uint64_t hash, const std::vector<Target>& targets) const;
    size_t getGain(uint64_t hash, const std::vector<Target>& targets, const HashSet& set) const;
    double getSavingScore(const std::vector<Target>& targets, const Vector& vector) const;

    uint64_t mix(uint64_t x) const;
    uint64_t getFingerprint(const Vector& vector) const;
};

} // namespace leo::vector_covering
