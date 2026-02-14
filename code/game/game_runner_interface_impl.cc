#include "core.h"
#include "dragon_constants.h"
#include "dragon_macros.h"
#include "game.h"
#include "game_utils/game_runner_interface.h"
#include "utils/smart_pointer.h"


#include <fstream>

namespace {

bool AreWeFullScreen() {
  std::ifstream ifs(sFullScreenPath);
  bool bRet;
  ifs >> bRet;
  if (ifs.fail())
    return false;
  return bRet;
}

} // namespace

ProgramInfo GetProgramInfo() {
  ProgramInfo inf;

#ifdef SMALL_SCREEN_VERSION
  inf.szScreenRez = Size(854, 480);
#else
  inf.szScreenRez = Size(960, 600);
#endif

  inf.strTitle = "Tower Defense";
  inf.nFramerate = 1000 / nFramesInSecond;
  inf.bMouseCapture = false;

  static bool bFullScreen = AreWeFullScreen();

  if (bFullScreen) {
    inf.bFullScreen = true;
    inf.bFlexibleResolution = true;
    inf.bBlackBox = true;
  }

  return inf;
}

smart_pointer<GameRunner> GetGameRunner(ProgramEngine pe) {
  return make_smart(new DragonGameRunner(pe));
}
