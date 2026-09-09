#pragma once

#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "../../../entities/substitution.h"
#include "../../../entities/solution.h"
#include "../../../entities/vector.h"
#include "../../selection/score_selector.h"

struct Candidate {
    Substitution step;
    Vector vector;
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

    void score(const std::vector<Candidate>& candidates, const VectorCoveringContext& context, std::vector<double>& scores) const;
private:
    void addOneStepScores(const std::vector<Candidate>& candidates, const VectorCoveringContext& context, std::vector<double>& scores) const;
};

class VectorCoveringSolver {
    int dimension;
    int count;
    std::vector<Vector> expressions;
    VectorCoveringParameters parameters;
    VectorCoveringScorer scorer;
    const ScoreSelector* selector;

    std::unordered_set<Vector> targets;
    std::unordered_set<Vector> uncovered;
    std::unordered_set<Vector> pool;
    std::vector<Vector> vectors;
    std::vector<Substitution> steps;
    std::vector<double> scores;
public:
    VectorCoveringSolver(const std::vector<std::vector<int>>& expressions, const VectorCoveringParameters& parameters, const VectorCoveringScorer& scorer, const ScoreSelector &selector);

    void setParameters(const VectorCoveringParameters& parameters);
    void setScorer(const VectorCoveringScorer& scorer);
    void setSelector(const ScoreSelector &selector);

    int solve();
    Solution getSolution() const;
private:
    void initialize();

    std::vector<Candidate> getCandidates() const;
    void addCandidate(const Candidate& candidate);
};
