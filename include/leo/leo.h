#pragma once

// entities
#include <leo/entities/expressions_system.h>
#include <leo/entities/solution.h>
#include <leo/entities/substitution.h>
#include <leo/entities/term.h>
#include <leo/entities/vector.h>

// utils
#include <leo/utils/solution_validator.h>

// selectors
#include <leo/optimization/selection/greedy_alternative_selector.h>
#include <leo/optimization/selection/greedy_random_selector.h>
#include <leo/optimization/selection/greedy_selector.h>
#include <leo/optimization/selection/score_selector.h>

// solvers
#include <leo/optimization/solvers/solver.h>

// cse solver
#include <leo/optimization/solvers/cse/common_subexpression_scorer.h>
#include <leo/optimization/solvers/cse/common_subexpression_solver.h>
#include <leo/optimization/solvers/cse/context.h>
#include <leo/optimization/solvers/cse/scorers/default_scorer.h>
#include <leo/optimization/solvers/cse/scorers/potential_scorer.h>
#include <leo/optimization/solvers/cse/subexpression.h>

// vector covering solver
#include <leo/optimization/solvers/vector_covering/candidate.h>
#include <leo/optimization/solvers/vector_covering/context.h>
#include <leo/optimization/solvers/vector_covering/scorers/default_scorer.h>
#include <leo/optimization/solvers/vector_covering/vector_covering_parameters.h>
#include <leo/optimization/solvers/vector_covering/vector_covering_scorer.h>
#include <leo/optimization/solvers/vector_covering/vector_covering_solver.h>
