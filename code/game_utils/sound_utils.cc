#include "game_utils/sound_utils.h"
#include "utils/exception.h"
#include "utils/string_utils.h"
#include "wrappers/SuiGen.h"

void SoundInterfaceProxy::PlaySound(Index i, int nChannel, bool bLoop) {
  if (bSoundOn)
    pSndRaw->PlaySound(i, nChannel, bLoop);
}

void BackgroundMusicPlayer::ToggleOff() {
  if (!bOff) {
    bOff = true;
    StopMusic();
  } else {
    bOff = false;
  }
}

void BackgroundMusicPlayer::SwitchTheme(int nTheme) {
  if (bOff)
    return;

  if (nCurrTheme == nTheme)
    return;

  if (nTheme < 0 || nTheme >= int(vThemes.size()))
    throw SimpleException("BackgroundMusicPlayer", "SwitchTheme",
                          "Bad theme index " + S(nTheme));

  StopMusic();
  pSnd->PlaySound(vThemes[nTheme], BG_MUSIC_CHANNEL, true);

  nCurrTheme = nTheme;
}

void BackgroundMusicPlayer::StopMusic() {
  if (nCurrTheme != -1)
    pSnd->StopSound(BG_MUSIC_CHANNEL);
  nCurrTheme = -1;
}
