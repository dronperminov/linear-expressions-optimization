#pragma once

#include "../common_subexpression_scorer.h"

class CommonSubexpressionPotentialScorer : public CommonSubexpressionScorer {
    double alpha;
public:
    CommonSubexpressionPotentialScorer(double alpha);

    void setAlpha(double alpha);

    void score(const std::vector<Subexpression>& subexpressions, const CommonSubexpressionContext& context, std::vector<double>& scores) const override;
private:
    double getPotentialScore(const Subexpression& subexpression, const CommonSubexpressionContext& context) const;
    int getSavingVecVec(const std::vector<int>& column1, const std::vector<int>& column2, size_t expressions) const;
    int getSavingVecCol(const std::vector<int>& column, const std::vector<std::vector<int>>& matrix, size_t j, size_t expressions) const;
    int getSavingColCol(const std::vector<std::vector<int>>& matrix, size_t i, size_t j, size_t expressions) const;
    int getSavingPart(int count) const;
};
