#ifndef TOWER_DEFENSE_SOUND_UTILS_H
#define TOWER_DEFENSE_SOUND_UTILS_H

/* sound_utils.h - Sound helpers: SoundInterfaceProxy (gated playback),
 * BackgroundMusicPlayer for level music, and channel/theme enums. */

#include "../utils/index.h"

template <class T> class SoundInterface;

/** Wraps SoundInterface<Index> and gates playback on bSoundOn
 * (Toggle/Get). */
class SoundInterfaceProxy {
  bool bSoundOn;
  SoundInterface<Index> *pSndRaw;

public:
  SoundInterfaceProxy(SoundInterface<Index> *pSndRaw_)
      : bSoundOn(true), pSndRaw(pSndRaw_) {}

  void PlaySound(Index i, int nChannel = -1, bool bLoop = false);

  void Toggle() { bSoundOn = !bSoundOn; }
  bool Get() { return bSoundOn; }
};

/** Plays level music from vThemes; SwitchTheme/StopMusic/ToggleOff control
 * playback. */
class BackgroundMusicPlayer {
public:
  int nCurrTheme;
  std::vector<Index> vThemes;
  SoundInterface<Index> *pSnd;

  bool bOff;

  BackgroundMusicPlayer() : nCurrTheme(-1), pSnd(nullptr), bOff(false) {}

  void SwitchTheme(int nTheme);
  void StopMusic();

  void ToggleOff();
};

enum { BG_MUSIC_CHANNEL = 0 };

enum {
  BG_BACKGROUND = 0,
  BG_SLOW_BACKGROUND = 1,
  BG_BACKGROUND2 = 2,
  BG_SLOW_BACKGROUND2 = 3,
  BG_BACKGROUND3 = 4,
  BG_SLOW_BACKGROUND3 = 5
};

#endif
