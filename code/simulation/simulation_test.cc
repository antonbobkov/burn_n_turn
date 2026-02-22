/*
 * simulation_test - Catch2 test: run game simulation, reach level and menu,
 * toggle sound and verify file change in CachingReadOnlyFileManager.
 */

#include "game/controller/dragon_game_controller.h"
#include "game/dragon_game_runner.h"
#include "game_utils/MessageWriter.h"
#include "game_utils/game_runner_interface.h"
#include "utils/exception.h"
#include "utils/file_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/gui_key_type.h"
#include "wrappers_mock/GuiMock.h"
#include "wrappers_mock/SuiMock.h"

#include <algorithm>
#include <catch2/catch.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

const unsigned kSimulationFrames = 900;
const unsigned kScreenW = 960;
const unsigned kScreenH = 600;
const std::string kGameDataFile("game_data.txt");
const unsigned kKeyPressFrames[] = {8, 18, 28};
const unsigned kKeyPressCount = 3;

/* Longer run: cheats, advance past cutscene, load chapter, late level, game over.
 */
const unsigned kGameOverSimulationFrames = 5500;
const unsigned kCheatAdvancePastCutscene = 4;  /* levels 0->1->2 then cutscene */
const unsigned kCheatAdvanceToLateLevel = 6;

/* Build list of smart_pointer types with non-zero count, sorted by count desc. */
std::vector<std::pair<std::string, int>> GetNonZeroSmartPointerCounts() {
  std::vector<std::pair<std::string, int>> nonzero;
  for (const auto &entry : g_smart_pointer_count) {
    if (entry.second != 0)
      nonzero.push_back({entry.first, entry.second});
  }
  std::sort(nonzero.begin(), nonzero.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });
  return nonzero;
}

} // namespace

TEST_CASE("Simulation reaches level and menu, sound toggle writes to file",
          "[simulation][integration]") {
  try {
  srand(12345);

  int counter_before = nGlobalSuperMegaCounter;

  {
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

    ProgramEngine pe(std::move(p_exit_ev), p_gr.get(), p_snd.get(),
                     std::move(p_msg), sz, fm.get());
    smart_pointer<DragonGameRunner> p_gl = make_smart(new DragonGameRunner(pe));

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
          sound_content_before = GetFileContent(fm.get(), kGameDataFile);
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
          std::string sound_after = GetFileContent(fm.get(), kGameDataFile);
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

  std::vector<std::pair<std::string, int>> nonzero = GetNonZeroSmartPointerCounts();
  for (const auto &p : nonzero)
    std::cout << "smart_pointer_count[\"" << p.first << "\"] = " << p.second
              << "\n";
  std::cout << "nGlobalSuperMegaCounter = " << nGlobalSuperMegaCounter << "\n";

  CHECK(nGlobalSuperMegaCounter == 0);
  CHECK(nonzero.empty());

  } catch (const SimpleException &e) {
    std::ostringstream msg;
    msg << "Simulation threw SimpleException: " << e.GetDescription(true);
    std::cerr << msg.str() << std::endl;
    FAIL(msg.str());
  } catch (const MyException &e) {
    std::ostringstream msg;
    msg << "Simulation threw MyException: " << e.GetDescription(true);
    std::cerr << msg.str() << std::endl;
    FAIL(msg.str());
  } catch (const std::exception &e) {
    std::ostringstream msg;
    msg << "Simulation threw std::exception: " << e.what();
    std::cerr << msg.str() << std::endl;
    FAIL(msg.str());
  } catch (...) {
    std::cerr << "Simulation threw unknown exception (not std::exception)"
              << std::endl;
    FAIL("Simulation threw unknown exception type");
  }
}

TEST_CASE("Simulation cheats, load chapter, wait for game over",
          "[simulation][integration]") {
  try {
  srand(12345);

  {
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

    ProgramEngine pe(std::move(p_exit_ev), p_gr.get(), p_snd.get(),
                     std::move(p_msg), sz, fm.get());
    smart_pointer<DragonGameRunner> p_gl =
        make_smart(new DragonGameRunner(pe));
    DragonGameController *ctrl = p_gl->GetTowerController();
    if (ctrl)
      ctrl->SetCheatsOnSetting(true);

    bool reached_game_over = false;
    unsigned n_cheat_advance_count = 0;
    unsigned n_cheat_late_count = 0;
    bool did_load_chapter = false;
    bool in_wait_phase = false;
    bool opened_menu_for_chapter = false;
    unsigned menu_nav_step = 0;
    unsigned exit_phase = 0;       /* 0=none, 1=from game over, 2=score, ... */
    unsigned menu_exit_step = 0;

    for (unsigned i = 0; i < kGameOverSimulationFrames && !b_exit; ++i) {
      std::string screen_name = p_gl->GetActiveControllerName();
      bool on_game_over = p_gl->IsOnGameOverScreen();

      if (on_game_over)
        reached_game_over = true;
      if (reached_game_over && exit_phase == 0)
        exit_phase = 1;

      /* Press Enter to get past logo and start into first level. */
      if (i >= 8 && i <= 30 && i % 10 == 8)
        p_gl->KeyDown(GUI_RETURN);

      if (screen_name == "level") {
        if (!in_wait_phase) {
          /* Advance a few levels via cheat (past first cutscene). */
          if (n_cheat_advance_count < kCheatAdvancePastCutscene &&
              i >= 50 + n_cheat_advance_count * 15) {
            p_gl->KeyDown(static_cast<GuiKeyType>('\\'));
            n_cheat_advance_count++;
          }
          /* Open menu to load chapter once past first cutscene (level 3); do
           * it soon so the level timer does not advance the game. */
          else if (n_cheat_advance_count >= kCheatAdvancePastCutscene &&
                   !did_load_chapter && !opened_menu_for_chapter && i >= 110) {
            p_gl->KeyDown(GUI_ESCAPE);
            opened_menu_for_chapter = true;
          }
        }
        /* After load chapter: advance to one of the last levels via cheat. */
        if (did_load_chapter && n_cheat_late_count < kCheatAdvanceToLateLevel &&
            i >= 200 + n_cheat_late_count * 15) {
          p_gl->KeyDown(static_cast<GuiKeyType>('\\'));
          n_cheat_late_count++;
        }
        /* Start wait phase: do nothing until game over. */
        if (did_load_chapter &&
            n_cheat_late_count >= kCheatAdvanceToLateLevel && !in_wait_phase) {
          in_wait_phase = true;
        }
      }

      if (screen_name == "menu" && opened_menu_for_chapter && !did_load_chapter) {
        /* Main menu: continue=0, restart=1, load chapter=2. Step 0,1: DOWN x2;
         * step 2: Enter (open load chapter). Step 3: DOWN (chapter 2); 4: Enter.
         */
        if (menu_nav_step == 0) {
          p_gl->KeyDown(GUI_DOWN);
          p_gl->KeyDown(GUI_DOWN);
          menu_nav_step = 1;
        } else if (menu_nav_step == 1) {
          p_gl->KeyDown(GUI_RETURN);
          menu_nav_step = 2;
        } else if (menu_nav_step == 2) {
          p_gl->KeyDown(GUI_DOWN);
          menu_nav_step = 3;
        } else if (menu_nav_step == 3) {
          p_gl->KeyDown(GUI_RETURN);
          did_load_chapter = true;
        }
      }

      /* Exit the game after game over: game over -> score -> start -> level ->
       * menu -> select exit. */
      if (exit_phase == 1 && on_game_over) {
        p_gl->KeyDown(GUI_RETURN);
        exit_phase = 2;
      } else if (exit_phase == 2 && screen_name == "score") {
        p_gl->KeyDown(GUI_RETURN);
        exit_phase = 3;
      } else if (exit_phase == 3 && screen_name == "start") {
        p_gl->KeyDown(GUI_RETURN);
        exit_phase = 4;
      } else if (exit_phase == 4 && screen_name == "level") {
        p_gl->KeyDown(GUI_ESCAPE);
        exit_phase = 5;
      } else if (exit_phase == 5 && screen_name == "menu") {
        if (menu_exit_step < 4)
          p_gl->KeyDown(GUI_DOWN);
        else
          p_gl->KeyDown(GUI_RETURN);
        menu_exit_step++;
      }

      p_gl->Update();
    }

    CHECK(did_load_chapter);
    CHECK(reached_game_over);
    CHECK(b_exit);
  }

  std::vector<std::pair<std::string, int>> nonzero =
      GetNonZeroSmartPointerCounts();
  for (const auto &p : nonzero)
    std::cout << "smart_pointer_count[\"" << p.first << "\"] = " << p.second
              << "\n";
  std::cout << "nGlobalSuperMegaCounter = " << nGlobalSuperMegaCounter << "\n";

  CHECK(nGlobalSuperMegaCounter == 0);
  CHECK(nonzero.empty());

  } catch (const SimpleException &e) {
    std::ostringstream msg;
    msg << "Simulation threw SimpleException: " << e.GetDescription(true);
    std::cerr << msg.str() << std::endl;
    FAIL(msg.str());
  } catch (const MyException &e) {
    std::ostringstream msg;
    msg << "Simulation threw MyException: " << e.GetDescription(true);
    std::cerr << msg.str() << std::endl;
    FAIL(msg.str());
  } catch (const std::exception &e) {
    std::ostringstream msg;
    msg << "Simulation threw std::exception: " << e.what();
    std::cerr << msg.str() << std::endl;
    FAIL(msg.str());
  } catch (...) {
    std::cerr << "Simulation threw unknown exception (not std::exception)"
              << std::endl;
    FAIL("Simulation threw unknown exception type");
  }
}
