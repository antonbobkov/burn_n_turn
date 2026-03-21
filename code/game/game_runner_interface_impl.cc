#include "dragon_constants.h"
#include "dragon_game_runner.h"
#include "../game_utils/game_runner_interface.h"
#include "../utils/configuration_file.h"
#include "../utils/file_utils.h"
#include "../utils/smart_pointer.h"
#include "../wrappers/geometry.h"

ProgramInfo GetProgramInfo() {
  ProgramInfo inf;

  inf.strTitle = "Tower Defense";
  inf.nFramerate = 1000 / nFramesInSecond;
  inf.bMouseCapture = false;

  StdFileManager fm;
  ConfigurationFile cfg(&fm, "config.txt");
  inf.szScreenRez = (cfg.GetEntry("SCREEN_SIZE") == "small")
                    ? Size(854, 480) : Size(960, 600);

  if (cfg.GetEntry("FULLSCREEN") == "true") {
    inf.bFullScreen = true;
    inf.bFlexibleResolution = true;
    inf.bBlackBox = true;
  }

  return inf;
}

std::unique_ptr<GameRunner> GetGameRunner(ProgramEngine const& pe) {
  return std::make_unique<DragonGameRunner>(pe);
}
