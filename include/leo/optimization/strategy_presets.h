#pragma once

#include <leo/optimization/strategy_pool.h>
#include <leo/optimization/solvers/vector_covering/parameters.h>

namespace leo::presets {

StrategyPool vectorCoveringDefault(const leo::vector_covering::Parameters& parameters);
StrategyPool vectorCoveringDistance(const leo::vector_covering::Parameters& parameters);
StrategyPool vectorCoveringAll(const leo::vector_covering::Parameters& parameters);

StrategyPool cseVanilla();
StrategyPool csePotential();
StrategyPool cseIntersections();
StrategyPool cseAll();

} // leo::presets
