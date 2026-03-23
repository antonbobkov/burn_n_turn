#ifndef GUI_TIMER_HEADER_ALREADY_DEFINED
#define GUI_TIMER_HEADER_ALREADY_DEFINED

/*
 * timer.h - Period-based timer for the Gui namespace.
 * Provides a simple frame or step counter: advance with Tick(), check with
 * Check(), and use UntilTick()/NextTick() for countdown or forcing the next
 * period.
 */

/* Period-based timer: internal counter and period.
 * Tick() advances the counter and returns true once per period.
 * Check() returns true when the counter is on a period boundary.
 * UntilTick() returns how many steps until the next period.
 * NextTick() sets the counter so the next Tick() returns true.
 * A timer with period 0 is considered inactive and never ticks. */
class Timer {
public:
  /* Build a timer with the given period (default 1). Counter starts at 0. */
  Timer(int nPeriod_ = 1);

  /* Steps remaining until the next period boundary. */
  int UntilTick();

  /* Advance by one step; return true when a period boundary is crossed. */
  bool Tick();

  /* Return true if the counter is currently on a period boundary. */
  bool Check();

  /* Set counter so the next Tick() returns true. */
  void NextTick();

  /* Reset the counter to 0. */
  void Reset();

  /* Return true if the period is non-zero (timer is active). */
  bool IsActive() const;

  /* Return the period length. */
  int GetPeriod() const;

  /* Return the current counter value. */
  int GetTimer() const;

private:
  int nTimer, nPeriod;
};

#endif // GUI_TIMER_HEADER_ALREADY_DEFINED
