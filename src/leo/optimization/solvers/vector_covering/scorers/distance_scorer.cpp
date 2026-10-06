#include <leo/optimization/solvers/vector_covering/scorers/distance_scorer.h>

#include <algorithm>
#include <cstdint>
#include <limits>

namespace leo::vector_covering {

DistanceScorer::DistanceScorer(double gainWeight, double tieWeight, double savingsWeight, int supportSlack) : gainWeight(gainWeight), tieWeight(tieWeight), savingsWeight(savingsWeight), supportSlack(supportSlack) {

}

void DistanceScorer::score(const std::vector<Candidate>& candidates, const Context& context, std::vector<double>& scores) const {
    std::vector<uint64_t> fingerprints = getFingerptints(context.vectors);

    HashSet ones(2 * fingerprints.size());
    HashSet twos(2 * fingerprints.size() * (fingerprints.size() - 1));
    fillOnesAndTwos(fingerprints, ones, twos);

    size_t maxSupport = 0;
    std::vector<Target> near2, near3, far4;

    for (const Vector& target : context.uncovered) {
        maxSupport = std::max(maxSupport, target.getSupport());

        uint64_t hash = getFingerprint(target);

        if (twos.contains(hash)) {
            near2.push_back({&target, hash});
            continue;
        }

        if (isNear(hash, fingerprints, twos)) {
            near3.push_back({&target, hash});
        }
        else {
            far4.push_back({&target, hash});
        }
    }

    scores.assign(candidates.size(), 0.0);

    for (size_t index = 0; index < candidates.size(); index++) {
        const Vector& vector = candidates[index].canonized;

        if (supportSlack >= 0 && vector.getSupport() > maxSupport + supportSlack) {
            scores[index] = -std::numeric_limits<double>::max();
            continue;
        }

        const uint64_t hash = getFingerprint(vector);

        size_t gain2 = getGain(hash, near2);
        size_t gain3 = getGain(hash, near3, ones);
        size_t gain4 = getGain(hash, far4, twos);

        double gain = gain2 + gain3 + gain4;
        double tie = 3.0 * gain2 + 5.0 * gain3 + 7.0 * gain4;
        double score = gainWeight * gain - tieWeight * tie;

        if (savingsWeight > 0) {
            score += getSavingScore(near3, vector);
            score += getSavingScore(far4, vector);
        }

        scores[index] = score;
    }
}

std::vector<uint64_t> DistanceScorer::getFingerptints(const std::vector<Vector>& vectors) const {
    std::vector<uint64_t> fingerprints(vectors.size());

    for (size_t i = 0; i < vectors.size(); i++)
        fingerprints[i] = getFingerprint(vectors[i]);

    return fingerprints;
}

void DistanceScorer::fillOnesAndTwos(const std::vector<uint64_t>& fingerprints, HashSet& ones, HashSet& twos) const {
    for (size_t i = 0; i < fingerprints.size(); i++) {
        ones.insert(fingerprints[i]);
        ones.insert(0 - fingerprints[i]);

        for (size_t j = i + 1; j < fingerprints.size(); j++) {
            twos.insert(fingerprints[i] + fingerprints[j]);
            twos.insert(fingerprints[i] - fingerprints[j]);
            twos.insert(fingerprints[j] - fingerprints[i]);
            twos.insert(0 - fingerprints[i] - fingerprints[j]);
        }
    }
}

bool DistanceScorer::isNear(uint64_t hash, const std::vector<uint64_t>& fingerprints, const HashSet& set) const {
    for (uint64_t fingerprint : fingerprints)
        if (set.contains(hash - fingerprint) || set.contains(hash + fingerprint))
            return true;

    return false;
}

size_t DistanceScorer::getGain(uint64_t hash, const std::vector<Target>& targets) const {
    size_t gain = 0;

    for (const Target& target : targets)
        gain += target.hash == hash || target.hash == 0 - hash;

    return gain;
}

size_t DistanceScorer::getGain(uint64_t hash, const std::vector<Target>& targets, const HashSet& set) const {
    size_t gain = 0;

    for (const Target& target : targets)
        gain += set.contains(target.hash - hash) || set.contains(target.hash + hash);

    return gain;
}

double DistanceScorer::getSavingScore(const std::vector<Target>& targets, const Vector& vector) const {
    double score = 0;
    size_t save = vector.getSupport() - 1;

    for (const Target& target : targets)
        if (vector.isSubVector(*target.vector))
            score += savingsWeight * save;

    return score;
}

uint64_t DistanceScorer::mix(uint64_t x) const {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

uint64_t DistanceScorer::getFingerprint(const Vector& vector) const {
    uint64_t hash = 0;

    for (size_t i = 0; i < vector.getDimension(); i++)
        hash += static_cast<uint64_t>(static_cast<int64_t>(vector[i])) * (mix(i + 1) | 1);

    return hash;
}

} // namespace leo::vector_covering
