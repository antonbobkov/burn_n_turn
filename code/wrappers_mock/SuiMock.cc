#include "SuiMock.h"

void MockSoundInterface::DeleteSound(std::string snd) { (void)snd; }

std::string MockSoundInterface::LoadSound(std::string sFile) { return sFile; }

void MockSoundInterface::PlaySound(std::string snd, int nChannel, bool bLoop) {
  (void)snd;
  (void)nChannel;
  (void)bLoop;
}

void MockSoundInterface::StopSound(int channel) { (void)channel; }

void MockSoundInterface::SetVolume(float fVolume) { (void)fVolume; }
