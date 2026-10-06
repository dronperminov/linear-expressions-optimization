#pragma once

#include <leo/entities/expressions_system.h>
#include <leo/optimization/strategy_pool.h>

namespace leo::presets {

StrategyPool vectorCoveringDefault(const ExpressionsSystem& expressionsSystem, bool addTargetPairs);
StrategyPool cseDefault(double potentialWeight);

} // leo::presets
