#include "GuiGen.h"
#include "SuiGen.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <fstream>
#include <sstream>

#include <math.h>
#include <stdlib.h>

#include "Preloader.h"

#include "Global.h"

#define FULL_VERSION
#define PC_VERSION
//#define KEYBOARD_CONTROLS

#ifdef KEYBOARD_CONTROLS
//#define JOYSTICK_TUTORIAL
#endif

#ifdef PC_VERSION
//#define FULL_SCREEN_VERSION
//#define LOADING_SCREEN
#endif

//#define SMALL_SCREEN_VERSION

#ifndef FULL_VERSION
#define TRIAL_VERSION
#endif

#ifdef SMALL_SCREEN_VERSION
#define FLIGHT_POWER_MODE
#endif

//#define UNDERLINE_UNIT_TEXT

const unsigned nFramesInSecond = 30;

const float fSpeedConstant = 1.5F / (float(nFramesInSecond) / 10);

const float fDragonSpeed = 7.F * fSpeedConstant;
const float fKnightSpeed = .75F * fSpeedConstant;
const float fKnightFireSpeed = 1.5F * fSpeedConstant;
const float fSkeletonSpeed = 1.F * fSpeedConstant;
const float fPrincessSpeed = .5F * fSpeedConstant;
const float fTraderSpeed = .5F * fSpeedConstant;
const float fMageSpeed = .5F * fSpeedConstant;
const float fSlimeSpeed = 1.F * fSpeedConstant;
const float fGhostSpeedMultiplier =
    1.3f; // where ghost speed = knightspeed*ghostspeedmultiplier

const float fFireballSpeed = 1.F * fSpeedConstant;

const float fDeathMultiplier = 2.F;

const unsigned nBonusOnGroundTime = 60 * nFramesInSecond;

const unsigned nBonusCheatTime = 25 * nFramesInSecond;
const unsigned nBonusPickUpTime = 35 * nFramesInSecond;
const unsigned nBonusTraderTime = 55 * nFramesInSecond;

const float fBonusTimeMutiplierOne = .85f;
const float fBonusTimeMutiplierTwo = .7f;

const float fIncreaseRateFraction1 = .25F;
const float fIncreaseRateFraction2 = .5F;

const float fIncreaseKnightRate1 = 2.F;
const float fIncreaseKnightRate2 = 3.F;

const float fIncreaseTraderRate1 = 1.5F;
const float fIncreaseTraderRate2 = 2.F;

const int nSummonChance = nFramesInSecond * 12;
const int nSummonRadius = 60;

const int nGolemHealthMax = 70;
const int nSlimeHealthMax = 30;

// flying constatns
const float fFlightCoefficient = 6.5F;
const float fTowerClickRadius = 25;
const float fDragonClickRadius = 25;

// Bonus stuff
const float fInitialFrequency = 2.F;
const int nInitialRegen = 0; // in deciseconds
const int nRegenDelay = 10;  // in deciseconds

#ifndef KEYBOARD_CONTROLS
const int nInitialFireballs = 4;
const unsigned nFireballsPerBonus = 2U;
#else
const int nInitialFireballs = 6;
const unsigned nFireballsPerBonus = 3U;
#endif

extern int nSlimeMax;

#pragma warning(disable : 4250)
#pragma warning(disable : 4355)

using namespace Gui;

typedef Gui::GraphicalInterface<Index> Graphic;
typedef Gui::SoundInterface<Index> Soundic;

struct TwrGlobalController;

extern MessageWriter *pWr;

/** Polar coordinates (radius r, angle a); converts to/from fPoint, supports
 * multiply for rotation. */
struct Polar {
  float r, a;
  Polar() : r(0), a(0) {}
  Polar(float a_, float r_) : a(a_), r(r_) {}
  Polar(fPoint p);
  Polar operator*(Polar p) { return Polar(a + p.a, r * p.r); }

  fPoint TofPoint() { return fPoint(r * cos(a), r * sin(a)); }
};

/**
 * Turns two direction keys (e.g. left, up) into a single diagonal direction.
 * Used for steering or aiming from keyboard or joystick.
 */
fPoint ComposeDirection(int dir1, int dir2);

/** Picks one direction within a spread (for multi-shot fire). */
fPoint GetWedgeAngle(fPoint fDir, float dWidth, unsigned nWhich,
                     unsigned nHowMany);

/** Random direction near fDir within fRange * 2π. */
fPoint RandomAngle(fPoint fDir = fPoint(1, 0), float fRange = 1.F);

/** Base for drawing an image at a point; ScalingDrawer adds scale and color
 * key. */
struct Drawer : virtual public SP_Info {
  SSP<Graphic> pGr;

  Drawer() : pGr(this, 0) {}

  virtual void Draw(Index nImg, Point p, bool bCentered = true) = 0;
};

/** Drawer that scales images by nFactor and uses cTr as transparency key. */
struct ScalingDrawer : public Drawer {
  unsigned nFactor;
  Color cTr;

  ScalingDrawer(SP<Graphic> pGr_, unsigned nFactor_,
                Color cTr_ = Color(0, 255, 255))
      : nFactor(nFactor_), cTr(cTr_) {
    pGr = pGr_;
  }

  void Scale(Index &pImg, int nFactor_ = -1);

  /*virtual*/ void Draw(Index nImg, Point p, bool bCentered = true);

  /*virtual*/ Index LoadImage(std::string strFile);
};

/** Remove from list any element for which bExist is false. */
template <class T> void CleanUp(std::list<T> &ar) {
  for (typename std::list<T>::iterator itr = ar.begin(), etr = ar.end();
       itr != etr;) {
    if (!(*itr)->bExist)
      ar.erase(itr++);
    else
      ++itr;
  }
}

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

/** Draws digits/words from a font bitmap; CacheColor/DrawColorWord for
 * recolored text. */
struct NumberDrawer : virtual public SP_Info {
  SSP<ScalingDrawer> pDr;
  std::vector<int> vImgIndx;
  std::vector<Index> vImg;

  std::map<Color, std::vector<Index>> mpCachedRecolorings;

  void CacheColor(Color c);

  /**
   * Loads the font: reads which character is at which position in the image,
   * loads the image and cuts it into small letter tiles, scales them, and
   * discards the original full image.
   */
  NumberDrawer(SP<ScalingDrawer> pDr_, std::string sFontPath,
               std::string sFontName);

  std::string GetNumber(unsigned n, unsigned nDigits = 0);

  void DrawNumber(unsigned n, Point p, unsigned nDigits = 0) {
    DrawWord(GetNumber(n, nDigits), p);
  }

  void DrawColorNumber(unsigned n, Point p, Color c, unsigned nDigits = 0) {
    DrawColorWord(GetNumber(n, nDigits), p, c);
  }

  void DrawWord(std::string s, Point p, bool bCenter = false);

  /**
   * Draws text at a position in a given color. Optionally centers the text.
   * Uses pre-made colored letters when available; otherwise colors each letter
   * on the fly. Skips characters that are not in the font.
   */
  void DrawColorWord(std::string s, Point p, Color c, bool bCenter = false);
};

/** Copy ASSP so the new pointer is owned by pInf. */
template <class T> ASSP<T> CopyASSP(SP_Info *pInf, ASSP<T> pAsp) {
  return ASSP<T>(pInf, pAsp);
}

/** Copy each ASSP from from into to, rebinding each to pInf. */
template <class T> void CopyArrayASSP(SP_Info *pInf, const T &from, T &to) {
  for (typename T::const_iterator itr = from.begin(), etr = from.end();
       itr != etr; ++itr)
    to.push_back(CopyASSP(pInf, *itr));
}

struct LevelLayout;
typedef std::vector<LevelLayout> LevelStorage;

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

struct SoundControls;

struct TimedFireballBonus;

struct TowerDataWrap;

/** Persists a value in a file; Get/Set, optional load on construction and save
 * on set. */
template <class T> class SavableVariable {
  T var;
  std::string sFileName;

public:
  SavableVariable(std::string sFileName_, T var_default, bool bLoad = true)
      : sFileName(sFileName_) {
    if (!bLoad)
      var = var_default;
    else {
      std::ifstream ifs(sFileName.c_str());
      ifs >> var;

      if (ifs.fail())
        var = var_default;
    }
  }

  void Save() {
    std::ofstream ofs(sFileName.c_str());
    ofs << var;
  }

  void Set(T new_var, bool bSave = true) {
    var = new_var;
    if (bSave)
      Save();
  }

  T Get() { return var; }

  const T *GetConstPointer() { return &var; }
};

/** Flip the boolean in sv and persist it. */
void BoolToggle(SavableVariable<bool> &sv) { sv.Set(!sv.Get()); }

/** Wraps Soundic and gates playback on bSoundOn (Toggle/Get). */
class SoundInterfaceProxy : virtual public SP_Info {
  bool bSoundOn;
  SSP<Soundic> pSndRaw;

public:
  SoundInterfaceProxy(SP<Soundic> pSndRaw_)
      : pSndRaw(this, pSndRaw_), bSoundOn(true) {}

  void PlaySound(Index i, int nChannel = -1, bool bLoop = false);

  void Toggle() { bSoundOn = !bSoundOn; }
  bool Get() { return bSoundOn; }
};

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

/** Controller that draws a single full-screen image and advances on key. */
struct SimpleController : public GameController {
  Index nImage;

  SimpleController(SP<TwrGlobalController> pGraph, std::string strFileName);
  ~SimpleController();

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
};

/** Controller that draws background + text that flashes every second. */
struct FlashingController : public GameController {
  Index nImage, nText;
  unsigned nTimer;
  bool bShow;

  FlashingController(SP<TwrGlobalController> pGraph, std::string strFileName,
                     std::string strTextName);
  ~FlashingController();

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
};

/** Root entity; bExist flag, virtual dtor. */
struct Entity : virtual public SP_Info {
  bool bExist;
  Entity() : bExist(true) {}
  virtual ~Entity() {}
};

/** Entity that can Move and Update each frame. */
struct EventEntity : virtual public Entity {
  virtual void Move() {}
  virtual void Update() {}
};

/** Entity with a screen position (GetPosition). */
struct ScreenEntity : virtual public Entity {
  virtual Point GetPosition() { return Point(0, 0); }
};

struct VisualEntity : virtual public ScreenEntity {
  virtual void Draw(SP<ScalingDrawer> pDr) {}
  virtual float GetPriority() { return 0; }
  VisualEntity() = default;
  VisualEntity(const VisualEntity &) = default;
  VisualEntity &operator=(const VisualEntity &) = default;
  /* Move assignment deleted to avoid -Wvirtual-move-assign: defaulted move
   * with virtual bases is unsound; copy is used instead (e.g. in vector). */
  VisualEntity &operator=(VisualEntity &&) = delete;
};

/** Split string on newlines into a vector of lines (appends \\n to s). */
std::vector<std::string> BreakUpString(std::string s);

/** VisualEntity that draws multi-line text via NumberDrawer at a position. */
struct TextDrawEntity : virtual public VisualEntity {
  float dPriority;
  Point pos;
  bool bCenter;
  std::vector<std::string> vText;
  SSP<NumberDrawer> pNum;

  TextDrawEntity(float dPriority_, Point pos_, bool bCenter_, std::string sText,
                 SP<NumberDrawer> pNum_)
      : dPriority(dPriority_), pos(pos_), bCenter(bCenter_), pNum(this, pNum_) {
    SetText(sText);
  }

  void SetText(std::string sText) { vText = BreakUpString(sText); }

  /** Draw vText lines at pos with pNum; advance p.y per line. */
  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return dPriority; }
};

/** Scrolling tutorial text; SetText queues sNewText and Update scrolls between
 * sText and sNewText. */
struct TutorialTextEntity : virtual public EventEntity, public VisualEntity {
  float dPriority;
  Point pos;

  std::vector<std::string> sText;
  std::vector<std::string> sNewText;

  SSP<NumberDrawer> pNum;

  Timer t;
  int nOffset;
  int nDelta;

  int nTextVerticalSpacing;

  const bool *pIsTutorialOn;

  TutorialTextEntity(float dPriority_, Point pos_, SP<NumberDrawer> pNum_,
                     const bool *pIsTutorialOn_)
      : dPriority(dPriority_), pos(pos_), pNum(this, pNum_), t(1), nOffset(0),
        nTextVerticalSpacing(7), nDelta(1), pIsTutorialOn(pIsTutorialOn_) {}

  /** Set content to v; scrolls from sText to v or sets sText if empty. */
  void SetText(std::vector<std::string> v);

  /** Draw sText lines with vertical offset nOffset; skip if tutorial off. */
  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return dPriority; }

  /** Advances the scroll each tick; when scroll ends, the new text replaces the
   * old. */
  /*virtual*/ void Update();
};

#ifdef KEYBOARD_CONTROLS
#ifdef JOYSTICK_TUTORIAL
const std::string sSteerMessage = "steer with joystick";
const std::string sShootingMessage = "move joystick to shoot a fireball";
const std::string sTakeOffMessage = "press button to take off";
#else
const std::string sSteerMessage = "steer with left and right keys";
const std::string sShootingMessage = "shoot with arrow keys";
const std::string sTakeOffMessage = "press space to take off";
#endif
#else
const std::string sSteerMessage =
    "click and hold to steer"; // steer with joystick
const std::string sShootingMessage =
    "click anywhere to shoot a fireball"; // move joystick to shoot a fireball
const std::string sTakeOffMessage =
    "press space or click the tower to take off"; // press button to take off
#endif

/** First tutorial: tracks knight kill, flying, princess spawn/capture;
 * GetText/Update drive TutorialTextEntity. */
struct TutorialLevelOne {
  bool bKilledKnight;
  bool bFlying;
  bool bPrincessGenerated;
  bool bPrincessCaptured;

  // potential memory leak
  SP<TutorialTextEntity> pTexter;

  TutorialLevelOne()
      : bKilledKnight(false), bFlying(false), bPrincessGenerated(false),
        bPrincessCaptured(false), pTexter(0) {}

  /**
   * Chooses which tutorial lines to show for level one. While flying: either
   * "steer and fly back" or "steer, pick up princess, bring to tower". Before
   * first knight kill: shoot and aim at knights. When princess appears: either
   * "princess in sight, take off" or "capture four princesses". Otherwise
   * returns no lines.
   */
  std::vector<std::string> GetText();

  /** Pushes the current tutorial lines to the display. */
  void Update();

  void ShotFired() {}

  /** Set bKilledKnight and refresh tutorial text. */
  void KnightKilled();

  /** Set flying on and refresh text. */
  void FlyOn();

  /** Set flying off and refresh text. */
  void FlyOff();

  /** Mark princess spawned and refresh text. */
  void PrincessGenerate();

  /** Mark princess captured and refresh text. */
  void PrincessCaptured();
};

/** Second tutorial: trader spawn/kill and bonus pickup; GetText/Update drive
 * TutorialTextEntity. */
struct TutorialLevelTwo {
  bool bTraderGenerated;
  bool bTraderKilled;
  bool bBonusPickedUp;

  // potential memory leak
  SP<TutorialTextEntity> pTexter;

  TutorialLevelTwo()
      : bTraderGenerated(false), bTraderKilled(false), bBonusPickedUp(false),
        pTexter(0) {}

  /** Build tutorial lines from trader/bonus state. */
  std::vector<std::string> GetText();

  /** Pushes the current tutorial lines to the display. */
  void Update();

  /** Set bTraderKilled and refresh text. */
  void TraderKilled();

  /** Mark trader spawned and refresh text. */
  void TraderGenerate();

  /** Mark bonus picked up and refresh text. */
  void BonusPickUp();
};

/** VisualEntity with an ImageSequence: draws current frame, Update toggles by
 * timer or on position change. */
struct SimpleVisualEntity : virtual public EventEntity, public VisualEntity {
  float dPriority;

  unsigned nPeriod;
  Timer t;

  bool bTimer, bStep, bCenter;

  Point pPrev;

  ImageSequence seq;

  SimpleVisualEntity(float dPriority_, const ImageSequence &seq_, bool bCenter_,
                     unsigned nPeriod_)
      : dPriority(dPriority_), seq(seq_), nPeriod(nPeriod_),
        t(nPeriod_ * seq_.GetTime()), bCenter(bCenter_), bTimer(true),
        bStep(false), bImageToggle(false) {}

  SimpleVisualEntity(float dPriority_, const ImageSequence &seq_, bool bCenter_,
                     bool bStep_ = false)
      : dPriority(dPriority_), seq(seq_), bCenter(bCenter_), bTimer(false),
        bStep(bStep_), nPeriod(1), bImageToggle(false) {}

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return dPriority; }

  bool bImageToggle;

  /*virtual*/ void Update();
};

/** EventEntity that plays a SoundSequence on a timer; sets bExist false when
 * sequence ends. */
struct SimpleSoundEntity : virtual public EventEntity {
  unsigned nPeriod;
  Timer t;

  SoundSequence seq;
  SSP<SoundInterfaceProxy> pSnd;

  SimpleSoundEntity(const SoundSequence &seq_, unsigned nPeriod_,
                    SP<SoundInterfaceProxy> pSnd_)
      : seq(seq_), nPeriod(nPeriod_), t(nPeriod * seq_.GetTime()),
        pSnd(this, pSnd_) {}

  /*virtual*/ void Update();
};

/** SimpleVisualEntity with fixed position (no movement). */
struct Animation : public SimpleVisualEntity {
  Point pos;

  Animation(float dPriority_, const ImageSequence &seq, unsigned nTimeMeasure_,
            Point p, bool bCenter = false)
      : SimpleVisualEntity(dPriority_, seq, bCenter, nTimeMeasure_), pos(p) {}

  /*virtual*/ Point GetPosition() { return pos; }
};

/** Animation that runs once then sets bExist false (seq plays to end). */
struct AnimationOnce : public SimpleVisualEntity {
  Point pos;
  bool bOnce;

  AnimationOnce(float dPriority_, const ImageSequence &seq,
                unsigned nTimeMeasure_, Point p, bool bCenter = false)
      : SimpleVisualEntity(dPriority_, seq, bCenter, nTimeMeasure_), pos(p),
        bOnce(true) {}

  /*virtual*/ Point GetPosition() { return pos; }
  /*virtual*/ void Update();
};

/** VisualEntity that draws a single image at a fixed point. */
struct StaticImage : public VisualEntity {
  Index img;
  float dPriority;
  Point p;
  bool bCentered;

  StaticImage(Index img_, Point p_ = Point(0, 0), bool bCentered_ = false,
              float dPriority_ = 0)
      : img(img_), dPriority(dPriority_), p(p_), bCentered(bCentered_) {}

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /*virtual*/ Point GetPosition() { return p; }

  /*virtual*/ float GetPriority() { return dPriority; }
};

/** VisualEntity that draws a filled rectangle (no position). */
struct StaticRectangle : public VisualEntity {
  float dPriority;
  Rectangle r;
  Color c;

  StaticRectangle(Rectangle r_, Color c_, float dPriority_ = 0)
      : r(r_), c(c_), dPriority(dPriority_) {}

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /*virtual*/ Point GetPosition() { return Point(0, 0); }

  /*virtual*/ float GetPriority() { return dPriority; }
};

struct MenuDisplay;

typedef void (MenuDisplay::*EvntPntr)();

/** One menu item: size, label, callback (EvntPntr), disabled flag. */
struct MenuEntry : virtual public SP_Info {
  Size szSize;
  std::string sText;
  EvntPntr pTriggerEvent;
  bool bDisabled;

  MenuEntry(Size szSize_, std::string sText_, EvntPntr pTriggerEvent_,
            bool bDisabled_ = false)
      : szSize(szSize_), sText(sText_), pTriggerEvent(pTriggerEvent_),
        bDisabled(bDisabled_) {}
};

/** Return "on" or "off" for menu toggles. */
std::string OnOffString(bool b);

std::string SoundString() { return "sound: "; }
std::string MusicString() { return "music: "; }
std::string TutorialString() { return "tutorial: "; }
std::string FullTextString() { return "full screen: "; }

/** Holds menu entries and current selection index (nMenuPosition). */
struct MenuEntryManager {
  std::vector<MenuEntry> vEntries;
  int nMenuPosition;

  MenuEntryManager() : nMenuPosition(0) {}
};

/** In-game menu: draws entries, caret, handles mouse/key; submenus and option
 * toggles. */
struct MenuDisplay : virtual public EventEntity, public VisualEntity {
  MenuEntryManager *pCurr;

  MenuEntryManager memMain;
  MenuEntryManager memLoadChapter;
  MenuEntryManager memOptions;
  std::vector<std::string> vOptionText;

  int nMusic, nSound, nTutorial, nFullScreen, nCheats;
  bool bCheatsUnlocked;

  Point pLeftTop;
  SSP<NumberDrawer> pNum;

  SSP<Animation> pMenuCaret;

  SSP<MenuController> pMenuController;

  MenuDisplay(Point pLeftTop_, SP<NumberDrawer> pNum_,
              SP<Animation> pMenuCaret_, SP<MenuController> pMenuController_,
              bool bCheatsUnlocked_);

  /** Draws the menu entries and the caret at the current selection. */
  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return 0; }

  /*virtual*/ void Update() { pCurr->vEntries.at(0).bDisabled = false; }

  /** Updates which menu item is highlighted from the mouse position. */
  void OnMouseMove(Point pMouse);

  void PositionIncrement(bool bUp);

  void Boop();

  void Restart();
  void Continue();
  void MusicToggle();
  void SoundToggle();
  void TutorialToggle();
  void FullScreenToggle();
  void CheatsToggle();
  void Exit();

  void Escape();
  void LoadChapterSubmenu();
  void OptionsSubmenu();
  void UpdateMenuEntries();

  void Chapter1();
  void Chapter2();
  void Chapter3();
};

/** VisualEntity that draws a countdown number and sets bExist false when it
 * reaches 0. */
struct Countdown : public VisualEntity, public EventEntity {
  SSP<NumberDrawer> pNum;
  unsigned nTime, nCount;

  Countdown(SP<NumberDrawer> pNum_, unsigned nTime_)
      : pNum(this, pNum_), nTime(nTime_), nCount(0) {}

  /** Decrements the countdown every second; removes this when it reaches zero.
   */
  /*virtual*/ void Update();

  /** Draws the remaining countdown at a fixed screen position. */
  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /*virtual*/ Point GetPosition() { return Point(0, 0); }
};

/** ScreenEntity with radius for hit detection (HitDetection). */
struct PhysicalEntity : virtual public ScreenEntity {
  virtual unsigned GetRadius() { return 0; }

  /** True if circles (GetPosition, GetRadius) overlap. */
  bool HitDetection(SP<PhysicalEntity> pPh);
};

/** Tracks mouse for trackball-style steering (angle and fire). */
struct TrackballTracker {
  MouseTracker mtr;

  std::list<Point> lsMouse;
  int nMaxLength;
  bool trigFlag;
  int threshold;
  TrackballTracker();

  /** Records the latest relative movement and drops the oldest sample. */
  void Update();

  /** Returns true when the middle sample has the largest length (gesture peak).
   */
  bool IsTrigger();

  /** Second element of lsMouse (recent movement). */
  Point GetMovement() { return *(++lsMouse.begin()); }

  /** Returns the average of recent movement samples. */
  fPoint GetAvMovement();

  int GetLengthSq(Point p) { return p.x * p.x + p.y * p.y; }

  /** Difference in squared length between first and second sample; 0 if only
   * one sample. */
  int GetDerivative();
};

/** PhysicalEntity that can be hit (OnHit), has type (GetType) and image
 * (GetImage). */
struct ConsumableEntity : virtual public PhysicalEntity {
  virtual char GetType() = 0;
  virtual void OnHit(char cWhat) = 0;
  virtual Index GetImage() = 0;
};

/** Moving unit: position, velocity, bounds, radius; Move() steps and clamps or
 * kills on exit. */
struct Critter : virtual public PhysicalEntity, public SimpleVisualEntity {
  unsigned nRadius;
  fPoint fPos;
  fPoint fVel;

  Rectangle rBound;
  bool bDieOnExit;

  std::string sUnderText;

  /*virtual*/ unsigned int GetRadius() { return nRadius; }
  /*virtual*/ Point GetPosition() { return fPos.ToPnt(); }
  /*virtual*/ void Move();

  Critter(unsigned nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
          float dPriority, const ImageSequence &seq, unsigned nPeriod)
      : SimpleVisualEntity(dPriority, seq, true, nPeriod), rBound(rBound_),
        nRadius(nRadius_), fPos(fPos_), fVel(fVel_), bDieOnExit(true),
        sUnderText("") {}

  Critter(unsigned nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
          float dPriority, const ImageSequence &seq, bool bStep = false)
      : SimpleVisualEntity(dPriority, seq, true, true), rBound(rBound_),
        nRadius(nRadius_), fPos(fPos_), fVel(fVel_), bDieOnExit(true),
        sUnderText("") {}
};

/** Critter that advances position and toggles frame on a timer (tm). */
struct FancyCritter : virtual public PhysicalEntity, public SimpleVisualEntity {
  unsigned nRadius;
  fPoint fPos;
  fPoint fVel;

  Rectangle rBound;
  bool bDieOnExit;

  Timer tm;

  /*virtual*/ unsigned int GetRadius() { return nRadius; }
  /*virtual*/ Point GetPosition() { return fPos.ToPnt(); }
  /*virtual*/ void Move();

  FancyCritter(unsigned nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
               float dPriority, const ImageSequence &seq, unsigned nPeriod)
      : SimpleVisualEntity(dPriority, seq, true, false), rBound(rBound_),
        nRadius(nRadius_), fPos(fPos_), fVel(fVel_), bDieOnExit(true),
        tm(nPeriod) {}
};

/** Sort key for draw order: priority and height (operator<). */
struct ScreenPos {
  float fPriority;
  int nHeight;

  ScreenPos(float fPriority_, Point p) : fPriority(fPriority_), nHeight(p.y) {}

  bool operator<(const ScreenPos &sp) const {
    if (fPriority == sp.fPriority)
      return nHeight < sp.nHeight;
    return fPriority < sp.fPriority;
  }
};

/** GameController with draw/update/consumable lists; Update runs Move, Update,
 * then draws by priority. */
struct BasicController : public GameController {
  std::list<ASSP<VisualEntity>> lsDraw;
  std::list<ASSP<EventEntity>> lsUpdate;
  std::list<ASSP<ConsumableEntity>> lsPpl;

  /** Add visual entity to lsDraw. */
  void AddV(SP<VisualEntity> pVs);
  /** Adds an event entity to the update list. */
  void AddE(SP<EventEntity> pEv);

  template <class T> void AddBoth(T &t) {
    lsDraw.push_back(ASSP<VisualEntity>(this, t));
    lsUpdate.push_back(ASSP<EventEntity>(this, t));
  }

  /** Add scaled fullscreen StaticRectangle of color c to lsDraw. */
  void AddBackground(Color c);

  BasicController(const BasicController &b);
  bool bNoRefresh;
  BasicController(SP<TwrGlobalController> pGl_, Rectangle rBound, Color c);

  /**
   * Each frame: remove dead things from the lists, move everything that can
   * move, then update everyone. Draw everything in order from back to front.
   * Refresh the screen unless refresh is disabled.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos) { pGl->Next(); }
};

/** Cursor image and position; Draw/Update for rendering and click state. */
struct MouseCursor {
  bool bPressed;
  ImageSequence imgCursor;
  Point pCursorPos;
  TwrGlobalController *pGl;

  MouseCursor(ImageSequence imgCursor_, Point pCursorPos_,
              TwrGlobalController *pGl_)
      : imgCursor(imgCursor_), pCursorPos(pCursorPos_), pGl(pGl_),
        bPressed(false) {}

  void DrawCursor();
  void SetCursorPos(Point pPos);
};

/** Controller for pause/main menu: MenuDisplay, resume position. */
struct MenuController : public BasicController {
  int nResumePosition;
  SSP<MenuDisplay> pMenuDisplay;

  MouseCursor mc;

  SSP<TextDrawEntity> pHintText;
  SSP<TextDrawEntity> pOptionText;

  MenuController(SP<TwrGlobalController> pGl_, Rectangle rBound, Color c,
                 int nResumePosition_);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void OnMouse(Point pPos);
  /*virtual*/ void OnMouseDown(Point pPos);

  /*virtual*/ void Update();
};

struct StartScreenController : public BasicController {
  StartScreenController(SP<TwrGlobalController> pGl_, Rectangle rBound, Color c)
      : BasicController(pGl_, rBound, c) {}

  /** Advance to next screen and play start_game sound. */
  void Next();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos) { Next(); }
};

struct BuyNowController;

struct SlimeUpdater : public VisualEntity {
  BuyNowController *pBuy;

  SlimeUpdater(BuyNowController *pBuy_) : pBuy(pBuy_) {}

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);
  /*virtual*/ float GetPriority() { return 0; }
};

/** Controller for buy-now screen: slime animations and timer. */
struct BuyNowController : public BasicController {
  int t;
  std::vector<SP<Animation>> mSlimes;
  std::vector<fPoint> mSlimeVel;
  std::vector<fPoint> mSlimePos;
  int nSlimeCount;

  Timer tVel;

  BuyNowController(SP<TwrGlobalController> pGl_, Rectangle rBound, Color c);

  /** Set fVel to random direction, sometimes toward center; scale by speed. */
  void RandomizeVelocity(fPoint &fVel, fPoint pPos);

  /** Draw all slime animations. */
  void DrawSlimes();

  /**
   * Runs the normal controller update. Every so often, some slimes get a new
   * random direction. All slimes move and animate. Countdown runs. Screen
   * refreshes.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
};

struct Cutscene : public BasicController {
  SSP<FancyCritter> pCrRun;
  SSP<FancyCritter> pCrFollow;

  Timer tm;
  bool Beepy;

  bool bRelease;

  /**
   * Sets up the chase: black screen, one character running and one waiting.
   * Runner starts left or right depending on flip; when the runner reaches the
   * middle, the chaser is released. Beep/boop timer for sound.
   */
  Cutscene(SP<TwrGlobalController> pGl_, Rectangle rBound_, std::string sRun,
           std::string sChase, bool bFlip = false);

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void OnMouseDown(Point pPos) {}
};

template <class T>
void Union(std::map<std::string, T> &TarMap,
           const std::map<std::string, T> &srcMap) {
  for (typename std::map<std::string, T>::const_iterator itr = srcMap.begin(),
                                                         etr = srcMap.end();
       itr != etr; ++itr)
    TarMap[itr->first] += itr->second;
}

/** Merge srcMap into TarMap with OR (TarMap[k] |= srcMap[k]). */
inline void Union(std::map<std::string, bool> &TarMap,
                  const std::map<std::string, bool> &srcMap) {
  for (std::map<std::string, bool>::const_iterator itr = srcMap.begin(),
                                                   etr = srcMap.end();
       itr != etr; ++itr)
    TarMap[itr->first] |= itr->second;
}

/** Write map as "key = value; " to ofs. */
template <class T>
std::ostream &Out(std::ostream &ofs, const std::map<std::string, T> &srcMap) {
  for (typename std::map<std::string, T>::const_iterator itr = srcMap.begin(),
                                                         etr = srcMap.end();
       itr != etr; ++itr)
    ofs << itr->first << " = " << itr->second << "; ";
  return ofs;
}

/** Power-up state: named float/unsigned/bool maps, nNum; += merges, Add
 * accumulates. */
struct FireballBonus : virtual public Entity {
  std::map<std::string, float> fMap;
  std::map<std::string, unsigned> uMap;
  std::map<std::string, bool> bMap;

  int nNum;

  FireballBonus(int nNum_, bool bDef);

  void Add(std::string str, float f) { fMap[str] += f; }
  void Add(std::string str, unsigned u) { uMap[str] += u; }
  void Add(std::string str, bool b) { bMap[str] |= b; }

  template <class T>
  FireballBonus(int nNum_, std::string str, T t) : nNum(nNum_) {
    Add(str, t);
  }

  FireballBonus &operator+=(const FireballBonus &f);
};

std::ostream &operator<<(std::ostream &ofs, FireballBonus b);

/** Chain reaction generation count or infinite; Evolve decrements, IsLast when
 * 0. */
struct Chain {
  bool bInfinite;
  unsigned nGeneration;

  Chain(bool bInfinite_ = false) : bInfinite(bInfinite_), nGeneration(0) {}
  Chain(unsigned nGeneration_) : bInfinite(false), nGeneration(nGeneration_) {}

  Chain Evolve();

  bool IsLast() { return (!bInfinite) && (nGeneration == 0); }
};

inline ImageSequence Reset(ImageSequence imgSeq) {
  imgSeq.nActive = 0;
  return imgSeq;
}

/** Expanding explosion; hits ConsumableEntities and spawns child
 * ChainExplosions via ch. */
struct ChainExplosion : virtual public AnimationOnce,
                        virtual public PhysicalEntity {
  float r_in, r;
  float delta;

  Chain ch;

  SSP<BasicController> pBc;

  ChainExplosion(const AnimationOnce &av, float r_, float delta_,
                 SP<BasicController> pBc_, Chain ch_ = Chain())
      : AnimationOnce(av), r(r_), r_in(r_), delta(delta_), pBc(this, pBc_),
        ch(ch_) {}

  /*virtual*/ unsigned GetRadius() { return unsigned(r); }
  /**
   * Each frame the explosion can grow. Remove dead units. For each unit inside
   * the blast (except golems and mega slimes): apply fire damage and optionally
   * spawn a new explosion on them for a chain reaction. Then advance the
   * explosion animation.
   */
  /*virtual*/ void Update();

  void Draw(SP<ScalingDrawer> pDr) { AnimationOnce::Draw(pDr); }
};

/** Map angle a to one of nDiv discrete directions. */
inline unsigned DiscreetAngle(float a, unsigned nDiv) {
  return unsigned((-a / 2 / 3.1415 + 2 - 1.0 / 4 + 1.0 / 2 / nDiv) * nDiv) %
         nDiv;
}

struct AdvancedController;
struct Dragon;

struct KnightOnFire : public Critter //, public ConsumableEntity
{
  SSP<BasicController> pBc;
  unsigned nTimer, nTimer_i;
  Timer t;
  Chain c;

  /** Set fVel to random direction, scaled by fKnightFireSpeed. */
  void RandomizeVelocity();

  KnightOnFire(const Critter &cr, SP<BasicController> pBc_, unsigned nTimer_,
               Chain c_);

  /*virtual*/ void Update();
};

/** Floating "+N" score text at a point; animates then removes. */
struct BonusScore : public EventEntity, public VisualEntity {
  SSP<AdvancedController> pAc;
  std::string sText;
  unsigned nScore;
  unsigned nScoreSoFar;
  Point p;
  Timer t;
  unsigned nC;
  Color c;

  BonusScore(SP<AdvancedController> pAc_, Point p_, unsigned nScore_);

  /*virtual*/ void Update();

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return 5; }

  /*virtual*/ Point GetPosition() { return p; }
};

struct SoundControls : public EventEntity {
  BackgroundMusicPlayer &plr;
  int nTheme;

  SoundControls(BackgroundMusicPlayer &plr_, int nTheme_)
      : plr(plr_), nTheme(nTheme_) {}

  /** SwitchTheme(nTheme) or StopMusic() when nTheme == -1. */
  /*virtual*/ void Update();
};

struct FireballBonusAnimation;
struct Castle;
struct Road;
struct Dragon;
struct KnightGenerator;

/** Tracks mouse position, last down, press state and counter for input. */
struct PositionTracker {
  Point pMouse;
  Point pLastDownPosition;
  bool bPressed;
  int nCounter;

  PositionTracker() : bPressed(false) {}

  /** Set pressed and reset counter. */
  void On();
  /** Clear pressed and return counter. */
  int Off();
  /** Increment nCounter while bPressed. */
  void Update();

  /** Set pMouse (scaled by 2). */
  void UpdateMouse(Point pMouse_);
  /** Set pLastDownPosition (scaled by 2). */
  void UpdateLastDownPosition(Point pMouse_);
  fPoint GetDirection(fPoint fDragonPos) { return pMouse - fDragonPos; }
  fPoint GetFlightDirection(fPoint fDragonPos);
};

struct Slime;
struct Sliminess;
struct MageGenerator;

/** Main game controller: castles, roads, dragon, generators, bonuses, input
 * (trackball/key). */
struct AdvancedController : public BasicController {
  std::vector<ASSP<Castle>> vCs;
  std::vector<ASSP<Road>> vRd;
  std::vector<ASSP<Dragon>> vDr;

  std::list<ASSP<FireballBonusAnimation>> lsBonus;
  std::list<ASSP<Slime>> lsSlimes;
  std::list<ASSP<Sliminess>> lsSliminess;

  TrackballTracker tr;

  Timer t; // time until ghosts

  bool bFirstUpdate;

  bool bGhostTime;

  bool bTimerFlash;
  Timer tBlink;
  bool bBlink;

  Timer tStep;
  bool bLeft;

  bool bCh;

  bool bLeftDown, bRightDown;
  unsigned nLastDir; // 0 neutral, 1 2 3 4 L R D U
  bool bWasDirectionalInput;

  unsigned nLvl;

  int nSlimeNum;

  bool bPaused;

  Timer tLoseTimer;

  KnightGenerator *pGr;
  MageGenerator *pMgGen;
  SP<SoundControls> pSc;

  PositionTracker pt;
  bool bTakeOffToggle;

  TutorialLevelOne tutOne;
  TutorialLevelTwo tutTwo;
  SSP<TutorialTextEntity> pTutorialText;

  AdvancedController(const AdvancedController &a);

  AdvancedController(SP<TwrGlobalController> pGl_, Rectangle rBound, Color c,
                     const LevelLayout &lvl);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void Update();

  /*virtual*/ void OnMouse(Point pPos);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ void OnMouseUp();
  /*virtual*/ void Fire();

  float GetCompletionRate();

  MouseCursor mc;

  void MegaGeneration();
  void MegaGeneration(Point p);
};

/** Draws high score in a rectangle. */
struct HighScoreShower : public VisualEntity {
  SSP<TwrGlobalController> pGl;
  Rectangle rBound;

  HighScoreShower(SP<TwrGlobalController> pGl_, Rectangle rBound_)
      : pGl(this, pGl_), rBound(rBound_) {}

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);
};

struct IntroTextShower : public VisualEntity {
  SSP<TwrGlobalController> pGl;
  Rectangle rBound;

  IntroTextShower(SP<TwrGlobalController> pGl_, Rectangle rBound_)
      : pGl(this, pGl_), rBound(rBound_) {}

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);
};

/** Controller that shows dragon score and exits on click or timer. */
struct DragonScoreController : public BasicController {
  Timer t;
  bool bClickToExit;

  DragonScoreController(SP<TwrGlobalController> pGl_, Rectangle rBound, Color c,
                        bool bScoreShow);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void Update();
  /*virtual*/ void DoubleClick();

  /*virtual*/ void OnMouseDown(Point pPos);
};

int GetFireballRaduis(FireballBonus &fb);
std::string GetSizeSuffix(FireballBonus &fb);
float GetExplosionInitialRaduis(FireballBonus &fb);
float GetExplosionExpansionRate(FireballBonus &fb);

/** Player fireball Critter; bThrough for passthrough, hits ConsumableEntities.
 */
struct Fireball : public Critter {
  SSP<AdvancedController> pBc;
  bool bThrough;
  FireballBonus fb;

  Chain ch;
  unsigned nChain;

  Fireball(const Fireball &f)
      : pBc(this, f.pBc), bThrough(f.bThrough), fb(f.fb), ch(f.ch),
        nChain(f.nChain), Critter(f) {}

  Fireball(Point p, fPoint v, SP<AdvancedController> pBc_, FireballBonus &fb_,
           Chain ch_ = Chain(), unsigned nChain_ = 1);

  /*virtual*/ void Update();
};

/** FireballBonus that updates on a timer (e.g. temporary power-up). */
struct TimedFireballBonus : public FireballBonus, virtual public EventEntity {
  Timer t;

  TimedFireballBonus(const FireballBonus &fb, unsigned nPeriod)
      : FireballBonus(fb), t(nPeriod) {}

  /*virtual*/ void Update();
};

/** Fireball that orbits at fRadius (circular motion). */
struct CircularFireball : virtual public Fireball,
                          virtual public TimedFireballBonus {
  float fRadius;
  fPoint i_pos;
  Timer t;

  CircularFireball(const Fireball &f, float fRadius_, unsigned nPeriod)
      : Fireball(f), TimedFireballBonus(FireballBonus(8, false), nPeriod),
        fRadius(fRadius_), i_pos(f.fPos), t(nPeriod) {}

  /*virtual*/ void Update();
};

struct Castle : public Critter {
  unsigned nPrincesses;
  SSP<AdvancedController> pAv;
  SSP<Dragon> pDrag;

  bool bBroken;

  Castle(Point p, Rectangle rBound_, SP<AdvancedController> pAv_);

  void OnKnight(char cWhat);

  /*unsigned*/ void Draw(SP<ScalingDrawer> pDr);
};

inline Point Center(Size sz) { return Point(sz.x / 2, sz.y / 2); }

struct KnightGenerator;
struct Dragon;

/** Level road segment: vertical/horizontal, coordinate, bounds; Draw renders
 * gray bar. */
struct Road : virtual public VisualEntity {
  bool bVertical;
  unsigned nCoord;
  Rectangle rBound;

  Road(bool bVertical_, unsigned nCoord_, Rectangle rBound_)
      : bVertical(bVertical_), nCoord(nCoord_), rBound(rBound_) {}
  Road(const Road &) = default;
  Road &operator=(const Road &) = default;
  /* Move assignment deleted to avoid -Wvirtual-move-assign (virtual base
   * VisualEntity); copy is used instead (e.g. vector<Road>). */
  Road &operator=(Road &&) = delete;

  /*virtual*/ float GetPriority() { return 0; }
  /*virtual*/ Point GetPosition() { return Point(); }

  /** Draw gray bar along segment (vertical or horizontal). */
  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /** Pick random entry/exit point and direction; write to p and v. */
  void RoadMap(Point &p, Point &v);
};

inline std::ostream &operator<<(std::ostream &ofs, const Road &r) {
  return ofs << r.bVertical << " " << r.nCoord << " ";
}

inline std::istream &operator>>(std::istream &ifs, Road &r) {
  return ifs >> r.bVertical >> r.nCoord;
}

template <class A, class B> void PushBackASSP(SP_Info *pInf, A &arr, B *pnt) {
  arr.push_back(ASSP<B>(pInf, pnt));
}

template <class A, class B>
void PushBackASSP(SP_Info *pInf, A &arr, SP<B> pnt) {
  arr.push_back(ASSP<B>(pInf, pnt));
}

struct FancyRoad : public Road {
  FancyRoad(const Road &rd, SP<AdvancedController> pAd_)
      : Road(rd), pAd(this, pAd_) {}
  SSP<AdvancedController> pAd;

  /** Draw tiled road image along segment. */
  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);
};

/** Princess unit: Critter + ConsumableEntity, captured by dragon. */
struct Princess : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;

  Princess(const Critter &cr, SP<AdvancedController> pAc_)
      : Critter(cr), pAc(this, pAc_) {}

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'P'; }

  /** Spawn BonusScore, set bExist false. */
  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);
};

/** Spawns skeleton knights on a timer at a position. */
struct SkellyGenerator : public EventEntity {
  Timer t;
  Point p;

  SSP<AdvancedController> pAdv;

  SkellyGenerator(Point p_, SP<AdvancedController> pAdv_);

  /*virutal*/ void Update();
};

inline int GetRandTimeFromRate(float fRate) {
  return 1 + int((float(rand()) / RAND_MAX * 1.5 + .25) * fRate);
}

void SummonSkeletons(SP<AdvancedController> pAc, Point p);

unsigned GetTimeUntillSpell() {
  return 8 * nFramesInSecond + rand() % (3 * nFramesInSecond);
}

struct Mage : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;

  bool bAngry;
  bool bCasting;
  Timer tUntilSpell;
  Timer tSpell;
  Timer tSpellAnimate;

  fPoint fMvVel;

  Mage(const Critter &cr, SP<AdvancedController> pAc_, bool bAngry_);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  void SummonSlimes();

  /*virtual*/ char GetType() { return 'M'; }

  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);
};

unsigned RandomBonus(bool bInTower = true);

ImageSequence GetBonusImage(int n, Preloader &pr);

/** Pick-up animation with radius; overlaps ConsumableEntity trigger collection.
 */
struct FireballBonusAnimation : public Animation,
                                virtual public PhysicalEntity {
  unsigned n;
  Timer tm;
  bool bBlink;
  SSP<AdvancedController> pAd;
  std::string sUnderText;
  ImageSequence coronaSeq;

  FireballBonusAnimation(Point p_, unsigned n_, SP<AdvancedController> pAd_);

  /*virtual*/ unsigned GetRadius() { return 20U; }

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /*virtual*/ void Update();
};

/** Trader unit: drops bonus, bFirstBns ref for first-bonus logic. */
struct Trader : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;
  bool &bFirstBns;

  Trader(const Critter &cr, SP<AdvancedController> pAc_, bool &bFirstBns_)
      : Critter(cr), pAc(this, pAc_), bFirstBns(bFirstBns_) {}

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'T'; }

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);
};

/** Knight unit: chases princess/castle, can become ghost (Ghostiness). */
struct Knight : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;

  char cType;

  int nGhostHit;
  int nGolemHealth;

  Knight(const Critter &cr, SP<AdvancedController> pAc_, char cType_)
      : Critter(cr), pAc(this, pAc_), cType(cType_), nGhostHit(1),
        nGolemHealth(nGolemHealthMax) {}

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /** Move position back one unit along current velocity. */
  void KnockBack();

  /**
   * Each frame: if the knight is on a castle, notify the castle and remove the
   * knight. If it is a skeleton: remove dead units and bonuses, then damage or
   * kill princesses and traders on hit, and collect bonus pickups. When the
   * knight moves, advance the walk animation and play step sounds at the right
   * moments.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return cType; }
};

/** Large slime unit: splits or merges (MegaSlime logic). */
struct MegaSlime : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;

  int nHealth;

  MegaSlime(fPoint fPos, Rectangle rBound, SP<AdvancedController> pAc_);

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) { Critter::Draw(pDr); }

  void RandomizeVelocity();

  /**
   * Remove collected bonuses. When the slime touches a bonus pickup, remove it
   * and play a sound. On the timer: advance the animation; on jump and land
   * frames, play sounds and either randomize movement or stop.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'E'; }
};

/** Ghost knight effect: timed visual at a position. */
struct Ghostiness : public EventEntity {
  Timer t;
  Point p;

  SSP<AdvancedController> pAdv;

  Critter knCp;
  int nGhostHit;

  Ghostiness(Point p_, SP<AdvancedController> pAdv_, Critter knCp_,
             int nGhostHit_);

  /*virutal*/ void Update();
};

/** Slime unit: moves toward target, timer for behavior. */
struct Slime : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;
  Timer t;
  int nGeneration;

  Slime(fPoint fPos, Rectangle rBound, SP<AdvancedController> pAc_,
        int nGeneration_);

  int GetGeneration() { return nGeneration; }

  void RandomizeVelocity();

  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'L'; }

  ~Slime();
};

/** Spawns slimes on a timer at a position. */
struct Sliminess : public EventEntity {
  Timer t;
  Point p;

  bool bFast;
  int nGeneration;
  SSP<AdvancedController> pAdv;
  SSP<AnimationOnce> pSlm;

  Sliminess(Point p_, SP<AdvancedController> pAdv_, bool bFast_,
            int nGeneration_);

  /*virutal*/ void Update();

  /** Cancels the spawn (this and the visual both removed). */
  void Kill();

  Point GetPosition() { return p; }

  ~Sliminess();
};

/** Spawns MegaSlimes; holds position and controller. */
struct MegaSliminess : public EventEntity {
  Point p;
  SSP<AdvancedController> pAdv;
  SSP<AnimationOnce> pSlm;

  MegaSliminess(Point p_, SP<AdvancedController> pAdv_);

  /*virutal*/ void Update();
};

/** SimpleVisualEntity that moves with fPos/fVel (e.g. menu slime). */
struct FloatingSlime : public SimpleVisualEntity {
  fPoint fPos;
  fPoint fVel;

  Timer tTermination;

  FloatingSlime(ImageSequence seq, Point pStart, Point pEnd, unsigned nTime);

  /*virtual*/ Point GetPosition() { return fPos.ToPnt(); }
  /*virtual*/ void Update();
};

/** Exception for BrokenLine segment errors (e.g. invalid/empty segment). */
class SegmentSimpleException : public MyException {
  std::string strProblem;

public:
  SegmentSimpleException(crefString strFnName_, crefString strProblem_)
      : MyException("SegmentSimpleException", "BrokenLine", strFnName_),
        strProblem(strProblem_) {}

  /*virtual*/ std::string GetErrorMessage() const { return strProblem; }
};

/** Polyline(s): vEdges is list of point sequences; CloseLast, Add, AddLine,
 * Join, stream I/O. */
struct BrokenLine {
  typedef std::vector<fPoint> VecLine;
  typedef std::vector<VecLine> VecLines;

  VecLines vEdges;

  /** Close last segment by appending first point; throws if empty. */
  void CloseLast();

  /** Append p to last segment; push new segment if empty. */
  void Add(fPoint p);

  void AddLine(const VecLine &vl) { vEdges.push_back(vl); }

  /** Append all segments of b to vEdges. */
  void Join(const BrokenLine &b);

  BrokenLine() {}
  BrokenLine(fPoint p1) { Add(p1); }
  BrokenLine(fPoint p1, fPoint p2);
  BrokenLine(fPoint p1, fPoint p2, fPoint p3);
  BrokenLine(fPoint p1, fPoint p2, fPoint p3, fPoint p4);
  BrokenLine(Rectangle r);

  /**
   * Picks a random spot along the path, with longer segments more likely to be
   * chosen. If the path has no length, picks from a random segment instead.
   * Fails if the path is empty.
   */
  fPoint RandomByLength();

  /** Random point on a random segment. */
  fPoint RandomBySegment();
};

std::ostream &operator<<(std::ostream &ofs, const BrokenLine &bl);
std::istream &operator>>(std::istream &ifs, BrokenLine &bl);

/** One level: bounds, knight spawn line, castle positions, roads, timer, spawn
 * freqs; Convert scales coords. */
struct LevelLayout {
  Rectangle sBound;

  unsigned nLvl;

  LevelLayout(Rectangle sBound_) : sBound(sBound_) {}

  BrokenLine blKnightGen;
  std::vector<Point> vCastleLoc;
  std::vector<Road> vRoadGen;
  unsigned nTimer;

  std::vector<float> vFreq;

  /** Scales the level's spawn path, castle positions, and roads to the given
   * grid size. */
  void Convert(int n = 24);
};

std::ostream &operator<<(std::ostream &ofs, const LevelLayout &f);
std::istream &operator>>(std::istream &ifs, LevelLayout &f);

/** Spawns knights along a path on a timer. */
struct KnightGenerator : virtual public EventEntity {
  bool bFirst;

  float dRate;
  Rectangle rBound;
  SSP<AdvancedController> pBc;
  ImageSequence seq;

  Timer tm;

  BrokenLine bl;

  /** Returns the current spawn rate (depends on completion and ghost mode). */
  float GetRate();

  KnightGenerator(float dRate_, Rectangle rBound_, SP<AdvancedController> pBc_,
                  const BrokenLine &bl_);

  /** Spawn one Knight (or Golem/ghost) on bl; add to pBc. */
  void Generate(bool bGolem = false);

  /*virtual*/ void Update();
};

/** Spawns princesses at a rate within bounds. */
struct PrincessGenerator : virtual public EventEntity {
  float dRate;
  Rectangle rBound;
  SSP<AdvancedController> pBc;
  Timer tm;
  bool bFirst;

  PrincessGenerator(float dRate_, Rectangle rBound_,
                    SP<AdvancedController> pBc_);

  /**
   * When the timer fires: pick a random road and spawn a princess there, moving
   * toward the castles. The first princess gets a "capture" hint. Add her to
   * the game, play the arrival sound, and notify the tutorial.
   */
  /*virtual*/ void Update();
};

struct MageGenerator : virtual public EventEntity {
  float dRate;
  Rectangle rBound;
  SSP<AdvancedController> pBc;
  Timer tm;

  MageGenerator(float dRate_, float dAngryRate_, Rectangle rBound_,
                SP<AdvancedController> pBc_);

  /*virtual*/ void Update();

  /** Spawn one Mage on a random road; add to pBc. */
  void MageGenerate();
};

/** Spawns traders at a rate within bounds. */
struct TraderGenerator : virtual public EventEntity {
  float dRate;
  Rectangle rBound;
  SSP<AdvancedController> pBc;
  Timer tm;
  bool bFirst;
  bool bFirstBns;

  float GetRate();

  TraderGenerator(float dRate_, Rectangle rBound_, SP<AdvancedController> pBc_);

  /*virtual*/ void Update();
};

struct DragonLeash {
  fPoint lastVel;
  // tilt in radians per frame
  float tilt;
  float speed; // in pixels/frame
  float trackballScaleFactor;
  float naturalScaleFactor;
  float maxTilt;

  DragonLeash() : lastVel(0, -1) {
    tilt = 0;
    speed = fDragonSpeed;
    trackballScaleFactor = float(1) / 100;
    naturalScaleFactor = .35F;
    maxTilt = .40F / (float(nFramesInSecond) / 10);
  }

  /** Smoothly adjusts steering tilt from trackball input and clamps to max
   * turn rate. */
  void ModifyTilt(Point trackball);

  /** Applies trackball to tilt and returns the new flying direction. */
  fPoint GetNewVelocity(Point trackball);
};

/** Set of key/button codes for input (e.g. fire, steer). */
struct ButtonSet {
  std::vector<int> vCodes;

  ButtonSet(int q, int w, int e, int d, int c, int x, int z, int a, int sp);

  bool IsSpace(int nCode) { return nCode == vCodes[8]; }

  /** Converts a key code to an 8-direction offset; (0,0) if no match. */
  Point GetPoint(int nCode);
};

unsigned GetRandNum(unsigned nRange);
unsigned GetRandFromDistribution(std::vector<float> vProb);
unsigned GetFireballChainNum(FireballBonus &fb);

/** Player dragon: carries bonuses and fireballs, steer/shoot, collision with
 * units. */
struct Dragon : public Critter {
  std::list<ASSP<TimedFireballBonus>> lsBonuses;
  std::list<ASSP<Fireball>> lsBalls;

  DragonLeash leash;

  /**
   * Creates a timed power-up of the given type. Higher levels shorten how long
   * it lasts. Types include: fireball regen, multi-shot, laser, bigger shot,
   * more ammo, explosion, chain, set-on-fire, ring (fires in a circle), nuke
   * (hits many enemies at once), speed, and fire rate.
   */
  SP<TimedFireballBonus> GetBonus(unsigned n, unsigned nTime);

  /** Saves all current power-ups so they can be restored on the next level. */
  void FlushBonuses();

  /** Restores power-ups that were saved when entering this level. */
  void RecoverBonuses();

  /** Merge all active lsBonuses into one FireballBonus; add nExtraFireballs to
   * total. CleanUp lsBonuses first. */
  FireballBonus GetAllBonuses();

  bool bFly;

  bool bCarry;
  char cCarry;
  int nPrCr;

  int nExtraFireballs;

  unsigned nTimer;
  Index imgCarry;

  bool bTookOff;

  int nFireballCount;
  Timer tFireballRegen;
  bool bRegenLocked;
  Timer tRegenUnlock;

  SSP<AdvancedController> pAd;

  SSP<Castle> pCs;

  ImageSequence imgStable;
  ImageSequence imgFly;

  ButtonSet bt;

  /**
   * Sets up the dragon: how many fireballs it has (from last level if any). If
   * a castle is given and free, the dragon starts perched there; otherwise
   * starts in the air at the first castle. Uses perched sprite and clears any
   * saved bonuses from the previous level.
   */
  Dragon(SP<Castle> pCs_, SP<AdvancedController> pAd_, ImageSequence imgStable_,
         ImageSequence imgFly_, ButtonSet bt_);

  /**
   * Each frame: when allowed, slowly refill fireballs toward the max. While
   * flying: if over a castle, land (or clear "just took off"). While flying and
   * able to carry: try to pick up a princess. While flying: collect any bonus
   * pickups. Then run normal movement.
   */
  /*vrtual*/ void Update();

  /** When perched on a castle, returns a position slightly above the feet;
   * otherwise the real position. */
  /*vrtual*/ Point GetPosition();

  /** Draws the carried unit if any. When perched, draws the dragon at the
   * castle (two frames for animation). When flying, draws the flying dragon
   * facing the right way. Also ticks down a short timer used for effects. */
  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /** Add pBonus to lsBonuses and AddE; play powerup sound unless bSilent. */
  void AddBonus(SP<TimedFireballBonus> pBonus, bool bSilent = false);

  /**
   * Shoots one or more fireballs in the given direction. Does nothing if out of
   * ammo or direction is zero. Uses current power-ups for speed and number of
   * shots. When on the tower, notifies the tutorial. Temporarily stops fireball
   * refill. Plays laser or normal shoot sound.
   */
  void Fire(fPoint fDir);

  /**
   * Switches between perched and flying. Taking off: play sound, switch to
   * flying sprite, set speed from input (or straight up if none). Landing:
   * find a castle underneath; if carrying a princess, deliver her (and maybe
   * win the level); if carrying a trader, get a random power-up; otherwise
   * just play landing sound. If not over a castle but over a trader, pick him
   * up. Clears what is being carried when landing.
   */
  void Toggle();
};

/** BasicController that advances (Next) when only background is left or on
 * input. */
struct AlmostBasicController : public BasicController {
  AlmostBasicController(const BasicController &b) : BasicController(b) {}

  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos) { pGl->Next(); }
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
