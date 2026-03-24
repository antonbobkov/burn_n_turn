#include "dragon_constants.h"
#include "dragon_game_runner.h"
#include "../game_utils/game_runner_interface.h"
#include "../utils/configuration_file.h"
#include "../utils/file_utils.h"
#include "../wrappers/geometry.h"

ProgramInfo GetProgramInfo() {
  StdFileManager fm;
  ConfigurationFile cfg(&fm, "config.txt");

  Size szScreenRez = (cfg.GetEntry("SCREEN_SIZE") == "small")
                     ? Size(854, 480) : Size(960, 600);
  bool bFullScreen = (cfg.GetEntry("FULLSCREEN") == "true");

  return ProgramInfo(szScreenRez, "Tower Defense", 1000 / nFramesInSecond,
                     false, bFullScreen, bFullScreen, bFullScreen);
}

std::unique_ptr<GameRunner> GetGameRunner(ProgramEngine const& pe) {
  return std::make_unique<DragonGameRunner>(pe);
}
