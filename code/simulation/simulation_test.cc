/*
 * simulation_test - Catch2 test: run game simulation, reach level and menu,
 * toggle sound and verify file change in CachingReadOnlyFileManager.
 */

#include "GuiMock.h"
#include "MessageWriter.h"
#include "SuiMock.h"
#include "file_utils.h"
#include "game.h"
#include "game_runner_interface.h"
#include "gui_key_type.h"
#include "smart_pointer.h"

#include <algorithm>
#include <catch2/catch.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>


using namespace Gui;

namespace {

const unsigned kSimulationFrames = 900;
const unsigned kScreenW = 960;
const unsigned kScreenH = 600;
const std::string kSoundFileName("soundon.txt");
const unsigned kKeyPressFrames[] = {8, 18, 28};
const unsigned kKeyPressCount = 3;

} // namespace

TEST_CASE("Simulation reaches level and menu, sound toggle writes to file",
          "[simulation][integration]") {
  srand(12345);

  int counter_before = nGlobalSuperMegaCounter;

  {
    smart_pointer<MockGraphicalInterface> p_mock_gr =
        make_smart(new MockGraphicalInterface());
    smart_pointer<GraphicalInterface<Index>> p_gr =
        make_smart(new SimpleGraphicalInterface<std::string>(p_mock_gr));
    smart_pointer<MockSoundInterface> p_mock_snd =
        make_smart(new MockSoundInterface());
    smart_pointer<SoundInterface<Index>> p_snd =
        make_smart(new SimpleSoundInterface<std::string>(p_mock_snd));

    std::unique_ptr<Gui::StdFileManager> underlying(new Gui::StdFileManager());
    std::unique_ptr<Gui::CachingReadOnlyFileManager> fm(
        new Gui::CachingReadOnlyFileManager(underlying.get(), ".txt"));

    bool b_exit = false;
    bool b_true = true;
    smart_pointer<Event> p_exit_ev = make_smart(NewSwitchEvent(b_exit, b_true));
    smart_pointer<MessageWriter> p_msg = make_smart(new EmptyWriter());
    Size sz(kScreenW, kScreenH);

    ProgramEngine pe(p_exit_ev, p_gr, p_snd, p_msg, sz, fm.get());
    smart_pointer<DragonGameRunner> p_gl =
        make_smart(new DragonGameRunner(pe));

    bool reached_level = false;
    bool reached_menu = false;
    std::string sound_content_before;
    bool sound_file_flipped = false;

    for (unsigned i = 0; i < kSimulationFrames && !b_exit; ++i) {
      /* Name of current screen (e.g. "level", "menu", "logo"). */
      std::string screen_name = p_gl->GetActiveControllerName();
      if (screen_name == "level")
        reached_level = true;
      if (screen_name == "menu")
        reached_menu = true;

      /* Press Enter to advance through logo and start into first level. */
      for (unsigned k = 0; k < kKeyPressCount; ++k)
        if (i == kKeyPressFrames[k])
          p_gl->KeyDown(GUI_RETURN);

      if (screen_name == "level") {
        /* Press Space to take off dragon from the tower. */
        if (i == 35)
          p_gl->KeyDown(static_cast<GuiKeyType>(' '));
        /* Move cursor so dragon flies toward that point (first flight leg). */
        if (i >= 36 && i <= 80)
          p_gl->MouseMove(
              Point(kScreenW / 2 - 80 + (i - 36) * 2, kScreenH / 2));
        /* Hold mouse button to start steering the dragon. */
        if (i == 36)
          p_gl->MouseDown(Point(kScreenW / 2 - 80, kScreenH / 2));
        /* Release mouse after first flight. */
        if (i == 81)
          p_gl->MouseUp();
        /* Shoot a fireball in the current flight direction. */
        if (i == 85)
          p_gl->Fire();
        /* Move cursor for second flight leg (other direction). */
        if (i >= 90 && i <= 130)
          p_gl->MouseMove(
              Point(kScreenW / 2 + 100 - (i - 90), kScreenH / 2 - 30));
        /* Hold mouse to steer again. */
        if (i == 90)
          p_gl->MouseDown(Point(kScreenW / 2 + 100, kScreenH / 2 - 30));
        /* Release mouse after second flight. */
        if (i == 131)
          p_gl->MouseUp();
        /* Shoot another fireball. */
        if (i == 135)
          p_gl->Fire();
        if (i == 150)
          sound_content_before = GetFileContent(fm.get(), kSoundFileName);
        /* Open the in-game pause menu. */
        if (i == 151)
          p_gl->KeyDown(GUI_ESCAPE);
      }

      if (screen_name == "menu") {
        /* Move menu selection down to "options". */
        if (i == 152)
          p_gl->KeyDown(GUI_DOWN);
        if (i == 153)
          p_gl->KeyDown(GUI_DOWN);
        if (i == 154)
          p_gl->KeyDown(GUI_DOWN);
        /* Confirm to open the options submenu. */
        if (i == 155)
          p_gl->KeyDown(GUI_RETURN);
        /* Move selection down to the sound toggle. */
        if (i == 156)
          p_gl->KeyDown(GUI_DOWN);
        /* Confirm to toggle sound on/off (writes to file). */
        if (i == 157)
          p_gl->KeyDown(GUI_RETURN);
        if (i >= 158 && i <= 165) {
          std::string sound_after = GetFileContent(fm.get(), kSoundFileName);
          if (sound_content_before != sound_after)
            sound_file_flipped = true;
        }
        /* Go back from options to main menu. */
        if (i == 166)
          p_gl->KeyDown(GUI_ESCAPE);
        /* Select "exit" on main menu (one DOWN from "options" at 3). */
        if (i == 167)
          p_gl->KeyDown(GUI_DOWN);
        if (i == 168)
          p_gl->KeyDown(GUI_RETURN);
      }

      /* Advance the game by one frame. */
      p_gl->Update();
    }

    CHECK(reached_level);
    CHECK(reached_menu);
    CHECK(sound_file_flipped);
    CHECK(b_exit);
  }

  std::vector<std::pair<std::string, int>> nonzero;
  for (const auto &entry : g_smart_pointer_count) {
    if (entry.second != 0)
      nonzero.push_back({entry.first, entry.second});
  }
  std::sort(nonzero.begin(), nonzero.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });
  for (const auto &p : nonzero)
    std::cout << "smart_pointer_count[\"" << p.first << "\"] = " << p.second
              << "\n";
  std::cout << "nGlobalSuperMegaCounter = " << nGlobalSuperMegaCounter << "\n";

  CHECK(nGlobalSuperMegaCounter == 0);
  CHECK(nonzero.empty());
}
