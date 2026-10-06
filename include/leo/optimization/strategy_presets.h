#pragma once

#include <leo/entities/expressions_system.h>
#include <leo/optimization/strategy_pool.h>

namespace leo::presets {

StrategyPool vectorCoveringDefault(const ExpressionsSystem& expressionsSystem, bool addTargetPairs);
StrategyPool vectorCoveringDistance(const ExpressionsSystem& expressionsSystem, bool addTargetPairs);
StrategyPool vectorCoveringAll(const ExpressionsSystem& expressionsSystem, bool addTargetPairs);

StrategyPool cseVanilla();
StrategyPool csePotential();
StrategyPool cseIntersections();
StrategyPool cseAll();

} // leo::presets
