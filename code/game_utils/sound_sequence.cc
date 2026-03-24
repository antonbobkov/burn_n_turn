#include "sound_sequence.h"

SoundSequence::SoundSequence() : nActive(0) {}

bool SoundSequence::Toggle() {
  if (nActive == (int)vSounds.size() - 1) {
    nActive = 0;
    return true;
  }
  nActive++;
  return false;
}

Index SoundSequence::GetSound() { return vSounds[nActive]; }

int SoundSequence::GetTime() const {
  if (vIntervals.empty())
    return 1;
  return vIntervals[nActive];
}

void SoundSequence::Add(Index iSnd, int nTime) {
  vSounds.push_back(iSnd);
  vIntervals.push_back(nTime);
}

bool SoundSequence::IsEmpty() const { return vSounds.empty(); }

bool SoundSequence::IsLast() const {
  return nActive == (int)vSounds.size() - 1;
}
