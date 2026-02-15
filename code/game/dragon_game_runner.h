#ifndef TOWER_DEFENSE_DRAGON_GAME_RUNNER_H
#define TOWER_DEFENSE_DRAGON_GAME_RUNNER_H

#include "game/controller/dragon_game_controller.h"
#include "game/level.h"
#include "game_utils/game_runner_interface.h"
#include "utils/file_utils.h"
#include "utils/smart_pointer.h"
#include <memory>

/** Holds exit event and graphics/sound interfaces for tower game setup. */
class TowerDataWrap {
public:
  TowerDataWrap(ProgramEngine pe);

  FilePath *GetFilePath() const { return fp_.get(); }

  smart_pointer<Event> pExitProgram;

  smart_pointer<GraphicalInterface<Index>> pGr;
  smart_pointer<SoundInterface<Index>> pSm;

  smart_pointer<ScalingDrawer> pDr;
  smart_pointer<NumberDrawer> pNum;
  smart_pointer<NumberDrawer> pBigNum;
  smart_pointer<FontWriter> pFancyNum;

  FileManager *p_fm_;
  std::unique_ptr<FilePath> fp_;

  LevelStorage vLvl;
  std::unique_ptr<DragonGameController> pCnt;

  Size szActualRez;
};

/** Top-level tower game: owns TowerDataWrap, builds and runs level flow. */
class DragonGameRunner : public GameRunner {
  TowerDataWrap *pData;

public:
  std::string get_class_name() override { return "DragonGameRunner"; }
  DragonGameRunner(ProgramEngine pe);
  ~DragonGameRunner();

  /*virtual*/ void Update();

  /*virtual*/ void KeyDown(GuiKeyType nCode);
  /*virtual*/ void KeyUp(GuiKeyType nCode);
  /*virtual*/ void MouseMove(Point pPos);
  /*virtual*/ void MouseDown(Point pPos);
  /*virtual*/ void MouseUp();
  /*virtual*/ void DoubleClick();
  /*virtual*/ void Fire();

  /** For simulation/inspection: current tower controller (levels, menu, etc.).
   */
  DragonGameController *GetTowerController() const;
  /** Active screen index (0=menu, 1..=logos/start/levels). */
  unsigned GetActiveControllerIndex() const;
  /** Total number of screens. */
  unsigned GetControllerCount() const;
  /** Name of current screen for simulation (e.g. "menu", "start", "level"). */
  std::string GetActiveControllerName() const;
};

#endif
