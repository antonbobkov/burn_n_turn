#ifndef GUI_TIMER_HEADER_ALREADY_DEFINED
#define GUI_TIMER_HEADER_ALREADY_DEFINED

/*
 * timer.h - Period-based timer for the Gui namespace.
 * Provides a simple frame or step counter: advance with Tick(), check with
 * Check(), and use UntilTick()/NextTick() for countdown or forcing the next
 * period.
 */

namespace Gui {

/* Period-based timer: internal counter (nTimer) and period (nPeriod).
 * Tick() advances the counter and returns true once per period.
 * Check() returns true when the counter is on a period boundary.
 * UntilTick() returns how many steps until the next period.
 * NextTick() sets the counter so the next Tick() returns true. */
struct Timer {
  unsigned nTimer, nPeriod;

  /* Build a timer with the given period (default 1). Counter starts at 0. */
  Timer(unsigned nPeriod_ = 1);

  /* Steps remaining until the next period boundary. */
  unsigned UntilTick();

  /* Advance by one step; return true when a period boundary is crossed. */
  bool Tick();

  /* Return true if the counter is currently on a period boundary. */
  bool Check();

  /* Set counter so the next Tick() returns true. */
  void NextTick();
};

} // namespace Gui

#endif // GUI_TIMER_HEADER_ALREADY_DEFINED
