#ifndef SOUND_SEQUENCE_HEADER
#define SOUND_SEQUENCE_HEADER

/*
 * SoundSequence: sequence of sound indices with per-sound intervals for
 * multi-sound effects. Used by Preloader and sound playback code.
 */

#include <vector>

#include "utils/index.h"

namespace Gui {

/** Sequence of sound indices with per-sound intervals; for multi-sound
 * effects. */
struct SoundSequence {
  std::vector<Index> vSounds;
  std::vector<unsigned> vIntervals;
  unsigned nActive;

  /** Advance to next sound; wraps to 0 at end. Returns true on wrap. */
  bool Toggle();
  Index GetSound();
  /** Interval for current sound; 1 if none defined. */
  unsigned GetTime() const;
  void Add(Index iSnd, unsigned nTime = 1);

  SoundSequence();
};

} // namespace Gui

#endif // SOUND_SEQUENCE_HEADER
