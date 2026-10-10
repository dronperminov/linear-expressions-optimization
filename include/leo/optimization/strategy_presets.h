#pragma once

#include <leo/optimization/solvers/cse/parameters.h>
#include <leo/optimization/solvers/vector_covering/parameters.h>
#include <leo/optimization/strategy_pool.h>

namespace leo::presets {

StrategyPool vectorCoveringDefault(const leo::vector_covering::Parameters& parameters);
StrategyPool vectorCoveringDistance(const leo::vector_covering::Parameters& parameters);
StrategyPool vectorCoveringAll(const leo::vector_covering::Parameters& parameters);

StrategyPool cseVanilla(const leo::cse::Parameters& parameters);
StrategyPool csePotential(const leo::cse::Parameters& parameters);
StrategyPool cseIntersections(const leo::cse::Parameters& parameters);
StrategyPool cseAll(const leo::cse::Parameters& parameters);

} // leo::presets
