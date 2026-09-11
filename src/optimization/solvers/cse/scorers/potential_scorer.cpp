#include <leo/optimization/solvers/cse/scorers/potential_scorer.h>

namespace leo::cse {

PotentialScorer::PotentialScorer(double alpha) {
    setAlpha(alpha);
}

void PotentialScorer::setAlpha(double alpha) {
    this->alpha = alpha;
}

void PotentialScorer::score(const std::vector<Subexpression>& subexpressions, const Context& context, std::vector<double>& scores) const {
    scores.resize(subexpressions.size());

    for (size_t i = 0; i < subexpressions.size(); i++)
        scores[i] = getPotentialScore(subexpressions[i], context);
}

double PotentialScorer::getPotentialScore(const Subexpression& subexpression, const Context& context) const {
    size_t variables = context.matrix[0].size();
    size_t expressions = context.matrix.size();

    std::vector<int> column(expressions, 0);
    for (size_t i = 0; i < expressions; i++)
        if (context.matrix[i][subexpression.i] != 0 && context.matrix[i][subexpression.i] == subexpression.sign * context.matrix[i][subexpression.j])
            column[i] = context.matrix[i][subexpression.i];

    std::vector<int> column1(expressions);
    std::vector<int> column2(expressions);

    for (size_t i = 0; i < expressions; i++) {
        column1[i] = context.matrix[i][subexpression.i] - column[i];
        column2[i] = context.matrix[i][subexpression.j] - subexpression.sign * column[i];
    }

    int diff = 0;
    diff += getSavingVecVec(column1, column2, expressions);
    diff -= getSavingColCol(context.matrix, subexpression.i, subexpression.j, expressions);

    diff += getSavingVecVec(column, column1, expressions);
    diff += getSavingVecVec(column, column2, expressions);

    for (size_t i = 0; i < variables; i++) {
        if (i == subexpression.i || i == subexpression.j)
            continue;

        diff += getSavingVecCol(column1, context.matrix, i, expressions);
        diff -= getSavingColCol(context.matrix, subexpression.i, i, expressions);

        diff += getSavingVecCol(column2, context.matrix, i, expressions);
        diff -= getSavingColCol(context.matrix, subexpression.j, i, expressions);

        diff += getSavingVecCol(column, context.matrix, i, expressions);
    }

    return subexpression.rows.size() - 1 + alpha * diff;
}

int PotentialScorer::getSavingVecVec(const std::vector<int>& column1, const std::vector<int>& column2, size_t expressions) const {
    int positive = 0;
    int negative = 0;

    for (size_t i = 0; i < expressions; i++) {
        if (column1[i] == 0 || column2[i] == 0)
            continue;

        if (column1[i] == column2[i]) {
            positive++;
        }
        else if (column1[i] == -column2[i]) {
            negative++;
        }
    }

    return getSavingPart(positive) + getSavingPart(negative);
}

int PotentialScorer::getSavingVecCol(const std::vector<int>& column, const std::vector<std::vector<int>>& matrix, size_t variable, size_t expressions) const {
    int positive = 0;
    int negative = 0;

    for (size_t i = 0; i < expressions; i++) {
        int value = matrix[i][variable];

        if (column[i] == 0 || value == 0)
            continue;

        if (column[i] == value) {
            positive++;
        }
        else if (column[i] == -value) {
            negative++;
        }
    }

    return getSavingPart(positive) + getSavingPart(negative);
}

int PotentialScorer::getSavingColCol(const std::vector<std::vector<int>>& matrix, size_t variable1, size_t variable2, size_t expressions) const {
    int positive = 0;
    int negative = 0;

    for (size_t i = 0; i < expressions; i++) {
        int value1 = matrix[i][variable1];
        int value2 = matrix[i][variable2];

        if (value1 == 0 || value2 == 0)
            continue;

        if (value1 == value2) {
            positive++;
        }
        else if (value1 == -value2) {
            negative++;
        }
    }

    return getSavingPart(positive) + getSavingPart(negative);   
}

int PotentialScorer::getSavingPart(int count) const {
    return std::max(count - 1, 0);
}

} // namespace leo::cse
