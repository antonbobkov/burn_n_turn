#include "core.h"
#include "entities.h"
#include "level.h"
#include "tutorial.h"

struct TwrGlobalController;
struct BasicController;
struct AdvancedController;
#include "fireball.h"

/** Base controller: holds pGl, rBound; virtual input/Update (OnKey, OnMouse,
 * Fire, etc.). */
struct GameController : virtual public SP_Info {
  SSP<TwrGlobalController> pGl;

  Rectangle rBound;

  GameController(const GameController &gc)
      : pGl(this, gc.pGl), rBound(gc.rBound) {}

  GameController(SP<TwrGlobalController> pGl_, Rectangle rBound_ = Rectangle())
      : pGl(this, pGl_), rBound(rBound_) {}

  virtual void Update() {}
  virtual void OnKey(GuiKeyType c, bool bUp) {}
  virtual void OnMouse(Point pPos) {}
  virtual void OnMouseDown(Point pPos) {}
  virtual void OnMouseUp() {}
  virtual void DoubleClick() {}
  virtual void Fire() {}
};

/** Plays level music from vThemes; SwitchTheme/StopMusic/ToggleOff control
 * playback. */
struct BackgroundMusicPlayer {
  int nCurrTheme;
  std::vector<Index> vThemes;
  SP<Soundic> pSnd;

  bool bOff;

  BackgroundMusicPlayer() : nCurrTheme(-1), pSnd(0), bOff(false) {}

  void SwitchTheme(int nTheme);
  void StopMusic();

  void ToggleOff();
};

enum { BG_MUSIC_CHANNEL = 0 };

enum {
  BG_BACKGROUND = 0,
  BG_SLOW_BACKGROUND = 1,
  BG_BACKGROUND2 = 2,
  BG_SLOW_BACKGROUND2 = 3,
  BG_BACKGROUND3 = 4,
  BG_SLOW_BACKGROUND3 = 5
};

struct TowerDataWrap;

struct MenuController;

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

  std::list<SP<TimedFireballBonus>> lsBonusesToCarryOver;

  TwrGlobalController(SP<ScalingDrawer> pDr_, SP<NumberDrawer> pNum_,
                      SP<NumberDrawer> pBigNum_, SP<FontWriter> pFancyNum_,
                      SP<Soundic> pSndRaw_, const LevelStorage &vLvl_,
                      Rectangle rBound_, TowerDataWrap *pWrp_,
                      FilePath fp = FilePath());

  TowerDataWrap *pWrp;

  void StartUp();
  void Next();
  void Restart(int nActive_ = -1);
  void Menu();

  // Point pCursorPos;
};

#include "critters.h"
#include "screen_controllers.h"

ProgramInfo GetProgramInfo();

struct Slime;
struct Sliminess;

#include "critter_generators.h"
#include "dragon.h"
#include "gameplay.h"

/** Fancy road: draws tiled road image; needs AdvancedController for resources.
 */
struct FancyRoad : public Road {
  FancyRoad(const Road &rd, SP<AdvancedController> pAd_)
      : Road(rd), pAd(this, pAd_) {}
  SSP<AdvancedController> pAd;

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);
};

/** Holds exit event and graphics/sound interfaces for tower game setup. */
struct TowerDataWrap {
  SP<Event> pExitProgram;

  SP<GraphicalInterface<Index>> pGr;
  SP<SoundInterface<Index>> pSm;

  FilePath fp;

  SP<ScalingDrawer> pDr;
  SP<NumberDrawer> pNum;
  SP<NumberDrawer> pBigNum;
  SP<FontWriter> pFancyNum;

  LevelStorage vLvl;
  SP<TwrGlobalController> pCnt;

  TowerDataWrap(ProgramEngine pe);

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
};
