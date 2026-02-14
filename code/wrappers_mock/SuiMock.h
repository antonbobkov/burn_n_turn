#ifndef SUIMOCK_ALREADY_INCLUDED
#define SUIMOCK_ALREADY_INCLUDED

#include <string>

#include "wrappers/SuiGen.h"

namespace Gui {

/* Sound backend that does no real sound. LoadSound returns the path string;
 * other ops are no-op. */
class MockSoundInterface : public SoundInterface<std::string> {
public:
  std::string get_class_name() override { return "MockSoundInterface"; }
  /*virtual*/ void DeleteSound(std::string snd);
  /*virtual*/ std::string LoadSound(std::string sFile);
  /*virtual*/ void PlaySound(std::string snd, int nChannel, bool bLoop);
  /*virtual*/ void StopSound(int channel);
  /*virtual*/ void SetVolume(float fVolume);
};

} // namespace Gui

#endif
