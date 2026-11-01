/*
 * simulation_test - Catch2 test: run game simulation, reach level and menu,
 * toggle sound and verify file change in CachingReadOnlyFileManager.
 */

#include "../game/castle.h"
#include "../game/controller/dragon_game_controller.h"
#include "../game/controller/level_controller.h"
#include "../game/critters/royal.h"
#include "../game/dragon_game_runner.h"
#include "../game/tutorial.h"
#include "../game_utils/MessageWriter.h"
#include "../game_utils/game_runner_interface.h"
#include "../utils/exception.h"
#include "../utils/file_utils.h"
#include "../wrappers/gui_key_type.h"
#include "../wrappers_mock/GuiMock.h"
#include "../wrappers_mock/SuiMock.h"

#include <algorithm>
#include <catch2/catch.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

const int kSimulationFrames = 900;
const int kScreenW = 960;
const int kScreenH = 600;
const std::string kGameDataFile("game_data.txt");
const int kKeyPressFrames[] = {8, 18, 28};
const int kKeyPressCount = 3;

/* Longer run: cheats, advance past cutscene, load chapter, late level, game
 * over.
 */
const int kGameOverSimulationFrames = 5500;
const int kCheatAdvancePastCutscene = 4; /* levels 0->1->2 then cutscene */
const int kCheatAdvanceToLateLevel = 6;

/* The whole game running on mocks, for tests that look inside a level. */
class RunningGame {
public:
  RunningGame()
      : mock_gr_(std::make_unique<MockGraphicalInterface>()),
        gr_(std::make_unique<SimpleGraphicalInterface<std::string>>(
            mock_gr_.get())),
        mock_snd_(std::make_unique<MockSoundInterface>()),
        snd_(std::make_unique<SimpleSoundInterface<std::string>>(
            mock_snd_.get())),
        underlying_(std::make_unique<StdFileManager>()),
        fm_(std::make_unique<CachingReadOnlyFileManager>(underlying_.get(),
                                                         ".txt")),
        pe_(std::make_unique<SwitchEvent<bool, bool>>(exit_, true_), gr_.get(),
            snd_.get(), std::make_unique<EmptyWriter>(),
            Size(kScreenW, kScreenH), fm_.get()),
        runner_(std::make_unique<DragonGameRunner>(pe_)) {}

  DragonGameController *Controller() { return runner_->GetTowerController(); }

  /* Press Enter through the opening screens until the first level starts. */
  LevelController *EnterFirstLevel() {
    for (int i = 0; i < 100; ++i) {
      if (runner_->GetActiveControllerName() == "level")
        return dynamic_cast<LevelController *>(
            Controller()->GetActiveController());
      for (int k = 0; k < kKeyPressCount; ++k)
        if (i == kKeyPressFrames[k])
          runner_->KeyDown(GUI_RETURN);
      runner_->Update();
    }
    return nullptr;
  }

private:
  bool exit_ = false;
  bool true_ = true;
  std::unique_ptr<MockGraphicalInterface> mock_gr_;
  std::unique_ptr<SimpleGraphicalInterface<std::string>> gr_;
  std::unique_ptr<MockSoundInterface> mock_snd_;
  std::unique_ptr<SimpleSoundInterface<std::string>> snd_;
  std::unique_ptr<StdFileManager> underlying_;
  std::unique_ptr<CachingReadOnlyFileManager> fm_;
  ProgramEngine pe_;
  std::unique_ptr<DragonGameRunner> runner_;
};

struct DrawCall {
  Index image;
  Point where;
  bool centered;
};

/* A drawer that paints nothing and writes down what it was asked to paint. */
class RecordingDrawer : public ScalingDrawer {
public:
  RecordingDrawer() : ScalingDrawer(nullptr, 2) {}
  void Draw(Index image, Point where, bool centered) override {
    calls.push_back({image, where, centered});
  }
  std::vector<DrawCall> calls;
};

/* How many live royals of this kind are walking around the level. */
int CountLiveRoyals(LevelController *level, RoyalKind kind) {
  int count = 0;
  for (Critter *critter : level->GetCritters())
    if (critter->Exists() && critter->GetType() == "royal" &&
        critter->GetRoyalKind() == kind)
      ++count;
  return count;
}

} // namespace

TEST_CASE("Royals are princes and princesses about equally often",
          "[royals][integration]") {
  srand(12345);
  int princes = 0;
  const int kCalls = 10000;
  for (int i = 0; i < kCalls; ++i)
    if (RandomRoyalKind() == RoyalKind::kPrince)
      ++princes;
  CHECK(princes >= 4500);
  CHECK(princes <= 5500);
}

TEST_CASE("Prince art and tower figures are loaded", "[royals][integration]") {
  RunningGame game;
  DragonGameController *ctrl = game.Controller();
  REQUIRE(ctrl != nullptr);
  CHECK(ctrl->GetImgSeq("prince").GetImageCount() == 2);
  CHECK(ctrl->GetImgSeq("prince_f").GetImageCount() == 2);
  CHECK(ctrl->GetImgSeq("prince_die").GetImageCount() == 7);
  CHECK(ctrl->GetImgSeq("prince_die_f").GetImageCount() == 7);
  CHECK(ctrl->GetImgSeq("royal_figures").GetImageCount() == 2);
}

TEST_CASE("Castle keeps royals in capture order and the newest flees first",
          "[royals][integration]") {
  srand(12345);
  RunningGame game;
  LevelController *level = game.EnterFirstLevel();
  REQUIRE(level != nullptr);
  Castle *castle = level->GetFirstCastle();
  REQUIRE(castle->HasDragon());

  castle->AddRoyals(
      {RoyalKind::kPrincess, RoyalKind::kPrince, RoyalKind::kPrince});
  castle->AddRoyals({RoyalKind::kPrincess});
  REQUIRE(castle->GetRoyals() ==
          std::vector<RoyalKind>{RoyalKind::kPrincess, RoyalKind::kPrince,
                                 RoyalKind::kPrince, RoyalKind::kPrincess});
  CHECK(castle->GetRoyalCount() == 4);

  int princesses_before = CountLiveRoyals(level, RoyalKind::kPrincess);
  castle->OnKnight("knight");
  CHECK(castle->GetRoyals() ==
        std::vector<RoyalKind>{RoyalKind::kPrincess, RoyalKind::kPrince,
                               RoyalKind::kPrince});
  CHECK(CountLiveRoyals(level, RoyalKind::kPrincess) == princesses_before + 1);

  int princes_before = CountLiveRoyals(level, RoyalKind::kPrince);
  castle->OnKnight("knight");
  CHECK(CountLiveRoyals(level, RoyalKind::kPrince) == princes_before + 1);
}

TEST_CASE("Castle draws its first four royals in order at their windows",
          "[royals][integration]") {
  srand(12345);
  RunningGame game;
  LevelController *level = game.EnterFirstLevel();
  REQUIRE(level != nullptr);
  Castle *castle = level->GetFirstCastle();
  castle->AddRoyals({RoyalKind::kPrince, RoyalKind::kPrincess,
                     RoyalKind::kPrince, RoyalKind::kPrince,
                     RoyalKind::kPrincess});

  RecordingDrawer drawer;
  castle->Draw(&drawer);

  /* The bare tower, then one figure per window; the fifth royal stays hidden. */
  REQUIRE(drawer.calls.size() == 5);
  const ImageSequence &tower = game.Controller()->GetImgSeq("castle");
  CHECK(drawer.calls[0].image == tower.GetImageAt(0));
  CHECK(drawer.calls[0].centered);

  const ImageSequence &figures = game.Controller()->GetImgSeq("royal_figures");
  const Index princess = figures.GetImageAt(0);
  const Index prince = figures.GetImageAt(1);
  const Index expected[] = {prince, princess, prince, prince};
  for (int i = 0; i < 4; ++i) {
    const DrawCall &call = drawer.calls[i + 1];
    CHECK(call.image == expected[i]);
    CHECK_FALSE(call.centered);
    CHECK(call.where == castle->GetPosition() + Point(-7 + 4 * i, -6));
  }
}

TEST_CASE("Royals scattering from a castle keep their kinds",
          "[royals][integration]") {
  srand(12345);
  RunningGame game;
  LevelController *level = game.EnterFirstLevel();
  REQUIRE(level != nullptr);
  Castle *castle = level->GetFirstCastle();

  /* The dragon is away, so everyone panics. */
  castle->SetDragon(nullptr);
  castle->AddRoyals(
      {RoyalKind::kPrincess, RoyalKind::kPrince, RoyalKind::kPrince});

  int princesses_before = CountLiveRoyals(level, RoyalKind::kPrincess);
  int princes_before = CountLiveRoyals(level, RoyalKind::kPrince);
  castle->OnKnight("knight");

  CHECK(castle->GetRoyals().empty());
  CHECK_FALSE(castle->IsBroken());
  CHECK(CountLiveRoyals(level, RoyalKind::kPrincess) == princesses_before + 1);
  CHECK(CountLiveRoyals(level, RoyalKind::kPrince) == princes_before + 2);
}

TEST_CASE("Tutorial names whichever royal spawned last",
          "[royals][integration]") {
  TutorialTextEntity text(0, Point(), nullptr, nullptr);
  TutorialLevelOne tutorial(&text, false, false);
  tutorial.Notify(TutorialEvent::KnightKilled);

  tutorial.Notify(TutorialEvent::PrinceGenerate);
  REQUIRE_FALSE(text.GetLatestText().empty());
  CHECK(text.GetLatestText()[0] == "prince in sight!");

  tutorial.Notify(TutorialEvent::PrincessGenerate);
  REQUIRE_FALSE(text.GetLatestText().empty());
  CHECK(text.GetLatestText()[0] == "princess in sight!");
}

TEST_CASE("Simulation reaches level and menu, sound toggle writes to file",
          "[simulation][integration]") {
  try {
    srand(12345);

    {
      auto p_mock_gr = std::make_unique<MockGraphicalInterface>();
      auto p_gr = std::make_unique<SimpleGraphicalInterface<std::string>>(
          p_mock_gr.get());
      auto p_mock_snd = std::make_unique<MockSoundInterface>();
      auto p_snd =
          std::make_unique<SimpleSoundInterface<std::string>>(p_mock_snd.get());

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
      auto p_gl = std::make_unique<DragonGameRunner>(pe);

      bool reached_level = false;
      bool reached_menu = false;
      std::string sound_content_before;
      bool sound_file_flipped = false;

      for (int i = 0; i < kSimulationFrames && !b_exit; ++i) {
        /* Name of current screen (e.g. "level", "menu", "logo"). */
        std::string screen_name = p_gl->GetActiveControllerName();
        if (screen_name == "level")
          reached_level = true;
        if (screen_name == "menu")
          reached_menu = true;

        /* Press Enter to advance through logo and start into first level. */
        for (int k = 0; k < kKeyPressCount; ++k)
          if (i == kKeyPressFrames[k])
            p_gl->KeyDown(GUI_RETURN);

        if (screen_name == "level") {
          /* Press Space to take off dragon from the tower. */
          if (i == 35)
            p_gl->KeyDown(static_cast<GuiKeyType>(' '));
          /* Move cursor so dragon flies toward that point (first flight leg).
           */
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
      auto p_snd =
          std::make_unique<SimpleSoundInterface<std::string>>(p_mock_snd.get());

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
      auto p_gl = std::make_unique<DragonGameRunner>(pe);
      DragonGameController *ctrl = p_gl->GetTowerController();
      if (ctrl)
        ctrl->SetCheatsOnSetting(true);

      bool reached_game_over = false;
      int n_cheat_advance_count = 0;
      int n_cheat_late_count = 0;
      bool did_load_chapter = false;
      bool in_wait_phase = false;
      bool opened_menu_for_chapter = false;
      int menu_nav_step = 0;
      int exit_phase = 0; /* 0=none, 1=from game over, 2=score, ... */
      int menu_exit_step = 0;

      for (int i = 0; i < kGameOverSimulationFrames && !b_exit; ++i) {
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
                     !did_load_chapter && !opened_menu_for_chapter &&
                     i >= 110) {
              p_gl->KeyDown(GUI_ESCAPE);
              opened_menu_for_chapter = true;
            }
          }
          /* After load chapter: advance to one of the last levels via cheat. */
          if (did_load_chapter &&
              n_cheat_late_count < kCheatAdvanceToLateLevel &&
              i >= 200 + n_cheat_late_count * 15) {
            p_gl->KeyDown(static_cast<GuiKeyType>('\\'));
            n_cheat_late_count++;
          }
          /* Start wait phase: do nothing until game over. */
          if (did_load_chapter &&
              n_cheat_late_count >= kCheatAdvanceToLateLevel &&
              !in_wait_phase) {
            in_wait_phase = true;
          }
        }

        if (screen_name == "menu" && opened_menu_for_chapter &&
            !did_load_chapter) {
          /* Main menu: continue=0, restart=1, load chapter=2. Step 0,1: DOWN
           * x2; step 2: Enter (open load chapter). Step 3: DOWN (chapter 2); 4:
           * Enter.
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

        /* Exit the game after game over: game over -> score -> start -> level
         * -> menu -> select exit. */
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
