#ifndef TOWER_DEFENSE_GAME_H
#define TOWER_DEFENSE_GAME_H

#include "common.h"
#include "core.h"
#include "entities.h"
#include "file_utils.h"
#include "fireball.h"
#include "level.h"
#include "tutorial.h"

/** Base controller: holds pGl, rBound; virtual input/Update (OnKey, OnMouse,
 * Fire, etc.). */
struct GameController : virtual public SP_Info {
  SSP<TwrGlobalController> pGl;

  Rectangle rBound;

  GameController(const GameController &gc)
      : pGl(this, gc.pGl), rBound(gc.rBound) {}

  GameController(smart_pointer<TwrGlobalController> pGl_,
                 Rectangle rBound_ = Rectangle())
      : pGl(this, pGl_), rBound(rBound_) {}

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

#include "screen_controllers.h"

struct TowerDataWrap;

/** Global game state: level storage, active controller, graphics/sound, score,
 * savable options, music. */
struct TwrGlobalController : virtual public SP_Info {
  std::vector<ASSP<GameController>> vCnt;
  unsigned nActive;

  std::vector<int> vLevelPointers;

  SSP<MenuController> pMenu;

  SSP<Graphic> pGraph;
  SSP<ScalingDrawer> pDr;
  SSP<NumberDrawer> pNum;
  SSP<NumberDrawer> pBigNum;
  SSP<FontWriter> pFancyNum;

  SSP<Soundic> pSndRaw;
  SSP<SoundInterfaceProxy> pSnd;

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

  TwrGlobalController(smart_pointer<ScalingDrawer> pDr_,
                      smart_pointer<NumberDrawer> pNum_,
                      smart_pointer<NumberDrawer> pBigNum_,
                      smart_pointer<FontWriter> pFancyNum_,
                      smart_pointer<Soundic> pSndRaw_,
                      const LevelStorage &vLvl_, Rectangle rBound_,
                      TowerDataWrap *pWrp_, FilePath *fp);

  TowerDataWrap *pWrp;

  smart_pointer<TwrGlobalController> pSelf;

  void StartUp(smart_pointer<TwrGlobalController> pSelf);
  void Next();
  void Restart(int nActive_ = -1);
  void Menu();
};

#include "critter_generators.h"
#include "critters.h"
#include "dragon.h"
#include "gameplay.h"
#include "screen_controllers.h"

/** Fancy road: draws tiled road image; needs AdvancedController for resources.
 */
struct FancyRoad : public Road {
  FancyRoad(const Road &rd, smart_pointer<AdvancedController> pAd_)
      : Road(rd), pAd(this, pAd_) {}
  SSP<AdvancedController> pAd;

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

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
  smart_pointer<TwrGlobalController> pCnt;

  Size szActualRez;
};

/** Top-level tower game: owns TowerDataWrap, builds and runs level flow. */
class TowerGameGlobalController : public GlobalController {
  TowerDataWrap *pData;

public:
  TowerGameGlobalController(ProgramEngine pe);
  ~TowerGameGlobalController();

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
  TwrGlobalController *GetTowerController() const;
  /** Active screen index (0=menu, 1..=logos/start/levels). */
  unsigned GetActiveControllerIndex() const;
  /** Total number of screens. */
  unsigned GetControllerCount() const;
  /** Name of current screen for simulation (e.g. "menu", "start", "level"). */
  std::string GetActiveControllerName() const;
};

#endif
