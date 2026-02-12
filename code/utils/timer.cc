/*
 * timer.cpp - Timer method definitions.
 */

#include "timer.h"

namespace Gui {

Timer::Timer(unsigned nPeriod_) : nTimer(0), nPeriod(nPeriod_) {}

unsigned Timer::UntilTick() { return nPeriod - nTimer % nPeriod; }

bool Timer::Tick() {
  if (nPeriod == 0)
    return false;
  return (++nTimer % nPeriod == 0);
}

bool Timer::Check() {
  if (nPeriod == 0)
    return false;
  return (nTimer % nPeriod == 0);
}

void Timer::NextTick() { nTimer = nPeriod - 1; }

} // namespace Gui
