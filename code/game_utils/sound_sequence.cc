#include "game_utils/sound_sequence.h"

namespace Gui {

SoundSequence::SoundSequence() : nActive(0) {}

bool SoundSequence::Toggle() {
  if (nActive == vSounds.size() - 1) {
    nActive = 0;
    return true;
  }
  nActive++;
  return false;
}

Index SoundSequence::GetSound() { return vSounds[nActive]; }

unsigned SoundSequence::GetTime() const {
  if (vIntervals.empty())
    return 1;
  return vIntervals[nActive];
}

void SoundSequence::Add(Index iSnd, unsigned nTime) {
  vSounds.push_back(iSnd);
  vIntervals.push_back(nTime);
}

} // namespace Gui
