#include "simulation.h"
#include "utils/smart_pointer.h"

#include "game/controller/dragon_game_controller.h"
#include "game/dragon_game_runner.h"
#include "game_utils/MessageWriter.h"
#include "game_utils/game_runner_interface.h"
#include "utils/file_utils.h"
#include "wrappers/gui_key_type.h"

#include "wrappers_mock/GuiMock.h"
#include "wrappers_mock/SuiMock.h"

#include <iostream>
#include <memory>
#include <string>

namespace {

const unsigned kSimulationFrames = 900;
const unsigned kScreenW = 960;
const unsigned kScreenH = 600;

const std::string kGameDataFile("game_data.txt");

/* Frames at which to press Enter to advance menu (logo1 -> logo2 -> start ->
 * first level). */
const unsigned kKeyPressFrames[] = {8, 18, 28};
const unsigned kKeyPressCount = 3;

/* Tower center in screen coords (scale 2: 480,300 logical -> 960,600 screen).
 */
const Crd kTowerX = kScreenW / 2;
const Crd kTowerY = kScreenH * 3 / 4;

void LogState(DragonGameRunner *ctrl) {
  DragonGameController *twr = ctrl->GetTowerController();
  if (!twr)
    return;
  std::cout << "[sim] screen " << ctrl->GetActiveControllerName()
            << " score=" << twr->GetScore()
            << " high=" << twr->GetHighScore() << "\n";
}

} // namespace

void RunSimulation() {
  std::cout << "[sim] Starting simulation (seed 12345)\n";
  srand(12345);

  std::cout << "[sim] Creating mocks and file manager\n";
  auto p_mock_gr = std::make_unique<MockGraphicalInterface>();
  auto p_gr = std::make_unique<SimpleGraphicalInterface<std::string>>(
      p_mock_gr.get());

  auto p_mock_snd = std::make_unique<MockSoundInterface>();
  auto p_snd = std::make_unique<SimpleSoundInterface<std::string>>(
      p_mock_snd.get());

  std::unique_ptr<StdFileManager> underlying(new StdFileManager());
  std::unique_ptr<CachingReadOnlyFileManager> fm(
      new CachingReadOnlyFileManager(underlying.get(), ".txt"));

  bool b_exit = false;
  bool b_true = true;
  auto p_exit_ev =
      std::make_unique<SwitchEvent<bool, bool>>(b_exit, b_true);
  auto p_msg = std::make_unique<EmptyWriter>();
  Size sz(kScreenW, kScreenH);

  std::cout << "[sim] Creating ProgramEngine and tower controller explicitly\n";
  ProgramEngine pe(std::move(p_exit_ev), p_gr.get(), p_snd.get(),
                   std::move(p_msg), sz, fm.get());
  smart_pointer<DragonGameRunner> p_gl = make_smart(new DragonGameRunner(pe));

  std::string sound_content_before;
  bool sound_toggle_verified = false;

  std::cout
      << "[sim] Running " << kSimulationFrames
      << " frames (keys to level, keyboard take off, fly & shoot, menu)\n";
  for (unsigned i = 0; i < kSimulationFrames && !b_exit; ++i) {
    std::string screen_name = p_gl->GetActiveControllerName();

    for (unsigned k = 0; k < kKeyPressCount; ++k)
      if (i == kKeyPressFrames[k])
        p_gl->KeyDown(GUI_RETURN);

    if (screen_name == "level") {
      /* Take off with keyboard (space). */
      if (i == 35)
        p_gl->KeyDown(static_cast<GuiKeyType>(' '));
      /* Fly: hold mouse and move to steer dragon. */
      if (i >= 36 && i <= 80)
        p_gl->MouseMove(Point(kScreenW / 2 - 80 + (i - 36) * 2, kScreenH / 2));
      if (i == 36)
        p_gl->MouseDown(Point(kScreenW / 2 - 80, kScreenH / 2));
      if (i == 81)
        p_gl->MouseUp();
      if (i == 85)
        p_gl->Fire();
      if (i >= 90 && i <= 130)
        p_gl->MouseMove(
            Point(kScreenW / 2 + 100 - (i - 90), kScreenH / 2 - 30));
      if (i == 90)
        p_gl->MouseDown(Point(kScreenW / 2 + 100, kScreenH / 2 - 30));
      if (i == 131)
        p_gl->MouseUp();
      if (i == 135)
        p_gl->Fire();

      /* Open game menu and toggle sound (writes to game_data.txt via
       * SavableVariable). */
      if (i == 150)
        sound_content_before = GetFileContent(fm.get(), kGameDataFile);
      if (i == 151)
        p_gl->KeyDown(GUI_ESCAPE);
    }

    /* Menu navigation: open options, toggle sound (when we are in menu). */
    if (screen_name == "menu") {
      if (i == 152)
        p_gl->KeyDown(GUI_DOWN);
      if (i == 153)
        p_gl->KeyDown(GUI_DOWN);
      if (i == 154)
        p_gl->KeyDown(GUI_DOWN);
      if (i == 155)
        p_gl->KeyDown(GUI_RETURN);
      if (i == 156)
        p_gl->KeyDown(GUI_DOWN);
      if (i == 157)
        p_gl->KeyDown(GUI_RETURN);
      if (i >= 158 && i <= 165 && !sound_toggle_verified) {
        std::string sound_after = GetFileContent(fm.get(), kGameDataFile);
        if (sound_content_before != sound_after) {
          sound_toggle_verified = true;
          std::cout << "[sim] game_data.txt changed after sound toggle\n";
        }
      }
      if (i == 166)
        p_gl->KeyDown(GUI_ESCAPE);
    }

    p_gl->Update();

    if (i == 0 || (i + 1) % 200 == 0 || i == kSimulationFrames - 1)
      LogState(p_gl.get());
  }
  if (sound_toggle_verified)
    std::cout << "[sim] Verified: sound toggle wrote to game_data.txt.\n";
  std::cout << "[sim] Done\n";
}
