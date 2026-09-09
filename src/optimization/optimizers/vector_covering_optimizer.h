#pragma once

#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "../../entities/substitution.h"
#include "../../entities/solution.h"
#include "../../entities/vector.h"
#include "../selection/candidate_selector.h"

struct Candidate {
    Substitution step;
    Vector vector;
    double score;
};

struct VectorCoveringParameters {
    int maxAbsValue;
};

struct VectorCoveringContext {
    std::unordered_set<Vector> uncovered;
    std::vector<Vector> vectors;
};

class VectorCoveringScorer {
    double coverWeight;
    double oneStepWeight;
    double hammingWeight;
    double matchesWeight;
public:
    VectorCoveringScorer();
    VectorCoveringScorer(double coverWeight, double oneStepWeight, double hammingWeight, double matchesWeight);

    void score(std::vector<Candidate>& candidates, const VectorCoveringContext& context) const;
private:
    void addOneStepScores(std::vector<Candidate>& candidates, const VectorCoveringContext& context) const;
};

class VectorCoveringOptimizer {
    int dimension;
    int count;
    std::vector<Vector> expressions;
    VectorCoveringParameters parameters;
    VectorCoveringScorer scorer;
    const CandidateSelector<Candidate>* selector;

    std::unordered_set<Vector> targets;
    std::unordered_set<Vector> uncovered;
    std::unordered_set<Vector> pool;
    std::vector<Vector> vectors;
    std::vector<Substitution> steps;
public:
    VectorCoveringOptimizer(const std::vector<std::vector<int>>& expressions, const VectorCoveringParameters& parameters, const VectorCoveringScorer& scorer, const CandidateSelector<Candidate> &selector);

    void setParameters(const VectorCoveringParameters& parameters);
    void setScorer(const VectorCoveringScorer& scorer);
    void setSelector(const CandidateSelector<Candidate> &selector);

    int optimize();
    Solution getSolution() const;
private:
    void initialize();

    std::vector<Candidate> getCandidates() const;
    void addCandidate(const Candidate& candidate);
};
