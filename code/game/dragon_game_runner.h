#ifndef TOWER_DEFENSE_DRAGON_GAME_RUNNER_H
#define TOWER_DEFENSE_DRAGON_GAME_RUNNER_H

#include "game_utils/game_runner_interface.h"
#include "wrappers/geometry.h"
#include "utils/smart_pointer.h"
#include <memory>

struct DragonGameController;
class FilePath;
struct LevelLayout;
class FileManager;
struct FontWriter;
struct ScalingDrawer;
struct NumberDrawer;

/** The tower's chest: exit signal and the realm's sight and sound, for when
 * the game is first raised. */
class TowerDataWrap {
public:
  TowerDataWrap(ProgramEngine const& pe);

  FilePath *GetFilePath() const { return fp_.get(); }

  Event* pExitProgram;

  GraphicalInterface<Index> *pGr;
  SoundInterface<Index> *pSm;

  std::unique_ptr<ScalingDrawer> pDr;
  std::unique_ptr<ScalingDrawer> pBigDr;
  std::unique_ptr<NumberDrawer> pNum;
  std::unique_ptr<NumberDrawer> pBigNum;
  std::unique_ptr<FontWriter> pFancyNum;

  FileManager *p_fm_;
  std::unique_ptr<FilePath> fp_;

  std::vector<LevelLayout> vLvl;
  std::unique_ptr<DragonGameController> pCnt;

  Size szActualRez;
};

/** The top of the tower: holds the chest, builds the chapters and runs the
 * tale. */
class DragonGameRunner : public GameRunner {
  TowerDataWrap *pData;

public:
  std::string get_class_name() override { return "DragonGameRunner"; }
  DragonGameRunner(ProgramEngine const& pe);
  ~DragonGameRunner();

  /*virtual*/ void Update();

  /*virtual*/ void KeyDown(GuiKeyType nCode);
  /*virtual*/ void KeyUp(GuiKeyType nCode);
  /*virtual*/ void MouseMove(Point pPos);
  /*virtual*/ void MouseDown(Point pPos);
  /*virtual*/ void MouseUp();
  /*virtual*/ void DoubleClick();
  /*virtual*/ void Fire();

  /** For scribes and trials: who holds the tower now (levels, menu, etc.). */
  DragonGameController *GetTowerController() const;
  /** Name of the screen before you (e.g. menu, start, level). */
  std::string GetActiveControllerName() const;
  /** True when the dragon has claimed the day (for the trial of simulation). */
  bool IsOnGameOverScreen() const;
};

#endif
