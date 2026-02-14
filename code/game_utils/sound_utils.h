#ifndef TOWER_DEFENSE_SOUND_UTILS_H
#define TOWER_DEFENSE_SOUND_UTILS_H

/* sound_utils.h - Sound helpers: SoundInterfaceProxy (gated playback),
 * BackgroundMusicPlayer for level music, and channel/theme enums.
 *
 * Dependencies (include required): SP_Info -> utils/smart_pointer.h;
 * Index -> utils/index.h; smart_pointer -> utils/smart_pointer.h. */

#include "utils/index.h"
#include "utils/smart_pointer.h"

template <class T> class SoundInterface;

/** Wraps SoundInterface<Index> and gates playback on bSoundOn
 * (Toggle/Get). */
class SoundInterfaceProxy : virtual public SP_Info {
  bool bSoundOn;
  smart_pointer<SoundInterface<Index>> pSndRaw;

public:
  std::string get_class_name() override { return "SoundInterfaceProxy"; }
  SoundInterfaceProxy(smart_pointer<SoundInterface<Index>> pSndRaw_)
      : pSndRaw(pSndRaw_), bSoundOn(true) {}

  void PlaySound(Index i, int nChannel = -1, bool bLoop = false);

  void Toggle() { bSoundOn = !bSoundOn; }
  bool Get() { return bSoundOn; }
};

/** Plays level music from vThemes; SwitchTheme/StopMusic/ToggleOff control
 * playback. */
struct BackgroundMusicPlayer {
  int nCurrTheme;
  std::vector<Index> vThemes;
  smart_pointer<SoundInterface<Index>> pSnd;

  bool bOff;

  BackgroundMusicPlayer() : nCurrTheme(-1), pSnd(), bOff(false) {}

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
