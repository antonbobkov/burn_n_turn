#include "dragon_constants.h"
#include "dragon_macros.h"
#include "game/dragon_game_runner.h"
#include "game_utils/game_runner_interface.h"
#include "utils/configuration_file.h"
#include "utils/file_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/geometry.h"

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

  StdFileManager fm;
  ConfigurationFile cfg(&fm, "config.txt");
  bool bFullScreen = (cfg.GetEntry("FULLSCREEN") == "true");

  if (bFullScreen) {
    inf.bFullScreen = true;
    inf.bFlexibleResolution = true;
    inf.bBlackBox = true;
  }

  return inf;
}

smart_pointer<GameRunner> GetGameRunner(ProgramEngine const& pe) {
  return make_smart(new DragonGameRunner(pe));
}
