#include "core.h"
#include "game.h"
#include "game_runner_interface.h"

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

Gui::ProgramInfo GetProgramInfo() {
  Gui::ProgramInfo inf;

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

SP<Gui::GlobalController> GetGlobalController(Gui::ProgramEngine pe) {
  return new TowerGameGlobalController(pe);
}
