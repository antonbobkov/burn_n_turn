/*
 * timer.cpp - Timer method definitions.
 */

#include "timer.h"

Timer::Timer(int nPeriod_) : nTimer(0), nPeriod(nPeriod_) {}

int Timer::UntilTick() { return nPeriod - nTimer % nPeriod; }

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

void Timer::Reset() { nTimer = 0; }

bool Timer::IsActive() const { return nPeriod != 0; }

int Timer::GetPeriod() const { return nPeriod; }

int Timer::GetTimer() const { return nTimer; }
