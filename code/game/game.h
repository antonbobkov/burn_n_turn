#ifndef TOWER_DEFENSE_GAME_H
#define TOWER_DEFENSE_GAME_H

#include "common.h"
#include "core.h"
#include "entities.h"
#include "utils/file_utils.h"
#include "fireball.h"
#include "wrappers/font_writer.h"
#include "wrappers/gui_key_type.h"
#include "level.h"
#include "utils/smart_pointer.h"
#include "tutorial.h"
#include <memory>

/** Base controller: holds pGl, rBound; virtual input/Update (OnKey, OnMouse,
 * Fire, etc.). */
struct GameController : virtual public SP_Info {
  std::string get_class_name() override { return "GameController"; }
  DragonGameControllerList *pGl;

  Rectangle rBound;

  GameController(const GameController &gc)
      : pGl(gc.pGl), rBound(gc.rBound) {}

  GameController(DragonGameControllerList *pGl_,
                 Rectangle rBound_ = Rectangle())
      : pGl(pGl_), rBound(rBound_) {}

  virtual void Update() {}
  virtual void OnKey(GuiKeyType c, bool bUp) {}
  virtual void OnMouse(Point pPos) {}
  virtual void OnMouseDown(Point pPos) {}
  virtual void OnMouseUp() {}
  virtual void DoubleClick() {}
  virtual void Fire() {}

  /** Name of this screen for simulation/inspection (e.g. "menu", "level"). */
  virtual std::string GetControllerName() const { return "basic"; }
};

#include "game/controller/basic_controllers.h"
#include "game/controller/menu_controller.h"

struct TowerDataWrap;

/** Global game state: level storage, active controller, graphics/sound, score,
 * savable options, music. */
struct DragonGameControllerList {
  std::vector<smart_pointer<GameController>> vCnt;
  unsigned nActive;

  std::vector<int> vLevelPointers;

  smart_pointer<MenuController> pMenu;

  smart_pointer<GraphicalInterface<Index>> pGraph;
  smart_pointer<ScalingDrawer> pDr;
  smart_pointer<NumberDrawer> pNum;
  smart_pointer<NumberDrawer> pBigNum;
  smart_pointer<FontWriter> pFancyNum;

  smart_pointer<SoundInterface<Index>> pSndRaw;
  smart_pointer<SoundInterfaceProxy> pSnd;

  LevelStorage vLvl;

  Preloader pr;

  int nScore;
  int nHighScore;

  bool bAngry;

  SavableVariable<int> snProgress;
  SavableVariable<bool> sbSoundOn;
  SavableVariable<bool> sbMusicOn;
  SavableVariable<bool> sbTutorialOn;
  SavableVariable<bool> sbFullScreen;
  SavableVariable<bool> sbCheatsOn;
  SavableVariable<bool> sbCheatsUnlocked;

  Rectangle rBound;

  BackgroundMusicPlayer plr;

  std::list<smart_pointer<TimedFireballBonus>> lsBonusesToCarryOver;

  DragonGameControllerList(smart_pointer<ScalingDrawer> pDr_,
                      smart_pointer<NumberDrawer> pNum_,
                      smart_pointer<NumberDrawer> pBigNum_,
                      smart_pointer<FontWriter> pFancyNum_,
                      smart_pointer<SoundInterface<Index>> pSndRaw_,
                      const LevelStorage &vLvl_, Rectangle rBound_,
                      TowerDataWrap *pWrp_, FilePath *fp);

  TowerDataWrap *pWrp;

  DragonGameControllerList *pSelf;

  void StartUp(DragonGameControllerList *pSelf);
  void Next();
  void Restart(int nActive_ = -1);
  void Menu();
};

#include "game/controller/level_controller.h"
#include "critter_generators.h"
#include "critters.h"
#include "dragon.h"
#include "gameplay.h"


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
  std::unique_ptr<DragonGameControllerList> pCnt;

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
  DragonGameControllerList *GetTowerController() const;
  /** Active screen index (0=menu, 1..=logos/start/levels). */
  unsigned GetActiveControllerIndex() const;
  /** Total number of screens. */
  unsigned GetControllerCount() const;
  /** Name of current screen for simulation (e.g. "menu", "start", "level"). */
  std::string GetActiveControllerName() const;
};

#endif
