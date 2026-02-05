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

// const int nSlimeTotalGenerations = 4;

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
  Polar(fPoint p) : r(p.Length()) {
    if (p.y == 0 && p.x == 0)
      a = 0;
    else
      a = atan2(p.y, p.x);
  }
  Polar operator*(Polar p) { return Polar(a + p.a, r * p.r); }

  fPoint TofPoint() { return fPoint(r * cos(a), r * sin(a)); }
};

/** Combine two direction codes (1–4: left/right/down/up) into a normalized
 * fPoint. */
inline fPoint ComposeDirection(int dir1, int dir2) {
  fPoint r(0, 0);
  switch (dir1) {
  case 1:
    r += fPoint(-1, 0);
    break;
  case 2:
    r += fPoint(1, 0);
    break;
  case 3:
    r += fPoint(0, 1);
    break;
  case 4:
    r += fPoint(0, -1);
    break;
  default:
    break;
  }
  switch (dir2) {
  case 1:
    r += fPoint(-1, 0);
    break;
  case 2:
    r += fPoint(1, 0);
    break;
  case 3:
    r += fPoint(0, 1);
    break;
  case 4:
    r += fPoint(0, -1);
    break;
  default:
    break;
  }
  r.Normalize();
  return r;
}

/** Angle within a wedge: fDir = center, dWidth = half-width, nWhich/nHowMany =
 * segment. */
inline fPoint GetWedgeAngle(fPoint fDir, float dWidth, unsigned nWhich,
                            unsigned nHowMany) {
  if (nHowMany == 1)
    return fDir;

  float d = 3.1415F * 2 * dWidth / (nHowMany - 1) * nWhich;

  return (Polar(fDir) * Polar(d - 3.1415F * dWidth, 1)).TofPoint();
}

/** Random direction near fDir within fRange * 2π. */
inline fPoint RandomAngle(fPoint fDir = fPoint(1, 0), float fRange = 1.F) {
  return (Polar(fDir) *
          Polar((float(rand()) / RAND_MAX - .5F) * fRange * 2 * 3.1415F, 1))
      .TofPoint();
}

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

  void Scale(Index &pImg, int nFactor_ = -1) {
    if (nFactor_ < 0)
      nFactor_ = nFactor;

    Image *pOrig = pGr->GetImage(pImg);
    Index pRet = pGr->GetBlankImage(
        Size(pOrig->GetSize().x * nFactor_, pOrig->GetSize().y * nFactor_));
    Image *pFin = pGr->GetImage(pRet);

    Point p, s;
    for (p.y = 0; p.y < pOrig->GetSize().y; ++p.y)
      for (p.x = 0; p.x < pOrig->GetSize().x; ++p.x)
        for (s.y = 0; s.y < nFactor_; ++s.y)
          for (s.x = 0; s.x < nFactor_; ++s.x)
            pFin->SetPixel(Point(p.x * nFactor_ + s.x, p.y * nFactor_ + s.y),
                           pOrig->GetPixel(p));

    pImg = pRet;
  }

  /*virtual*/ void Draw(Index nImg, Point p, bool bCentered = true) {
    p.x *= nFactor;
    p.y *= nFactor;
    if (bCentered) {
      Size sz = pGr->GetImage(nImg)->GetSize();
      p.x -= sz.x / 2;
      p.y -= sz.y / 2;
    }

    pGr->DrawImage(p, nImg, false);
  }

  /*virtual*/ Index LoadImage(std::string strFile) {
    Index n = pGr->LoadImage(strFile);
    pGr->GetImage(n)->ChangeColor(Color(0, 0, 0), Color(0, 0, 0, 0));
    return n;
  }
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

  void CacheColor(Color c) {
    std::vector<Index> vNewColors;

    for (size_t i = 0, sz = vImg.size(); i < sz; ++i) {
      Index vColImg = pDr->pGr->CopyImage(vImg[i]);
      pDr->pGr->GetImage(vColImg)->ChangeColor(Color(255, 255, 0), c);
      vNewColors.push_back(vColImg);
    }

    mpCachedRecolorings[c] = vNewColors;
  }

  NumberDrawer(SP<ScalingDrawer> pDr_, std::string sFontPath,
               std::string sFontName)
      : pDr(this, pDr_), vImgIndx(256, -1) {
    std::ifstream ifs((sFontPath + sFontName + ".txt").c_str());

    unsigned n;
    unsigned char c;
    for (n = 0; c = ifs.get(), !ifs.fail(); ++n)
      vImgIndx[c] = n;

    Index nImg = pDr->LoadImage((sFontPath + sFontName + ".bmp").c_str());
    Image *pImg = pDr->pGr->GetImage(nImg);
    for (unsigned i = 0; i < n; ++i) {
      Index nCurr = pDr->pGr->GetBlankImage(Size(3, 5));
      Image *pCurr = pDr->pGr->GetImage(nCurr);

      Point p;
      for (p.y = 0; p.y < 5; ++p.y)
        for (p.x = 0; p.x < 3; ++p.x)
          pCurr->SetPixel(p, pImg->GetPixel(Point(i * 4 + p.x, p.y)));
      pDr->Scale(nCurr);
      vImg.push_back(nCurr);
    }

    pDr->pGr->DeleteImage(nImg);
  }

  /*
  void DrawNumber(unsigned n, Point p, unsigned nDigits = 0)
{
  std::vector<unsigned> vDigits;
  if(n == 0)
      vDigits.push_back(0);
  while(n != 0)
  {
      vDigits.push_back(n%10);
      n /= 10;
  }

  unsigned i, sz = unsigned(vDigits.size());
  for(i = 0; int(i) < int(nDigits) - int(sz); ++i)
      vDigits.push_back(0);

  for(i = 0; i < vDigits.size(); ++i)
  {
      pDr->Draw(vImg[ vImgIndx['0' + vDigits[vDigits.size() - i - 1]] ],
Point(p.x + 4 * i, p.y), false);
  }
}
  */

  std::string GetNumber(unsigned n, unsigned nDigits = 0) {
    std::string s;
    if (n == 0)
      s += '0';
    while (n != 0) {
      s += ('0' + n % 10);
      n /= 10;
    }

    unsigned i, sz = unsigned(s.size());
    for (i = 0; int(i) < int(nDigits) - int(sz); ++i)
      s += '0';

    std::reverse(s.begin(), s.end());

    return s;
  }

  void DrawNumber(unsigned n, Point p, unsigned nDigits = 0) {
    DrawWord(GetNumber(n, nDigits), p);
  }

  void DrawColorNumber(unsigned n, Point p, Color c, unsigned nDigits = 0) {
    DrawColorWord(GetNumber(n, nDigits), p, c);
  }

  void DrawWord(std::string s, Point p, bool bCenter = false) {
    if (bCenter) {
      p.x -= 2 * s.length();
      p.y -= 2;
    }

    for (unsigned i = 0; i < s.length(); ++i) {
      int n = int(s[i]);

      if (vImgIndx[n] == -1)
        continue;

      pDr->Draw(vImg[vImgIndx[n]], Point(p.x + 4 * i, p.y), false);
    }
  }

  void DrawColorWord(std::string s, Point p, Color c, bool bCenter = false) {
    if (bCenter) {
      p.x -= 2 * s.length();
      p.y -= 2;
    }

    std::vector<Index> *pImageVector;

    bool bManualRecolor = true;

    std::map<Color, std::vector<Index>>::iterator itr =
        mpCachedRecolorings.find(c);

    if (itr != mpCachedRecolorings.end()) {
      bManualRecolor = false;
      pImageVector = &(itr->second);
    }

    for (unsigned i = 0; i < s.length(); ++i) {
      int n = int(s[i]);

      if (vImgIndx[n] == -1)
        continue;

      if (bManualRecolor) {
        Index vColImg = pDr->pGr->CopyImage(vImg[vImgIndx[n]]);
        pDr->pGr->GetImage(vColImg)->ChangeColor(Color(255, 255, 0), c);
        pDr->Draw(vColImg, Point(p.x + 4 * i, p.y), false);
      } else {
        pDr->Draw((*pImageVector)[vImgIndx[n]], Point(p.x + 4 * i, p.y), false);
      }
    }
  }
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

  void ToggleOff() {
    if (!bOff) {
      bOff = true;
      StopMusic();
    } else {
      bOff = false;
    }
  }
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

  void PlaySound(Index i, int nChannel = -1, bool bLoop = false) {
    if (bSoundOn)
      pSndRaw->PlaySound(i, nChannel, bLoop);
  }

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
  // void DrawCursor();
  // void fPos(Point pPos);
};

/** Controller that draws a single full-screen image and advances on key. */
struct SimpleController : public GameController {
  Index nImage;

  SimpleController(SP<TwrGlobalController> pGraph, std::string strFileName)
      : GameController(pGraph) {
    nImage = pGl->pGraph->LoadImage(strFileName);
  }

  ~SimpleController() { pGl->pGraph->DeleteImage(nImage); }

  /*virtual*/ void Update() {
    pGl->pGraph->DrawImage(Gui::Point(0, 0), nImage);
  }

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp) {
    if (bUp)
      return;

    pGl->Next();
  }
};

/** Controller that draws background + text that flashes every second. */
struct FlashingController : public GameController {
  Index nImage, nText;
  unsigned nTimer;
  bool bShow;

  FlashingController(SP<TwrGlobalController> pGraph, std::string strFileName,
                     std::string strTextName)
      : GameController(pGraph), nTimer(0), bShow(true) {
    nImage = pGl->pGraph->LoadImage(strFileName);
    nText = pGl->pGraph->LoadImage(strTextName);

    pGl->pGraph->GetImage(nText)->ChangeColor(Color(255, 255, 255),
                                              Color(0, 0, 0, 0));
  }

  ~FlashingController() {
    pGl->pGraph->DeleteImage(nImage);
    pGl->pGraph->DeleteImage(nText);
  }

  /*virtual*/ void Update() {
    ++nTimer;
    if (nTimer % nFramesInSecond == 0)
      bShow = !bShow;

    pGl->pGraph->DrawImage(Point(0, 0), nImage, false);
    if (bShow)
      pGl->pGraph->DrawImage(Point(0, 0), nText, false);
    pGl->pGraph->RefreshAll();
  }

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp) {
    if (bUp)
      return;

    pGl->Next();
  }
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
std::vector<std::string> BreakUpString(std::string s) {
  s += '\n';

  std::vector<std::string> vRet;
  std::string sCurr;

  for (unsigned i = 0; i < s.size(); ++i) {
    if (s[i] == '\n') {
      vRet.push_back(sCurr);
      sCurr = "";
    } else
      sCurr += s[i];
  }

  return vRet;
}

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

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    Point p = pos;
    for (unsigned i = 0; i < vText.size(); ++i) {
      pNum->DrawWord(vText[i], p, bCenter);
      p.y += 7;
    }
  }

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

  void SetText(std::vector<std::string> v) {
    if (sText == v && nDelta == 1)
      return;

    if (sNewText == v && nDelta == -1)
      return;

    if (sText.empty()) {
      nOffset = 0;
      nDelta = 1;
      sText = v;
    } else {
      sNewText = v;
      nDelta = -1;
    }
  }

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    if (pIsTutorialOn && (*pIsTutorialOn == false))
      return;

    for (unsigned i = 0; i < sText.size(); ++i) {
      Point p = pos;
      p.y -= nOffset;
      p.y += nTextVerticalSpacing * i + nTextVerticalSpacing / 3;
      pNum->DrawWord(sText[i], p, true);
    }
  }

  /*virtual*/ float GetPriority() { return dPriority; }

  /*virtual*/ void Update() {
    if (t.Tick()) {
      nOffset += nDelta;
      int nMax = nTextVerticalSpacing * sText.size();

      if (nOffset >= nMax)
        nOffset = nMax;
      if (nOffset < 0) {
        nOffset = 0;
        nDelta = 1;
        sText = sNewText;
        sNewText.clear();
      }
    }
  }
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

  std::vector<std::string> GetText() {
    std::vector<std::string> sText;

    if (bFlying) {
      if (!bKilledKnight || !bPrincessGenerated) {
        sText.push_back(sSteerMessage);
        sText.push_back("fly back to your tower");

        return sText;
      } else {
        sText.push_back(sSteerMessage);
        sText.push_back("fly over the princess to pick her up");
        sText.push_back("bring captured princess to the tower");
#ifndef JOYSTICK_TUTORIAL
        sText.push_back("(you can shoot while flying!)");
#endif

        return sText;
      }
    }

    if (!bKilledKnight) {
      sText.push_back(sShootingMessage);
      sText.push_back("aim for the knights!");
      sText.push_back("don't let them get to the tower");

      return sText;
    }

    if (bPrincessGenerated) {
      if (!bPrincessCaptured) {
        sText.push_back("princess in sight!");
        sText.push_back(sTakeOffMessage);

        return sText;
      }

      sText.push_back("capture four princesses to beat the level");
      sText.push_back("don't let knights get to the tower!");

      return sText;
    }

    return sText;
  };

  void Update() {
    if (pTexter != 0)
      pTexter->SetText(GetText());
  }

  void ShotFired() {}

  void KnightKilled() {
    if (bKilledKnight == false) {
      bKilledKnight = true;
      Update();
    }
  }

  void FlyOn() {
    bFlying = true;
    Update();
  }

  void FlyOff() {
    bFlying = false;
    Update();
  };

  void PrincessGenerate() {
    bPrincessGenerated = true;
    Update();
  };

  void PrincessCaptured() {
    bPrincessCaptured = true;
    Update();
  }
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

  std::vector<std::string> GetText() {
    std::vector<std::string> sText;

    if (!bTraderGenerated)
      return sText;

    if (!bTraderKilled) {
      sText.push_back("trader in sight!");
      sText.push_back("kill a trader to get a power up");

      return sText;
    }

    if (!bBonusPickedUp) {
      sText.push_back("traders drop power ups");
      sText.push_back("fly over to pick them up");
      sText.push_back("collect as many as you can!");

      return sText;
    }

    return sText;
  };

  void Update() {
    if (pTexter != 0)
      pTexter->SetText(GetText());
  }

  void TraderKilled() {
    if (bTraderKilled == false) {
      bTraderKilled = true;
      Update();
    }
  }

  void TraderGenerate() {
    bTraderGenerated = true;
    Update();
  };

  void BonusPickUp() {
    bBonusPickedUp = true;
    Update();
  }
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

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    pDr->Draw(seq.GetImage(), GetPosition(), bCenter);
  }

  /*virtual*/ float GetPriority() { return dPriority; }

  bool bImageToggle;

  /*virtual*/ void Update() {
    if (bTimer) {
      if (t.Tick()) {
        seq.Toggle();
        t = Timer(nPeriod * seq.GetTime());
      }
    } else if (bStep) {
      Point p = GetPosition();
      if (p != pPrev) {
        bImageToggle = !bImageToggle;

        if (bImageToggle) // too smooth if every pixel
          seq.Toggle();
      }
      pPrev = p;
    }
  }
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

  /*virtual*/ void Update() {
    if (t.Tick()) {
      pSnd->PlaySound(seq.GetSound());

      if (seq.nActive == seq.vSounds.size() - 1) {
        bExist = false;
        return;
      }

      seq.Toggle();
      t = Timer(nPeriod * seq.GetTime());
    }
  }
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
  /*virtual*/ void Update() {
    SimpleVisualEntity::Update();

    if (SimpleVisualEntity::seq.nActive == 0) {
      if (!bOnce)
        bExist = false;
    } else {
      bOnce = false;
    }
  }
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

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    pDr->Draw(img, GetPosition(), bCentered);
  }

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

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    pDr->pGr->DrawRectangle(r, c, false);
  }

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
std::string OnOffString(bool b) {
  if (b)
    return "on";
  else
    return "off";
}

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

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    Point p = pLeftTop;
    for (unsigned i = 0; i < pCurr->vEntries.size(); ++i) {
      if (!pCurr->vEntries[i].bDisabled)
        pNum->DrawWord(pCurr->vEntries[i].sText, p, false);
      else
        pNum->DrawColorWord(pCurr->vEntries[i].sText, p, Color(125, 125, 125),
                            false);

      if (pCurr->nMenuPosition == i)
        pMenuCaret->pos = p + Point(-11, pCurr->vEntries[i].szSize.y / 4);

      p.y += pCurr->vEntries[i].szSize.y;
    }
  }

  /*virtual*/ float GetPriority() { return 0; }

  /*virtual*/ void Update() { pCurr->vEntries.at(0).bDisabled = false; }

  void OnMouseMove(Point pMouse) {
    pMouse.x /= 4;
    pMouse.y /= 4;

    Point p = pLeftTop;
    for (unsigned i = 0; i < pCurr->vEntries.size(); ++i) {
      if (!pCurr->vEntries[i].bDisabled &&
          InsideRectangle(Rectangle(p, pCurr->vEntries[i].szSize), pMouse)) {
        pCurr->nMenuPosition = i;
        return;
      }
      p.y += pCurr->vEntries[i].szSize.y;
    }
  }

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

  /*virtual*/ void Update() {
    ++nCount;
    if (nCount % nFramesInSecond == 0)
      --nTime;
    if (nTime == 0)
      bExist = false;
  }

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    pNum->DrawNumber(nTime, Point(22, 2));
  }

  /*virtual*/ Point GetPosition() { return Point(0, 0); }
};

/** ScreenEntity with radius for hit detection (HitDetection). */
struct PhysicalEntity : virtual public ScreenEntity {
  virtual unsigned GetRadius() { return 0; }

  bool HitDetection(SP<PhysicalEntity> pPh) {
    Point d = GetPosition() - pPh->GetPosition();
    unsigned r1 = GetRadius(), r2 = pPh->GetRadius();
    return unsigned(d.x * d.x + d.y * d.y) < (r1 * r1 + r2 * r2);
  }
};

// extern std::ofstream ofs_move;
// extern std::ofstream ofs_angl;

/** Tracks mouse for trackball-style steering (angle and fire). */
struct TrackballTracker {
  MouseTracker mtr;

  std::list<Point> lsMouse;
  int nMaxLength;
  bool trigFlag;
  int threshold;
  TrackballTracker() {
    nMaxLength = 3;
    for (int i = 0; i < nMaxLength; i++)
      lsMouse.push_back(Point(0, 0));
    trigFlag = false;
    threshold = 25;
  }

  void Update() {
    Point p = mtr.GetRelMovement();

    // if(p != Point())
    //    pWr->Write("My " + S(p.x) + " " + S(p.y) + "\n");

    /*
    pWr->Write( S(int(fPoint(p).Length())) + "\t" );



    int i = int(fPoint(p).Length());
    if(i == 0)
    {
        ofs_move << "\n";
        ofs_angl << "\n";
        pWr->Write("\n");
    }
    else
    {
        ofs_move << i << "\t";
        ofs_angl << int(atan2(double(-p.y), p.x) * 180 / 3.1415) << "\t";
        pWr->Write(S(int(atan2(double(-p.y), p.x) * 180 / 3.1415)) + "\n");
    }
    */

    lsMouse.push_front(p);
    lsMouse.pop_back();
  }

  bool IsTrigger() {
    /*
int p = GetDerivative();
    if(!trigFlag && p >= threshold*threshold)
    {
            trigFlag = true;
            return true;
    }
    else if(trigFlag && p < threshold*threshold)
    {
            trigFlag = false;
    }

    return false;
*/

    std::vector<int> v;
    for (std::list<Point>::iterator itr = lsMouse.begin(); v.size() < 3; ++itr)
      v.push_back(int(fPoint(*itr).Length()));

    return (v[1] > v[0]) && (v[1] > v[2]);
  }

  Point GetMovement() {
    /*
    Point p1 = *(lsMouse.begin());
    Point p2 = *(++lsMouse.begin());

    int l1 = fPoint(p1).Length();
    int l2 = fPoint(p2).Length();

    return l1 > l2 ? p1 : p2;
    */

    return *(++lsMouse.begin());
  }

  fPoint GetAvMovement() {
    fPoint p;
    for (std::list<Point>::iterator itr = lsMouse.begin(), etr = lsMouse.end();
         itr != etr; ++itr)
      p += *itr;

    p.x /= lsMouse.size();
    p.y /= lsMouse.size();

    return p;
  }

  int GetLengthSq(Point p) { return p.x * p.x + p.y * p.y; }

  int GetDerivative() {
    if (nMaxLength > 1)
      return GetLengthSq(lsMouse.front()) - GetLengthSq(*++lsMouse.begin());
    else
      return 0;
  }
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
  /*virtual*/ void Move() {
    fPos += fVel;
    if (!InsideRectangle(rBound, fPos.ToPnt())) {
      if (bDieOnExit)
        bExist = false;
      else {
        if (InsideRectangle(rBound, (fPos - fPoint(0, fVel.y)).ToPnt()))
          fPos.y -= fVel.y;
        else if (InsideRectangle(rBound, (fPos - fPoint(fVel.x, 0)).ToPnt()))
          fPos.x -= fVel.x;
        else
          fPos -= fVel;
      }
    }
  }

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
  /*virtual*/ void Move() {
    if (tm.Tick()) {
      fPos += fVel;
      seq.Toggle();
    }

    if (!InsideRectangle(rBound, fPos.ToPnt())) {
      if (bDieOnExit)
        bExist = false;
      else {
        if (InsideRectangle(rBound, (fPos - fPoint(0, fVel.y)).ToPnt()))
          fPos.y -= fVel.y;
        else if (InsideRectangle(rBound, (fPos - fPoint(fVel.x, 0)).ToPnt()))
          fPos.x -= fVel.x;
        else
          fPos -= fVel;
      }
    }
  }

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

  void AddV(SP<VisualEntity> pVs) {
    lsDraw.push_back(ASSP<VisualEntity>(this, pVs));
  }
  void AddE(SP<EventEntity> pEv) {
    lsUpdate.push_back(ASSP<EventEntity>(this, pEv));
  }

  template <class T> void AddBoth(T &t) {
    lsDraw.push_back(ASSP<VisualEntity>(this, t));
    lsUpdate.push_back(ASSP<EventEntity>(this, t));
  }

  void AddBackground(Color c) {
    // Index nBckImg = pGl->pDr->pGr->GetBlankImage(rBound.sz);
    // pGl->pDr->pGr->RectangleOnto(nBckImg, rBound.sz, c);
    // pGl->pDr->Scale(nBckImg);

    Rectangle r = rBound.sz;
    r.sz.x *= pGl->pDr->nFactor;
    r.sz.y *= pGl->pDr->nFactor;

    SP<StaticRectangle> pBkg = new StaticRectangle(r, c, -1.F);

    AddV(pBkg);
  }

  BasicController(const BasicController &b)
      : GameController(b), bNoRefresh(b.bNoRefresh) {
    CopyArrayASSP(this, b.lsDraw, lsDraw);
    CopyArrayASSP(this, b.lsUpdate, lsUpdate);
    CopyArrayASSP(this, b.lsPpl, lsPpl);
  }

  bool bNoRefresh;

  BasicController(SP<TwrGlobalController> pGl_, Rectangle rBound, Color c)
      : GameController(pGl_, rBound), bNoRefresh(false) {
    AddBackground(c);
  }

  /*virtual*/ void Update() {
    CleanUp(lsUpdate);
    CleanUp(lsDraw);
    CleanUp(lsPpl);

    std::list<ASSP<EventEntity>>::iterator itr;
    for (itr = lsUpdate.begin(); itr != lsUpdate.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;
      (*itr)->Move();
    }

    for (itr = lsUpdate.begin(); itr != lsUpdate.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      (*itr)->Update();
    }

    {
      std::list<ASSP<VisualEntity>>::iterator itr;

      std::multimap<ScreenPos, SP<VisualEntity>> mmp;

      for (itr = lsDraw.begin(); itr != lsDraw.end(); ++itr) {
        if (!(*itr)->bExist)
          continue;

        mmp.insert(std::pair<ScreenPos, SP<VisualEntity>>(
            ScreenPos((*itr)->GetPriority(), ((*itr)->GetPosition())), *itr));
      }

      for (std::multimap<ScreenPos, SP<VisualEntity>>::iterator
               mitr = mmp.begin(),
               metr = mmp.end();
           mitr != metr; ++mitr)
        mitr->second->Draw(pGl->pDr);
    }

    if (!bNoRefresh)
      pGl->pGraph->RefreshAll();
  }

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp) {
    if (bUp)
      return;

    pGl->Next();
  }

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
                 int nResumePosition_)
      : BasicController(pGl_, rBound, c), nResumePosition(nResumePosition_),
        pMenuDisplay(this, 0),
        mc(pGl->pr("claw"), Point(), pGl.GetRawPointer()), pHintText(this, 0),
        pOptionText(this, 0) {
    bNoRefresh = true;
  }

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void OnMouse(Point pPos);
  /*virtual*/ void OnMouseDown(Point pPos);

  /*virtual*/ void Update();
};

struct StartScreenController : public BasicController {
  StartScreenController(SP<TwrGlobalController> pGl_, Rectangle rBound, Color c)
      : BasicController(pGl_, rBound, c) {}

  void Next() {
    pGl->Next();
    pGl->pSnd->PlaySound(pGl->pr.GetSnd("start_game"));
  }

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp) {
    if (bUp)
      return;

    if (c == GUI_ESCAPE)
      pGl->Menu();
    else
      Next();
  }

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

  BuyNowController(SP<TwrGlobalController> pGl_, Rectangle rBound, Color c)
      : BasicController(pGl_, rBound, c), t(120), nSlimeCount(50),
        tVel(nFramesInSecond / 2) {
    for (int i = 0; i < nSlimeCount; i++) {
      mSlimes.push_back(
          new Animation(0, pGl->pr("slime"), nFramesInSecond / 10,
                        Point(rBound.sz.x / 2, rBound.sz.y / 2 + 25), true));
      mSlimeVel.push_back(fPoint());
      mSlimePos.push_back(mSlimes.back()->pos);
    }

    bNoRefresh = true;
  }

  void RandomizeVelocity(fPoint &fVel, fPoint pPos) {
    fVel = RandomAngle();

    if (rand() % 7 == 0)
      fVel = fPoint(rBound.sz.x / 2, rBound.sz.y / 2) - fPoint(pPos);

    fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fSlimeSpeed);
  }

  void DrawSlimes() {
    for (unsigned i = 0; i < mSlimes.size(); i++)
      mSlimes[i]->Draw(pGl->pDr);
  };

  /*virtual*/ void Update() {
    BasicController::Update();

    if (tVel.Tick()) {
      for (unsigned i = 0; i < mSlimes.size(); i++)
        if (float(rand()) / RAND_MAX < .25)
          RandomizeVelocity(mSlimeVel[i], mSlimePos[i]);
    }

    for (unsigned i = 0; i < mSlimes.size(); i++) {
      mSlimes[i]->Update();
      // if(rand()%2 == 0)
      //	mSlimes[i]->pos += Point(rand()%3 - 1, rand()%3 - 1);
      mSlimePos[i] += mSlimeVel[i];
      mSlimes[i]->pos = mSlimePos[i].ToPnt();
      // mSlimes[i]->Draw(pGl->pDr);
    }

    if (t >= 0)
      t--;

    pGl->pGraph->RefreshAll();
  }

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp) {
    if (t < 0)
      BasicController::OnKey(c, bUp);
  }

  /*virtual*/ void OnMouseDown(Point pPos) {
    if (t < 0)
      BasicController::OnMouseDown(pPos);
  }
};

struct Cutscene : public BasicController {
  SSP<FancyCritter> pCrRun;
  SSP<FancyCritter> pCrFollow;

  Timer tm;
  bool Beepy;

  bool bRelease;

  Cutscene(SP<TwrGlobalController> pGl_, Rectangle rBound_, std::string sRun,
           std::string sChase, bool bFlip = false)
      : BasicController(pGl_, rBound_, Color(0, 0, 0)), pCrRun(this, 0),
        pCrFollow(this, 0), bRelease(false), tm(nFramesInSecond / 5),
        Beepy(true) {
    ImageSequence seq1 = pGl_->pr(sRun);

    int xPos = 5;
    if (bFlip)
      xPos = rBound_.sz.x - 5;
    float m = 1;
    if (bFlip)
      m = -1;

    SP<FancyCritter> pCr1 =
        new FancyCritter(7, fPoint(xPos, rBound_.sz.y / 2), fPoint(m * 10, 0),
                         rBound, 3, seq1, nFramesInSecond / 5);
    AddBoth(pCr1);

    pCrRun = pCr1;

    ImageSequence seq2 = pGl_->pr(sChase);

    SP<FancyCritter> pCr2 =
        new FancyCritter(7, fPoint(xPos, rBound_.sz.y / 2), fPoint(m * 12, 0),
                         rBound, 3, seq2, nFramesInSecond / 5);

    pCrFollow = pCr2;
  }

  /*virtual*/ void Update() {
    if (!bRelease && pCrRun->GetPosition().x >= rBound.sz.x / 3 &&
        pCrRun->GetPosition().x <= rBound.sz.x * 2 / 3) {
      bRelease = true;

      SP<FancyCritter> pCr2 = pCrFollow;
      AddBoth(pCr2);
    }

    if (!pCrFollow->bExist) {
      pGl->Next();
    }

    if (tm.Tick()) {
      if (Beepy)
        pGl->pSnd->PlaySound(pGl->pr.GetSnd("beep"));
      else
        pGl->pSnd->PlaySound(pGl->pr.GetSnd("boop"));

      Beepy = !Beepy;
    }

    BasicController::Update();
  }

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp) {
    if (!bUp && c == '\\' && pGl->sbCheatsOn.Get())
      pGl->Next();
  }

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

  FireballBonus(int nNum_, bool bDef) : nNum(nNum_) {
    if (bDef) {
      fMap["speed"] = 5;
      fMap["frequency"] = fInitialFrequency;
      uMap["pershot"] = 1;
      uMap["total"] = nInitialFireballs;
    }
  }

  void Add(std::string str, float f) { fMap[str] += f; }
  void Add(std::string str, unsigned u) { uMap[str] += u; }
  void Add(std::string str, bool b) { bMap[str] |= b; }

  template <class T>
  FireballBonus(int nNum_, std::string str, T t) : nNum(nNum_) {
    Add(str, t);
  }

  FireballBonus &operator+=(const FireballBonus &f) {
    Union(fMap, f.fMap);
    Union(uMap, f.uMap);
    Union(bMap, f.bMap);

    return *this;
  }
};

inline std::ostream &operator<<(std::ostream &ofs, FireballBonus b) {
  Out(ofs, b.fMap) << "\n";
  Out(ofs, b.uMap) << "\n";
  Out(ofs, b.bMap) << "\n";

  return ofs;
}

/** Chain reaction generation count or infinite; Evolve decrements, IsLast when
 * 0. */
struct Chain {
  bool bInfinite;
  unsigned nGeneration;

  Chain(bool bInfinite_ = false) : bInfinite(bInfinite_), nGeneration(0) {}
  Chain(unsigned nGeneration_) : bInfinite(false), nGeneration(nGeneration_) {}

  Chain Evolve() {
    if (bInfinite)
      return Chain(true);
    else if (nGeneration == 0)
      return Chain();
    else
      return Chain(nGeneration - 1);
  }

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
  /*virtual*/ void Update() {
    if (SimpleVisualEntity::t.Check()) {
      r += delta;
    }

    CleanUp(pBc->lsPpl);

    for (std::list<ASSP<ConsumableEntity>>::iterator itr = pBc->lsPpl.begin();
         itr != pBc->lsPpl.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      if (this->HitDetection(*itr)) {
        if ((*itr)->GetType() == 'W')
          continue;
        if ((*itr)->GetType() == 'E')
          continue;

        (*itr)->OnHit('F');

        if (!ch.IsLast()) {
          SP<ChainExplosion> pCE =
              new ChainExplosion(AnimationOnce(GetPriority(), Reset(seq),
                                               SimpleVisualEntity::t.nPeriod,
                                               (*itr)->GetPosition(), true),
                                 r_in, delta, pBc, ch.Evolve());
          pBc->AddBoth(pCE);
        }
      }
    }

    AnimationOnce::Update();
  }

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

  void RandomizeVelocity() {
    fVel = RandomAngle();
    fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fKnightFireSpeed);
  }

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

  BonusScore(SP<AdvancedController> pAc_, Point p_, unsigned nScore_)
      : p(p_ + Point(0, -5)), t(unsigned(.1F * nFramesInSecond)), nC(0),
        pAc(this, pAc_), c(255, 255, 0), nScore(nScore_), nScoreSoFar(0) {
    std::ostringstream ostr(sText);
    ostr << '+' << nScore;
    sText = ostr.str();
  }

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

  /*virtual*/ void Update() {
    if (nTheme != -1)
      plr.SwitchTheme(nTheme);
    else
      plr.StopMusic();
  }
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

  void On() {
    bPressed = true;
    nCounter = 0;
  }
  int Off() {
    bPressed = false;
    return nCounter;
  }
  void Update() {
    if (bPressed)
      ++nCounter;
  }

  void UpdateMouse(Point pMouse_) {
    pMouse = pMouse_;
    pMouse.x /= 2;
    pMouse.y /= 2;
  }
  void UpdateLastDownPosition(Point pMouse_) {
    pLastDownPosition = pMouse_;
    pLastDownPosition.x /= 2;
    pLastDownPosition.y /= 2;
  }
  fPoint GetDirection(fPoint fDragonPos) { return pMouse - fDragonPos; }
  fPoint GetFlightDirection(fPoint fDragonPos) {
    return pLastDownPosition - fDragonPos;
  }
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

  // Timer tShootTimer;

  PositionTracker pt;
  bool bTakeOffToggle;

  TutorialLevelOne tutOne;
  TutorialLevelTwo tutTwo;
  SSP<TutorialTextEntity> pTutorialText;

  AdvancedController(const AdvancedController &a)
      : BasicController(a), tr(a.tr), t(a.t), bCh(a.bCh), nLvl(a.nLvl),
        nSlimeNum(a.nSlimeNum), bPaused(a.bPaused), tLoseTimer(a.tLoseTimer),
        bGhostTime(a.bGhostTime), tBlink(a.tBlink), bBlink(a.bBlink),
        pGr(a.pGr), tStep(a.tStep), bLeft(a.bLeft), pSc(a.pSc),
        bLeftDown(a.bLeftDown), bRightDown(a.bRightDown), nLastDir(a.nLastDir),
        bWasDirectionalInput(a.bWasDirectionalInput), pt(a.pt), mc(a.mc),
        bFirstUpdate(true), pTutorialText(this, a.pTutorialText),
        tutOne(a.tutOne), tutTwo(a.tutTwo), bTimerFlash(a.bTimerFlash),
        pMgGen(a.pMgGen) {
    CopyArrayASSP(this, a.vCs, vCs);
    CopyArrayASSP(this, a.vRd, vRd);
    CopyArrayASSP(this, a.vDr, vDr);
    CopyArrayASSP(this, a.lsBonus, lsBonus);
    CopyArrayASSP(this, a.lsSlimes, lsSlimes);
    CopyArrayASSP(this, a.lsSliminess, lsSliminess);
  }

  AdvancedController(SP<TwrGlobalController> pGl_, Rectangle rBound, Color c,
                     const LevelLayout &lvl);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void Update();

  /*virtual*/ void OnMouse(Point pPos);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ void OnMouseUp();
  /*virtual*/ void Fire();

  float GetCompletionRate();

  // void SetDir(Point pPos, bool bInTower);

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

  /*virtual*/ void OnMouseDown(Point pPos) {
    if (bClickToExit)
      pGl->Next();
  }
};

int GetFireballRaduis(FireballBonus &fb) {
  int n = fb.uMap["big"];

  if (n == 0)
    return 6;
  else if (n == 1)
    return 9;
  else if (n == 2)
    return 12;
  else
    return 18;
}

std::string GetSizeSuffix(FireballBonus &fb) {
  int n = fb.uMap["big"];

  if (n == 0)
    return "";
  else if (n == 1)
    return "_15";
  else if (n == 2)
    return "_2";
  else
    return "_3";
}

float GetExplosionInitialRaduis(FireballBonus &fb) {
  int n = fb.uMap["big"];
  float fCf;

  if (n == 0)
    fCf = 1;
  else if (n == 1)
    fCf = 1.5;
  else if (n == 2)
    fCf = 2;
  else
    fCf = 3;

  return 3 * fCf;
}

float GetExplosionExpansionRate(FireballBonus &fb) {
  int n = fb.uMap["big"];
  float fCf;

  if (n == 0)
    fCf = 1;
  else if (n == 1)
    fCf = 1.5;
  else if (n == 2)
    fCf = 2;
  else
    fCf = 3;

  return 3.9F * fCf;
}

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
           Chain ch_ = Chain(), unsigned nChain_ = 1)
      : Critter(GetFireballRaduis(fb_), p, v, pBc_->rBound, 5.F,
                ImageSequence(), nFramesInSecond / 10),
        pBc(this, pBc_), fb(fb_), ch(ch_), nChain(nChain_) {
    Critter::fVel.Normalize(fb.fMap["speed"]);

    if (!fb.bMap["laser"])
      Critter::seq = pBc->pGl->pr("fireball" + GetSizeSuffix(fb));
    else {
      Polar pol(Critter::fVel);
      unsigned n = DiscreetAngle(pol.a, 16);
      Critter::seq =
          ImageSequence(pBc->pGl->pr("laser" + GetSizeSuffix(fb)).vImage[n]);
    }
  }

  /*virtual*/ void Update() {
    CleanUp(pBc->lsPpl);

    bool bMultiHit = false;

    for (std::list<ASSP<ConsumableEntity>>::iterator itr = pBc->lsPpl.begin();
         itr != pBc->lsPpl.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      if (this->HitDetection(*itr)) {
        char cType = (*itr)->GetType();

        if (cType == 'W' || cType == 'E') {
          (*itr)->OnHit('F');

          bExist = false;
          return;
        } else
          pBc->pGl->pSnd->PlaySound(pBc->pGl->pr.GetSnd("death"));

        if ((*itr)->GetType() != 'K' || (fb.uMap["setonfire"] == 0))
          (*itr)->OnHit('F');
        else {
          (*itr)->bExist = false;
          SP<KnightOnFire> pKn = new KnightOnFire(
              Critter((*itr)->GetRadius(), (*itr)->GetPosition(), fPoint(),
                      rBound, 1.F, ImageSequence(), true),
              pBc, 15 * nFramesInSecond, Chain(fb.uMap["setonfire"]));
          pBc->AddBoth(pKn);
        }

        if (!bMultiHit) {
          bool bKeepGoing =
              (fb.uMap["through"] != 0) || fb.bMap["through_flag"];

          if (nChain != 0 || !bKeepGoing)
            bExist = false;

          if (bKeepGoing) {
            if (fb.bMap["through_flag"])
              fb.bMap["through_flag"] = false;
            else
              --fb.uMap["through"];
          }

          if (bKeepGoing) {
            // fPoint v = RandomAngle(fVel, 1.F/12);
            fPoint v = fVel;

            for (unsigned i = 0; i < nChain; ++i) {
              SP<Fireball> pFb = new Fireball(
                  (*itr)->GetPosition(), GetWedgeAngle(v, 1.F / 6, i, nChain),
                  pBc, fb, Chain(), nChain);
              pBc->AddBoth(pFb);
            }
          }

          if (fb.uMap["explode"] > 0) {
            SP<ChainExplosion> pEx;

            if (!fb.bMap["laser"]) {
              pEx = new ChainExplosion(
                  AnimationOnce(GetPriority(),
                                pBc->pGl->pr("explosion" + GetSizeSuffix(fb)),
                                nFramesInSecond / 10, (*itr)->GetPosition(),
                                true),
                  GetExplosionInitialRaduis(fb), GetExplosionExpansionRate(fb),
                  pBc, Chain(fb.uMap["explode"] - 1));
            } else {
              pEx = new ChainExplosion(
                  AnimationOnce(GetPriority(),
                                pBc->pGl->pr("laser_expl" + GetSizeSuffix(fb)),
                                nFramesInSecond / 10, (*itr)->GetPosition(),
                                true),
                  GetExplosionInitialRaduis(fb), GetExplosionExpansionRate(fb),
                  pBc, Chain(fb.uMap["explode"] - 1));
            }

            pBc->AddBoth(pEx);

            pBc->pGl->pSnd->PlaySound(pBc->pGl->pr.GetSnd("explosion"));
          }
        }

        bMultiHit = true;
      }
    }

    Critter::Update();
  }
};

/** FireballBonus that updates on a timer (e.g. temporary power-up). */
struct TimedFireballBonus : public FireballBonus, virtual public EventEntity {
  Timer t;

  TimedFireballBonus(const FireballBonus &fb, unsigned nPeriod)
      : FireballBonus(fb), t(nPeriod) {}

  /*virtual*/ void Update() {
    if (t.Tick())
      bExist = false;
  }
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

  /*virtual*/ void Update() {
    Fireball::Update();

    if (t.Tick())
      bExist = false;

    fPoint p = fPoint(GetPosition()) - i_pos;
    if (p.Length() < fRadius)
      return;
    fPoint fPen(-p.y, p.x);
    p.Normalize((p.Length() - fRadius) * 2);
    fPen -= p;

    fPen.Normalize(fVel.Length());
    fVel = fPen;

    if (fb.bMap["laser"]) {
      Polar pol(Critter::fVel);
      unsigned n = DiscreetAngle(pol.a, 16);
      Critter::seq =
          ImageSequence(pBc->pGl->pr("laser" + GetSizeSuffix(fb)).vImage[n]);
    }
  }
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

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    unsigned n = pDr->nFactor;
    if (bVertical)
      pDr->pGr->DrawRectangle(Rectangle((nCoord - 5) * n, rBound.p.y * n,
                                        (nCoord + 5) * n, rBound.sz.y * n),
                              Color(63, 63, 63), false);
    else
      pDr->pGr->DrawRectangle(Rectangle(rBound.p.x * n, (nCoord - 5) * n,
                                        rBound.sz.x * n, (nCoord + 5) * n),
                              Color(63, 63, 63), false);
  }

  void RoadMap(Point &p, Point &v) {
    if (rand() % 2) {
      if (!bVertical) {
        p.y = nCoord - 7;
        p.x = rBound.p.x;
        v = Point(1, 0);
      } else {
        p.x = nCoord;
        p.y = rBound.p.y;
        v = Point(0, 1);
      }
    } else {
      if (!bVertical) {
        p.y = nCoord - 7;
        p.x = rBound.sz.x - 1;
        v = Point(-1, 0);
      } else {
        p.x = nCoord;
        p.y = rBound.sz.y - 1;
        v = Point(0, -1);
      }
    }
  }
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

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    unsigned n = pDr->nFactor;
    Image *p = pDr->pGr->GetImage(pAd->pGl->pr["road"]);
    Size sz = p->GetSize();

    if (bVertical)
      for (int i = 0; (i - 1) * sz.y < rBound.sz.y * int(n); ++i)
        pDr->pGr->DrawImage(Point(nCoord * n - sz.x / 2, i * sz.y),
                            pAd->pGl->pr["road"], false);
    else
      for (int i = 0; (i - 1) * sz.x < rBound.sz.x * int(n); ++i)
        pDr->pGr->DrawImage(Point(i * sz.x, nCoord * n - sz.y / 2),
                            pAd->pGl->pr["road"], false);
  }
};

/** Princess unit: Critter + ConsumableEntity, captured by dragon. */
struct Princess : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;

  Princess(const Critter &cr, SP<AdvancedController> pAc_)
      : Critter(cr), pAc(this, pAc_) {}

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'P'; }

  /*virtual*/ void OnHit(char cWhat) {
    SP<BonusScore> pB = new BonusScore(pAc, GetPosition(), 250);
    pAc->AddBoth(pB);

    bExist = false;

    SP<AnimationOnce> pAn = new AnimationOnce(
        GetPriority(),
        fVel.x < 0 ? pAc->pGl->pr("princess_die_f")
                   : pAc->pGl->pr("princess_die"),
        unsigned(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true);

    pAc->AddBoth(pAn);
  }

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    Critter::Draw(pDr);

    Point p = GetPosition();
    p.y += 13;
#ifdef UNDERLINE_UNIT_TEXT
    if (sUnderText != "")
      pAc->pGl->pNum->DrawWord(sUnderText, p, true);
#endif
  }
};

/** Spawns skeleton knights on a timer at a position. */
struct SkellyGenerator : public EventEntity {
  Timer t;
  Point p;

  SSP<AdvancedController> pAdv;

  SkellyGenerator(Point p_, SP<AdvancedController> pAdv_)
      : p(p_), t(unsigned(.7F * nFramesInSecond)), pAdv(this, pAdv_) {
    SP<AnimationOnce> pSlm =
        new AnimationOnce(2.F, pAdv->pGl->pr("skelly_summon"),
                          unsigned(.1F * nFramesInSecond), p_, true);
    pAdv_->AddBoth(pSlm);
  }

  /*virutal*/ void Update();
};

inline int GetRandTimeFromRate(float fRate) {
  return 1 + int((float(rand()) / RAND_MAX * 1.5 + .25) * fRate);
}

void SummonSkeletons(SP<AdvancedController> pAc, Point p) {
  int nNum = 4;

  if (pAc->nLvl > 6)
    nNum = 6;
  if (pAc->nLvl >= 10)
    nNum = 8;

  for (int i = 0; i < nNum; ++i) {
    fPoint f = GetWedgeAngle(Point(1, 1), 1, i, nNum + 1);
    f.Normalize(15);

    pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("slime_summon"));
    SP<SkellyGenerator> pSkel = new SkellyGenerator(p + f.ToPnt(), pAc);
    pAc->AddE(pSkel);
  }
}

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

  Mage(const Critter &cr, SP<AdvancedController> pAc_, bool bAngry_)
      : Critter(cr), pAc(this, pAc_), bAngry(bAngry_), bCasting(false),
        tUntilSpell(GetTimeUntillSpell()), tSpell(3 * nFramesInSecond),
        tSpellAnimate(unsigned(.7F * nFramesInSecond)) {
    fMvVel = Critter::fVel;

    // meh
    bAngry = true;
    pAc->pGl->bAngry = true;
  }

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  void SummonSlimes();

  /*virtual*/ char GetType() { return 'M'; }

  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat) {
    bExist = false;

    SP<AnimationOnce> pAn = new AnimationOnce(
        GetPriority(),
        fVel.x < 0 ? pAc->pGl->pr("mage_die_f") : pAc->pGl->pr("mage_die"),
        unsigned(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true);

    pAc->AddBoth(pAn);

    pAc->pGl->bAngry = true;

    if (pAc->nLvl > 6)
      SummonSlimes();
    // SummonSkeletons(pAc, GetPosition());
  }
};

unsigned RandomBonus(bool bInTower = true);

ImageSequence GetBonusImage(int n, Preloader &pr) {
  if (n == 0)
    return pr("void_bonus");
  if (n == 1)
    return pr("pershot_bonus");
  if (n == 2)
    return pr("laser_bonus");
  if (n == 3)
    return pr("big_bonus");
  if (n == 4)
    return pr("totnum_bonus");
  if (n == 5)
    return pr("explode_bonus");
  if (n == 6)
    return pr("split_bonus");
  if (n == 7)
    return pr("burning_bonus");
  if (n == 8)
    return pr("ring_bonus");
  if (n == 9)
    return pr("nuke_bonus");
  if (n == 10)
    return pr("speed_bonus");

  return pr("void_bonus");
}

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

  FireballBonusAnimation(Point p_, unsigned n_, SP<AdvancedController> pAd_)
      : Animation(.5F, ImageSequence(), nFramesInSecond / 10, p_, true), n(n_),
        bBlink(false), pAd(this, pAd_), tm(nBonusOnGroundTime), sUnderText("") {
    seq = GetBonusImage(n, pAd->pGl->pr);
    coronaSeq = pAd->pGl->pr("corona");
  }

  /*virtual*/ unsigned GetRadius() { return 20U; }

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    pDr->Draw(coronaSeq.GetImage(), GetPosition(), bCenter);
    Animation::Draw(pDr);

    Point p = GetPosition();
    p.y += 13;

#ifdef UNDERLINE_UNIT_TEXT
    if (sUnderText != "")
      pAd->pGl->pNum->DrawWord(sUnderText, p, true);
#endif // UNDERLINE_UNIT_TEXT
  }

  /*virtual*/ void Update() {
    coronaSeq.Toggle();
    Animation::Update();

    if (tm.Tick())
      bExist = false;

    if (!bBlink && tm.nPeriod && (tm.nPeriod - tm.nTimer) < 7 * nFramesInSecond)
    // if(!bBlink && float(tm.nPeriod - tm.nTimer)/tm.nPeriod < .15f)
    {
      bBlink = true;

      ImageSequence img;

      unsigned nSz = seq.vImage.size();

      for (unsigned i = 0; i < nSz; ++i) {
        int nLm = 1;
        if (seq.vIntervals.size() > i)
          nLm = seq.vIntervals[i];
        if (nLm == 0)
          nLm = 1;
        for (int j = 0; j < nLm; ++j) {
          img.Add(seq.vImage[i]);
          img.Add(pAd->pGl->pr["empty"]);
        }
      }

      seq = img;
    }
  }
};

/** Trader unit: drops bonus, bFirstBns ref for first-bonus logic. */
struct Trader : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;
  bool &bFirstBns;

  Trader(const Critter &cr, SP<AdvancedController> pAc_, bool &bFirstBns_)
      : Critter(cr), pAc(this, pAc_), bFirstBns(bFirstBns_) {}

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'T'; }

  /*virtual*/ void OnHit(char cWhat) {
    SP<BonusScore> pB = new BonusScore(pAc, GetPosition(), 60);
    pAc->AddBoth(pB);

    bExist = false;

    pAc->tutTwo.TraderKilled();

    SP<AnimationOnce> pAn = new AnimationOnce(
        GetPriority(),
        fVel.x < 0 ? pAc->pGl->pr("trader_die") : pAc->pGl->pr("trader_die_f"),
        unsigned(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true);

    pAc->AddBoth(pAn);

    SP<FireballBonusAnimation> pFb =
        new FireballBonusAnimation(GetPosition(), RandomBonus(false), pAc);
    if (bFirstBns) {
      pFb->sUnderText = "loot";
      bFirstBns = false;
    }
    pAc->AddBoth(pFb);
    PushBackASSP(pAc.GetRawPointer(), pAc->lsBonus, pFb);
  }

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    Critter::Draw(pDr);

    Point p = GetPosition();
    p.y += 13;
    // p.x *= pDr->nFactor;
    // p.y *= pDr->nFactor;
#ifdef UNDERLINE_UNIT_TEXT
    if (sUnderText != "")
      pAc->pGl->pNum->DrawWord(sUnderText, p, true);
#endif // UNDERLINE_UNIT_TEXT
  }
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

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    Critter::Draw(pDr);

    Point p = GetPosition();
    p.y += 13;
#ifdef UNDERLINE_UNIT_TEXT
    if (sUnderText != "")
      pAc->pGl->pNum->DrawWord(sUnderText, p, true);
#endif // UNDERLINE_UNIT_TEXT
  }

  void KnockBack() {
    if (fVel != fPoint(0, 0))
      fPos -= fVel / fVel.Length();
  }

  /*virtual*/ void Update() {
    for (unsigned i = 0; i < pAc->vCs.size(); ++i)
      if (this->HitDetection(pAc->vCs[i])) {
        pAc->vCs[i]->OnKnight(GetType());

        bExist = false;
        break;
      }

    if (cType == 'S') {
      CleanUp(pAc->lsPpl);

      for (std::list<ASSP<ConsumableEntity>>::iterator itr = pAc->lsPpl.begin();
           itr != pAc->lsPpl.end(); ++itr) {
        if (!(*itr)->bExist)
          continue;

        if (this->HitDetection(*itr)) {

          if ((*itr)->GetType() == 'P' || (*itr)->GetType() == 'T') {
            pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("death"));
            (*itr)->OnHit('S');
          }
        }
      }

      CleanUp(pAc->lsBonus);

      for (std::list<ASSP<FireballBonusAnimation>>::iterator itr =
               pAc->lsBonus.begin();
           itr != pAc->lsBonus.end(); ++itr) {
        if (!(*itr)->bExist)
          continue;

        if (this->HitDetection(*itr)) {
          pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("skeleton_bonus"));
          (*itr)->bExist = false;
        }
      }
    }

    Point p = GetPosition();
    if (p != pPrev) {
      bImageToggle = !bImageToggle;

      if (bImageToggle) // too smooth if every pixel
      {
        seq.Toggle();

        // golem
        if (seq.nActive == 3)
          pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("step_left"));
        else if (seq.nActive == 6)
          pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("step_right"));
      }
    }
    pPrev = p;
  }

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() {
    return cType;

    /*
    if(!bGhost)
    {
            if(bSkeleton)
                    return 'S';
            else
                    return 'K';
    }
    else
    {
            return 'G';
    }
    */
  }
};

/** Large slime unit: splits or merges (MegaSlime logic). */
struct MegaSlime : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;

  int nHealth;

  MegaSlime(fPoint fPos, Rectangle rBound, SP<AdvancedController> pAc_)
      : Critter(8, fPos, fPoint(0, 0), rBound, 3, pAc_->pGl->pr("megaslime"),
                nFramesInSecond / 5),
        pAc(this, pAc_), nHealth(nSlimeHealthMax) {
    bDieOnExit = false;
  }

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) { Critter::Draw(pDr); }

  void RandomizeVelocity() {
    fVel = RandomAngle();

    if (rand() % 7 == 0)
      fVel = fPoint(rBound.sz.x / 2, rBound.sz.y / 2) - fPoint(GetPosition());

    fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fSlimeSpeed);
  }

  /*virtual*/ void Update() {
    // if(t.Tick() && float(rand())/RAND_MAX < .25)
    //	RandomizeVelocity();

    CleanUp(pAc->lsBonus);

    for (std::list<ASSP<FireballBonusAnimation>>::iterator itr =
             pAc->lsBonus.begin();
         itr != pAc->lsBonus.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      if (this->HitDetection(*itr)) {
        (*itr)->bExist = false;
        pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_bonus"));
      }
    }

    if (t.Tick()) {
      seq.Toggle();
      t = Timer(nPeriod * seq.GetTime() + rand() % 2);

      if (seq.nActive == 11) {
        pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_jump"));
        RandomizeVelocity();
      } else if (seq.nActive == 16) {
        fVel = fPoint(0, 0);
        pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_land"));
      }
    }

    // Critter::Update();
  }

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
             int nGhostHit_)
      : p(p_), pAdv(this, pAdv_), knCp(knCp_), nGhostHit(nGhostHit_) {
    ImageSequence seq = pAdv->pGl->pr("ghost_knight_burn");
    if (nGhostHit == 0)
      seq = pAdv->pGl->pr("ghost_burn");

    unsigned n = unsigned(.2F * nFramesInSecond / fDeathMultiplier);

    t = Timer(n * seq.GetTotalTime());

    SP<AnimationOnce> pFire = new AnimationOnce(2.F, seq, n, p_, true);
    pAdv_->AddBoth(pFire);
  }

  /*virutal*/ void Update();
};

/** Slime unit: moves toward target, timer for behavior. */
struct Slime : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;
  Timer t;
  int nGeneration;

  Slime(fPoint fPos, Rectangle rBound, SP<AdvancedController> pAc_,
        int nGeneration_)
      : Critter(5, fPos, fPoint(0, 0), rBound, 3, pAc_->pGl->pr("slime"), true),
        pAc(this, pAc_), t(nFramesInSecond / 2), nGeneration(nGeneration_) {
    RandomizeVelocity();
    ++pAc->nSlimeNum;
  }

  int GetGeneration() { return nGeneration; }

  void RandomizeVelocity() {
    fVel = RandomAngle();

    if (rand() % 7 == 0)
      fVel = fPoint(rBound.sz.x / 2, rBound.sz.y / 2) - fPoint(GetPosition());

    fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fSlimeSpeed);
  }

  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'L'; }

  ~Slime() {
    if (pAc != 0) {
      --pAc->nSlimeNum;
      // std::cout << pAc->nSlimeNum << "\n";
    }
  }
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
            int nGeneration_)
      : p(p_), pAdv(this, pAdv_), bFast(bFast_), nGeneration(nGeneration_),
        pSlm(this, 0) {
    ImageSequence seq = bFast ? pAdv->pGl->pr("slime_reproduce_fast")
                              : pAdv->pGl->pr("slime_reproduce");

    t = bFast ? Timer(unsigned(1.3F * nFramesInSecond))
              : Timer(unsigned(2.3F * nFramesInSecond));

    SP<AnimationOnce> pSlmTmp =
        new AnimationOnce(2.F, seq, unsigned(.1F * nFramesInSecond), p_, true);
    pSlm = pSlmTmp;
    pAdv_->AddBoth(pSlmTmp);

    ++pAdv_->nSlimeNum;
  }

  /*virutal*/ void Update() {
    if (t.Tick()) {
      bExist = false;

      SP<Slime> pSlm = new Slime(p, pAdv->rBound, pAdv, nGeneration);
      pAdv->AddBoth(pSlm);
      PushBackASSP(pAdv.GetRawPointer(), pAdv->lsPpl, pSlm);
      PushBackASSP(pAdv.GetRawPointer(), pAdv->lsSlimes, pSlm);
    }
  }

  void Kill() {
    bExist = false;
    pSlm->bExist = false;
  }

  Point GetPosition() { return p; }

  ~Sliminess() {
    if (pAdv != 0)
      --pAdv->nSlimeNum;
  }
};

/** Spawns MegaSlimes; holds position and controller. */
struct MegaSliminess : public EventEntity {
  Point p;
  SSP<AdvancedController> pAdv;
  SSP<AnimationOnce> pSlm;

  MegaSliminess(Point p_, SP<AdvancedController> pAdv_)
      : p(p_), pAdv(this, pAdv_), pSlm(this, 0) {
    ImageSequence seq = pAdv->pGl->pr("megaslime_reproduce");

    SP<AnimationOnce> pSlmTmp =
        new AnimationOnce(2.F, seq, unsigned(.1F * nFramesInSecond), p_, true);
    pSlm = pSlmTmp;
    pAdv_->AddBoth(pSlmTmp);

    pAdv->pGl->pSnd->PlaySound(pAdv->pGl->pr.GetSnd("slime_spawn"));
  }

  /*virutal*/ void Update() {
    if (pSlm->bExist == false) {
      bExist = false;

      SP<MegaSlime> pSlm = new MegaSlime(p, pAdv->rBound, pAdv);
      pAdv->AddBoth(pSlm);
      PushBackASSP(pAdv.GetRawPointer(), pAdv->lsPpl, pSlm);
    }
  }
};

/** SimpleVisualEntity that moves with fPos/fVel (e.g. menu slime). */
struct FloatingSlime : public SimpleVisualEntity {
  fPoint fPos;
  fPoint fVel;

  Timer tTermination;

  FloatingSlime(ImageSequence seq, Point pStart, Point pEnd, unsigned nTime)
      : SimpleVisualEntity(2.F, seq, true, unsigned(.1F * nFramesInSecond)) {
    fPos = pStart;
    tTermination = Timer(nTime);
    fVel = (fPoint(pEnd) - fPoint(pStart)) / float(nTime);
  }

  /*virtual*/ Point GetPosition() { return fPos.ToPnt(); }
  /*virtual*/ void Update() {
    SimpleVisualEntity::Update();

    fPos += fVel;

    if (tTermination.Tick())
      bExist = false;
  }
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

  void CloseLast() {
    if (vEdges.empty() || vEdges.back().empty())
      throw SegmentSimpleException("CloseLast",
                                   "Invalid (empty) segment arrays");

    if (vEdges.back().front() != vEdges.back().back())
      vEdges.back().push_back(vEdges.back().front());
  }

  void Add(fPoint p) {
    if (vEdges.empty())
      vEdges.push_back(VecLine());
    vEdges.back().push_back(p);
  }

  void AddLine(const VecLine &vl) { vEdges.push_back(vl); }

  void Join(const BrokenLine &b) {
    for (unsigned i = 0; i < b.vEdges.size(); ++i)
      vEdges.push_back(b.vEdges[i]);
  }

  BrokenLine() {}
  BrokenLine(fPoint p1) { Add(p1); }
  BrokenLine(fPoint p1, fPoint p2) {
    Add(p1);
    Add(p2);
  }
  BrokenLine(fPoint p1, fPoint p2, fPoint p3) {
    Add(p1);
    Add(p2);
    Add(p3);
  }
  BrokenLine(fPoint p1, fPoint p2, fPoint p3, fPoint p4) {
    Add(p1);
    Add(p2);
    Add(p3);
    Add(p4);
  }
  BrokenLine(Rectangle r) {
    Add(r.p);
    Add(fPoint(float(r.Left()), float(r.Bottom())));
    Add(fPoint(float(r.Right()), float(r.Top())));
    Add(r.GetBottomRight());
    CloseLast();
  }

  fPoint RandomByLength() {
    if (vEdges.empty())
      throw SegmentSimpleException("RandomByLength",
                                   "Invalid (empty) segment arrays");

    float fLength = 0;
    for (unsigned i = 0; i < vEdges.size(); ++i) {
      if (vEdges[i].empty())
        throw SegmentSimpleException("RandomByLength",
                                     "Invalid (empty) segment arrays");
      if (vEdges[i].size() == 1)
        continue;
      for (unsigned j = 1; j < vEdges[i].size(); ++j)
        fLength += (vEdges[i][j] - vEdges[i][j - 1]).Length();
    }

    if (fLength == 0)
      return RandomBySegment();

    fLength = fLength * rand() / RAND_MAX;

    for (unsigned i = 0; i < vEdges.size(); ++i) {
      if (vEdges[i].size() == 1)
        continue;

      for (unsigned j = 1; j < vEdges[i].size(); ++j) {
        fPoint f = vEdges[i][j] - vEdges[i][j - 1];
        if (fLength <= f.Length()) {
          f.Normalize(fLength);
          return vEdges[i][j - 1] + f;
        }

        fLength -= f.Length();
      }
    }

    return vEdges[0][0];
  }

  fPoint RandomBySegment() {
    if (vEdges.empty())
      throw SegmentSimpleException("RandomBySegment",
                                   "Invalid (empty) segment arrays");

    unsigned nSegment = rand() % vEdges.size();

    if (vEdges[nSegment].size() == 0)
      throw SegmentSimpleException("RandomBySegment",
                                   "Invalid (empty) segment arrays");
    if (vEdges[nSegment].size() == 1)
      return vEdges[nSegment][0];

    unsigned nSegment2 = rand() % (vEdges[nSegment].size() - 1);

    fPoint f = vEdges[nSegment][nSegment2 + 1] - vEdges[nSegment][nSegment2];
    f.Normalize(f.Length() * rand() / RAND_MAX);
    return vEdges[nSegment][nSegment2] + f;
  }
};

inline std::ostream &operator<<(std::ostream &ofs, const BrokenLine &bl) {
  for (unsigned i = 0; i < bl.vEdges.size(); ++i) {
    for (unsigned j = 0; j < bl.vEdges[i].size(); ++j)
      ofs << bl.vEdges[i][j] << " ";
    ofs << "| ";
  }
  ofs << "& ";
  return ofs;
}

inline std::istream &operator>>(std::istream &ifs, BrokenLine &bl) {
  bl.vEdges.clear();

  std::string str;

  std::getline(ifs, str, '&');
  ifs.get();

  std::istringstream istr(str);

  while (true) {
    std::string part;
    std::getline(istr, part, '|');

    if (istr.fail())
      break;

    istr.get();

    BrokenLine bPart;

    std::istringstream is(part);
    fPoint p;
    while (is >> p)
      bPart.Add(p);

    bl.Join(bPart);
  }

  return ifs;
}

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

  void Convert(int n = 24) {
    float p1 = float(sBound.sz.x) / n;
    float p2 = float(sBound.sz.y) / n;

    unsigned i, j;

    for (i = 0; i < blKnightGen.vEdges.size(); ++i)
      for (j = 0; j < blKnightGen.vEdges[i].size(); ++j) {
        blKnightGen.vEdges[i][j].x *= p1;
        blKnightGen.vEdges[i][j].y *= p2;
      }

    for (i = 0; i < vCastleLoc.size(); ++i) {
      vCastleLoc[i].x = Crd(vCastleLoc[i].x * p1);
      vCastleLoc[i].y = Crd(vCastleLoc[i].y * p2);
    }

    for (i = 0; i < vRoadGen.size(); ++i)
      if (vRoadGen[i].bVertical)
        vRoadGen[i].nCoord = Crd(vRoadGen[i].nCoord * p1);
      else
        vRoadGen[i].nCoord = Crd(vRoadGen[i].nCoord * p2);
  }
};

inline std::ostream &operator<<(std::ostream &ofs, const LevelLayout &f) {
  ofs << "LEVEL " << f.nLvl << "\n\n";

  ofs << "FREQ ";
  for (unsigned k = 0; k < f.vFreq.size(); ++k)
    ofs << f.vFreq[k] / nFramesInSecond << " ";
  ofs << "\n";

  ofs << "SPWN " << f.blKnightGen << "\n";

  ofs << "CSTL ";
  for (unsigned i = 0; i < f.vCastleLoc.size(); ++i)
    ofs << f.vCastleLoc[i] << " ";
  ofs << "\n";

  ofs << "ROAD ";
  for (unsigned j = 0; j < f.vRoadGen.size(); ++j)
    ofs << f.vRoadGen[j] << " ";
  ofs << "\n";

  ofs << "TIME " << f.nTimer << "\n";

  ofs << "\n";

  return ofs;
}

inline std::istream &operator>>(std::istream &ifs, LevelLayout &f) {
  f = LevelLayout(f.sBound);

  ParsePosition("LEVEL", ifs);

  ifs >> f.nLvl;

  {
    std::string str;
    ParseGrabLine("FREQ", ifs, str);
    std::istringstream istr(str);

    float n;
    while (istr >> n) {
      n *= nFramesInSecond;

      f.vFreq.push_back(n);
    }
  }

  {
    std::string str;
    ParseGrabLine("SPWN", ifs, str);
    std::istringstream istr(str);

    istr >> f.blKnightGen;
  }

  {
    std::string str;
    ParseGrabLine("CSTL", ifs, str);
    std::istringstream istr(str);

    Point p;
    while (istr >> p)
      f.vCastleLoc.push_back(p);
  }

  {
    std::string str;
    ParseGrabLine("ROAD", ifs, str);
    std::istringstream istr(str);

    Road r(0, 0, f.sBound);
    while (istr >> r)
      f.vRoadGen.push_back(r);
  }

  {
    std::string str;
    ParseGrabLine("TIME", ifs, str);
    std::istringstream istr(str);

    istr >> f.nTimer;

    f.nTimer *= nFramesInSecond;
  }

  return ifs;
}

/** Spawns knights along a path on a timer. */
struct KnightGenerator : virtual public EventEntity {
  bool bFirst;

  float dRate;
  Rectangle rBound;
  SSP<AdvancedController> pBc;
  ImageSequence seq;

  Timer tm;

  BrokenLine bl;

  float GetRate() {
    if (pBc->bGhostTime)
      return dRate / fIncreaseKnightRate2;

    if (pBc->GetCompletionRate() < fIncreaseRateFraction1)
      return dRate;
    else if (pBc->GetCompletionRate() < fIncreaseRateFraction2)
      return dRate / fIncreaseKnightRate1;
    else
      return dRate / fIncreaseKnightRate2;
  }

  KnightGenerator(float dRate_, Rectangle rBound_, SP<AdvancedController> pBc_,
                  const BrokenLine &bl_)
      : dRate(dRate_), rBound(rBound_), pBc(this, pBc_),
        seq(pBc_->pGl->pr("knight")), bl(bl_), tm(1), bFirst(false) {
    if (pBc->nLvl == 1 && pBc->pGl->nHighScore == 0)
      bFirst = true;
#ifdef TRIAL_VERSION
    if (pBc->nLvl == 1)
      bFirst = true;
#endif
  }

  void Generate(bool bGolem = false) {
    Point p = bl.RandomByLength().ToPnt();

    unsigned n = unsigned(rand() % pBc->vCs.size());

    fPoint v = pBc->vCs[n]->GetPosition() - p;
    v.Normalize(fKnightSpeed);
    p += rBound.p;

    SP<Knight> pCr =
        new Knight(Critter(7, p, v, rBound, 3, seq, true), pBc, 'K');

    if (bFirst) {
      pCr->sUnderText = "destroy";
      bFirst = false;
    }

    if (bGolem) {
      pCr = new Knight(
          Critter(14, p, v * .5, rBound, 3,
                  v.x < 0 ? pBc->pGl->pr("golem") : pBc->pGl->pr("golem_f"),
                  true),
          pBc, 'W');
    } else if (pBc->bGhostTime) // && (pBc->nLvl <= 6)) we want BOTH ghosts and
                                // golems
    {
      pCr->seq = pBc->pGl->pr("ghost_knight");
      pCr->cType = 'G';
      pCr->fVel.Normalize(fKnightSpeed * fGhostSpeedMultiplier);
    }

    pBc->AddBoth(pCr);
    PushBackASSP(pBc.GetRawPointer(), pBc->lsPpl, pCr);
  }

  /*virtual*/ void Update() {
    if (tm.Tick()) {
      tm = Timer(GetRandTimeFromRate(GetRate()));
      Generate();
    }
  }
};

/** Spawns princesses at a rate within bounds. */
struct PrincessGenerator : virtual public EventEntity {
  float dRate;
  Rectangle rBound;
  SSP<AdvancedController> pBc;
  Timer tm;
  bool bFirst;

  PrincessGenerator(float dRate_, Rectangle rBound_,
                    SP<AdvancedController> pBc_)
      : dRate(dRate_), rBound(rBound_), pBc(this, pBc_),
        tm(GetRandTimeFromRate(dRate_)), bFirst(false) {
    if (pBc->nLvl == 1 && pBc->pGl->nHighScore == 0)
      bFirst = true;

#ifdef TRIAL_VERSION
    if (pBc->nLvl == 1)
      bFirst = true;
#endif
  }

  /*virtual*/ void Update() {
    // if(float(rand())/RAND_MAX < dRate || tm.Tick())
    if (tm.Tick()) {
      tm = Timer(GetRandTimeFromRate(dRate));

      Point p, v;

      pBc->vRd[rand() % pBc->vRd.size()]->RoadMap(p, v);

      fPoint vel(v);

      vel.Normalize(fPrincessSpeed);

      SP<Princess> pCr =
          new Princess(Critter(7, p, vel, rBound, 3,
                               vel.x < 0 ? pBc->pGl->pr("princess_f")
                                         : pBc->pGl->pr("princess"),
                               true),
                       pBc);
      if (bFirst) {
        pCr->sUnderText = "capture";
        bFirst = false;
      }
      pBc->AddBoth(pCr);
      PushBackASSP(pBc.GetRawPointer(), pBc->lsPpl, pCr);
      pBc->pGl->pSnd->PlaySound(pBc->pGl->pr.GetSnd("princess_arrive"));

      pBc->tutOne.PrincessGenerate();
    }
  }
};

struct MageGenerator : virtual public EventEntity {
  float dRate;
  Rectangle rBound;
  SSP<AdvancedController> pBc;
  Timer tm;

  MageGenerator(float dRate_, float dAngryRate_, Rectangle rBound_,
                SP<AdvancedController> pBc_)
      : rBound(rBound_), pBc(this, pBc_) {
    if (pBc->pGl->bAngry)
      dRate = dAngryRate_;
    else
      dRate = dRate_;

    tm = Timer(GetRandTimeFromRate(dRate));
  }

  /*virtual*/ void Update() {
    // if(float(rand())/RAND_MAX < dRate )
    if (tm.Tick()) {
      if (dRate == 0)
        return;

      tm = Timer(GetRandTimeFromRate(dRate));

      MageGenerate();
    }
  }

  void MageGenerate() {
    Point p, v;

    pBc->vRd[rand() % pBc->vRd.size()]->RoadMap(p, v);

    fPoint vel(v);

    vel.Normalize(fMageSpeed);

    SP<Mage> pCr = new Mage(
        Critter(7, p, vel, rBound, 3,
                vel.x < 0 ? pBc->pGl->pr("mage_f") : pBc->pGl->pr("mage"),
                true),
        pBc, pBc->pGl->bAngry);
    pBc->AddBoth(pCr);
    PushBackASSP(pBc.GetRawPointer(), pBc->lsPpl, pCr);
  }
};

/** Spawns traders at a rate within bounds. */
struct TraderGenerator : virtual public EventEntity {
  float dRate;
  Rectangle rBound;
  SSP<AdvancedController> pBc;
  Timer tm;
  bool bFirst;
  bool bFirstBns;

  float GetRate() {
    if (pBc->GetCompletionRate() < fIncreaseRateFraction1)
      return dRate;
    else if (pBc->GetCompletionRate() < fIncreaseRateFraction2)
      return dRate / fIncreaseTraderRate1;
    else
      return dRate / fIncreaseTraderRate2;
  }

  TraderGenerator(float dRate_, Rectangle rBound_, SP<AdvancedController> pBc_)
      : dRate(dRate_), rBound(rBound_), pBc(this, pBc_),
        tm(GetRandTimeFromRate(dRate_)), bFirst(false), bFirstBns(false) {
    if (pBc->nLvl == 1 && pBc->pGl->nHighScore == 0) {
      bFirst = true;
      bFirstBns = true;
    }

#ifdef TRIAL_VERSION
    if (pBc->nLvl == 1) {
      bFirst = true;
      bFirstBns = true;
    }
#endif
  }

  /*virtual*/ void Update() {
    if (tm.Tick()) {
      tm = Timer(GetRandTimeFromRate(GetRate()));

      Point p, v;

      pBc->vRd[rand() % pBc->vRd.size()]->RoadMap(p, v);

      fPoint vel(v);
      vel.Normalize(fTraderSpeed);

      SP<Trader> pCr = new Trader(
          Critter(7, p, vel, rBound, 3,
                  vel.x < 0 ? pBc->pGl->pr("trader") : pBc->pGl->pr("trader_f"),
                  true),
          pBc, bFirstBns);

      if (bFirst) {
        pCr->sUnderText = "kill";
        bFirst = false;
      }

      pBc->AddBoth(pCr);
      PushBackASSP(pBc.GetRawPointer(), pBc->lsPpl, pCr);

      pBc->tutTwo.TraderGenerate();
    }
  }
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

  void ModifyTilt(Point trackball) {
    tilt -= tilt * naturalScaleFactor;
    tilt += trackball.x * trackballScaleFactor;

    if (tilt > maxTilt)
      tilt = maxTilt;
    if (tilt < -maxTilt)
      tilt = -maxTilt;
  }

  fPoint GetNewVelocity(Point trackball) {
    ModifyTilt(trackball);
    Polar p = Polar(lastVel);
    p.r = speed;
    p.a += tilt;
    lastVel = p.TofPoint();
    return lastVel;
  }
};

/** Set of key/button codes for input (e.g. fire, steer). */
struct ButtonSet {
  std::vector<int> vCodes;

  ButtonSet(int q, int w, int e, int d, int c, int x, int z, int a, int sp) {
    vCodes.push_back(q);
    vCodes.push_back(w);
    vCodes.push_back(e);
    vCodes.push_back(d);
    vCodes.push_back(c);
    vCodes.push_back(x);
    vCodes.push_back(z);
    vCodes.push_back(a);
    vCodes.push_back(sp);
  }

  bool IsSpace(int nCode) { return nCode == vCodes[8]; }

  Point GetPoint(int nCode) {
    Point p = Point();

    if (nCode == vCodes[0])
      p = Point(-1, -1);
    if (nCode == vCodes[1])
      p = Point(0, -1);
    if (nCode == vCodes[2])
      p = Point(1, -1);
    if (nCode == vCodes[3])
      p = Point(1, 0);
    if (nCode == vCodes[4])
      p = Point(1, 1);
    if (nCode == vCodes[5])
      p = Point(0, 1);
    if (nCode == vCodes[6])
      p = Point(-1, 1);
    if (nCode == vCodes[7])
      p = Point(-1, 0);

    return p;
  }
};

unsigned GetRandNum(unsigned nRange) {
  return unsigned(float(rand()) / (float(RAND_MAX) + 1) * nRange);
}

inline unsigned GetRandFromDistribution(std::vector<float> vProb) {
  float fSum = 0;

  for (unsigned i = 0; i < vProb.size(); ++i)
    fSum += vProb[i];

  float fRand = float(rand()) / (float(RAND_MAX) + 1) * fSum;

  fSum = 0;

  for (unsigned i = 0; i < vProb.size(); ++i) {
    fSum += vProb[i];
    if (fSum > fRand)
      return i;
  }

  throw std::string("bad range");
}

inline unsigned RandomBonus(bool bInTower /* = true*/) {
  std::vector<float> v;

  v.push_back(0.F);  // time
  v.push_back(1.2F); // pershot
  v.push_back(.5F);  // laser
  v.push_back(1.F);  // big
  v.push_back(1.F);  // total num
  v.push_back(.8F);  // explode
  v.push_back(1.F);  // split fireball
  v.push_back(.15F); // set on fire

  if (bInTower)
    v.push_back(1.F); // ring fireball
  else
    v.push_back(0.F);

  v.push_back(.1F); // nuke
  v.push_back(.8F); // speed
  v.push_back(0.F); // shooting frequency

  return GetRandFromDistribution(v);
}

unsigned GetFireballChainNum(FireballBonus &fb) {
  int nRet = fb.uMap["fireballchainnum"];

  if (nRet != 0)
    ++nRet;

  return nRet;
}

/*
unsigned GetFireballChainSplit(FireballBonus& fb)
{
        int nRet = fb.uMap["fireballchainnum"];
        if(fb.bMap["fireballchainnum_flag"])
                nRet += 2;
        return nRet;
}
*/

/** Player dragon: carries bonuses and fireballs, steer/shoot, collision with
 * units. */
struct Dragon : public Critter {
  std::list<ASSP<TimedFireballBonus>> lsBonuses;
  std::list<ASSP<Fireball>> lsBalls;

  DragonLeash leash;

  SP<TimedFireballBonus> GetBonus(unsigned n, unsigned nTime) {
    if (pAd->nLvl > 6)
      nTime = unsigned(nTime * fBonusTimeMutiplierTwo);
    else if (pAd->nLvl > 3)
      nTime = unsigned(nTime * fBonusTimeMutiplierOne);

    SP<TimedFireballBonus> pBonus;

    if (n == 0)
      pBonus =
          new TimedFireballBonus(FireballBonus(n, "regenerate", 2U), nTime * 2);
    else if (n == 1)
      pBonus = new TimedFireballBonus(FireballBonus(n, "pershot", 1U), nTime);
    else if (n == 2) {
      pBonus = new TimedFireballBonus(FireballBonus(n, false), nTime);
      pBonus->Add("through", 1U);
      pBonus->Add("laser", true);
    } else if (n == 3)
      pBonus = new TimedFireballBonus(FireballBonus(n, "big", 1U), nTime);
    else if (n == 4) {
      pBonus = new TimedFireballBonus(
          FireballBonus(n, "total", nFireballsPerBonus), nTime * 2);
      //++nExtraFireballs;
    } else if (n == 5)
      pBonus = new TimedFireballBonus(FireballBonus(n, "explode", 1U), nTime);
    else if (n == 6) {
      pBonus = new TimedFireballBonus(FireballBonus(n, false), nTime);
      pBonus->Add("fireballchainnum", 1U);
      pBonus->Add("through_flag", true);
    }
    /*
            else if(n == 7)
    {
        pBonus = new TimedFireballBonus(FireballBonus(n, false), nTime);
        pBonus->Add("fireballchain_flag", true);
        pBonus->Add("fireballchainnum_flag", true);
        pBonus->Add("fireballchain", 1U);
    }
            */
    else if (n == 7)
      pBonus = new TimedFireballBonus(FireballBonus(n, "setonfire", 1U), nTime);
    else if (n == 8) {
      FireballBonus fb = GetAllBonuses();

      // fb.fMap["speed"] = (4.F + fb.uMap["regenerate"]*2.F) * 10 /
      // nFramesInSecond;

      Point p = GetPosition();

      if (pCs != 0)
        p = pCs->GetPosition();

      int nNumCirc = fb.uMap["pershot"] + 1;

      fPoint fVel = RandomAngle();

      for (int i = 0; i < nNumCirc; ++i) {
        SP<CircularFireball> pFb = new CircularFireball(
            Fireball(p, GetWedgeAngle(fVel, 1.F, i, nNumCirc + 1), pAd, fb,
                     // Chain(true /*fb.uMap["fireballchain"]*/),
                     // fb.uMap["fireballchainnum"]), 40, nTime);
                     Chain(), GetFireballChainNum(fb)),
            35, nTime * 2);
        pAd->AddBoth(pFb);
      }

      pBonus = new TimedFireballBonus(FireballBonus(n, false), nTime * 2);
    } else if (n == 9) {
      CleanUp(pAd->lsPpl);

      nSlimeMax *= 2;

      for (std::list<ASSP<ConsumableEntity>>::iterator itr = pAd->lsPpl.begin(),
                                                       etr = pAd->lsPpl.end();
           itr != etr; ++itr)
        if ((*itr)->GetType() == 'K' || (*itr)->GetType() == 'S' ||
            (*itr)->GetType() == 'L') {
          if ((*itr)->GetType() == 'K' &&
              GetAllBonuses().uMap["setonfire"] != 0) {
            (*itr)->bExist = false;
            SP<KnightOnFire> pKn = new KnightOnFire(
                Critter((*itr)->GetRadius(), (*itr)->GetPosition(), fPoint(),
                        rBound, 1.F, ImageSequence(), true),
                pAd, 15 * nFramesInSecond,
                Chain(GetAllBonuses().uMap["setonfire"]));
            pAd->AddBoth(pKn);
          } else
            (*itr)->OnHit('F');
        }

      nSlimeMax /= 2;
    } else if (n == 10) {
      pBonus = new TimedFireballBonus(FireballBonus(n, "speed", 2.5F), nTime);
    } else {
      pBonus =
          new TimedFireballBonus(FireballBonus(n, "frequency", .5F), nTime);
    }

    return pBonus;
  }

  void FlushBonuses() {
    for (std::list<ASSP<TimedFireballBonus>>::iterator itr = lsBonuses.begin(),
                                                       etr = lsBonuses.end();
         itr != etr; ++itr)
      pAd->pGl->lsBonusesToCarryOver.push_back(*itr);
  }

  FireballBonus GetAllBonuses() {
    CleanUp(lsBonuses);
    FireballBonus fbRet(-1, true);

    for (std::list<ASSP<TimedFireballBonus>>::iterator itr = lsBonuses.begin(),
                                                       etr = lsBonuses.end();
         itr != etr; ++itr)
      fbRet += **itr;

    fbRet.uMap["total"] += nExtraFireballs;

    return fbRet;
  }

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

  Dragon(SP<Castle> pCs_, SP<AdvancedController> pAd_, ImageSequence imgStable_,
         ImageSequence imgFly_, ButtonSet bt_)
      : pAd(this, pAd_), imgStable(imgStable_), imgFly(imgFly_),
        Critter(13,
                pCs_ == 0 ? pAd_->vCs[0]->GetPosition() : pCs_->GetPosition(),
                Point(), pAd_->rBound, 1, ImageSequence()),
        bFly(), bCarry(false), cCarry(' '), nTimer(0), pCs(this, pCs_), bt(bt_),
        nFireballCount(0), tFireballRegen(1), bTookOff(false), nPrCr(0),
        nExtraFireballs(0), bRegenLocked(false),
        tRegenUnlock(nFramesInSecond * nRegenDelay / 10)

  {
    nFireballCount = GetAllBonuses().uMap["total"];

    if (pCs != 0 && pCs->pDrag == 0) {
      pCs->pDrag = this;
      bFly = false;
      Critter::dPriority = 3;
      Critter::fPos = pCs->GetPosition();
    } else {
      bFly = true;
      Critter::dPriority = 5;
      Critter::fPos = pAd->vCs[0]->GetPosition();
    }

    SimpleVisualEntity::seq = imgStable;
    Critter::bDieOnExit = false;

    pAd->pGl->lsBonusesToCarryOver.clear();
  }

  void RecoverBonuses() {
    for (std::list<SP<TimedFireballBonus>>::iterator
             itr = pAd->pGl->lsBonusesToCarryOver.begin(),
             etr = pAd->pGl->lsBonusesToCarryOver.end();
         itr != etr; ++itr) {
      AddBonus(*itr, true);
    }
  }

  /*vrtual*/ void Update() {
    // CleanUp(lsBonuses);

    if (bRegenLocked) {
      if (tRegenUnlock.Tick())
        bRegenLocked = false;
    } else {
      FireballBonus fb = GetAllBonuses();

      int nPeriod = nInitialRegen; // - fb.uMap["regenerate"];
      if (nPeriod < 2)
        nPeriod = 2;

      nPeriod = int(float(nPeriod) * nFramesInSecond / 10);

      tFireballRegen.Tick();

      if (int(tFireballRegen.nTimer) >= nPeriod) {
        tFireballRegen.nTimer = 0;

        if (nFireballCount < int(fb.uMap["total"])) {
          // pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("reload"));

          if (nInitialRegen ==
              0) // this is so 0 fireball regen will not be limited by framerate
            nFireballCount = int(fb.uMap["total"]);
          else
            ++nFireballCount;
        }
      }
    }

    if (bFly) {
      bool bHitCastle = false;

      for (unsigned i = 0; i < pAd->vCs.size(); ++i)
        if (this->HitDetection(pAd->vCs[i])) {
          if (pAd->vCs[i]->pDrag != 0)
            continue;
          bHitCastle = true;
          break;
        }

      if (bTookOff == true) {
        if (bHitCastle == false)
          bTookOff = false;
      } else {
        if (bHitCastle == true)
          Toggle();
      }
    }

    if (bFly && (!bCarry || cCarry == 'P')) {
      for (std::list<ASSP<ConsumableEntity>>::iterator itr = pAd->lsPpl.begin();
           itr != pAd->lsPpl.end(); ++itr) {
        if (!(*itr)->bExist)
          continue;

        if ((**itr).GetType() == 'P' && this->HitDetection(*itr)) {
          bCarry = true;
          imgCarry = (*itr)->GetImage();
          cCarry = 'P';
          ++nPrCr;

          (*itr)->bExist = false;

          pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("pickup"));
          break;
        }
      }
    }

    if (bFly) {
      CleanUp(pAd->lsBonus);

      for (std::list<ASSP<FireballBonusAnimation>>::iterator itr =
               pAd->lsBonus.begin();
           itr != pAd->lsBonus.end(); ++itr) {
        if (!(*itr)->bExist)
          continue;

        if (this->HitDetection(*itr)) {
          AddBonus(GetBonus((*itr)->n, nBonusPickUpTime));
          (*itr)->bExist = false;

          pAd->tutTwo.BonusPickUp();
        }
      }
    }

    Critter::Update();
  }

  /*vrtual*/ Point GetPosition() {
    if (pCs != 0)
      return (fPos + fPoint(0, -1)).ToPnt();
    return fPos.ToPnt();
  }

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    if (bCarry)
      pDr->Draw(imgCarry, GetPosition(), true);

    if (!bFly) {
      if (!nTimer)
        pDr->Draw(SimpleVisualEntity::seq.vImage[0],
                  pCs->GetPosition() - Point(0, 22));
      else
        pDr->Draw(SimpleVisualEntity::seq.vImage[1],
                  pCs->GetPosition() - Point(0, 22));
    } else {
      Polar p(Critter::fVel);
      pDr->Draw(imgFly.vImage[DiscreetAngle(p.a, 16)], GetPosition());
    }

    if (nTimer > 0)
      --nTimer;
  }

  void AddBonus(SP<TimedFireballBonus> pBonus, bool bSilent = false) {
    if (!bSilent)
      pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("powerup"));

    if (pBonus == 0)
      return;

    PushBackASSP(this, lsBonuses, pBonus);
    pAd->AddE(pBonus);
  }

  void Fire(fPoint fDir) {
    if (fDir == fPoint())
      return;

    CleanUp(lsBalls);

    FireballBonus fb(-1, true);
#ifndef FLIGHT_POWER_MODE
    if (!bFly)
#endif
      fb = GetAllBonuses();
    fb.fMap["speed"] *= fFireballSpeed;
    if (bFly)
      fb.fMap["speed"] += fDragonSpeed;

    // if(fb.uMap["total"] <= lsBalls.size())
    //    return;

    if (nFireballCount == 0)
      return;

    if (!bFly)
      pAd->tutOne.ShotFired();

    tFireballRegen.nTimer = 0;
    bRegenLocked = true;
    tRegenUnlock.nTimer = 0;

    --nFireballCount;

    nTimer = 4;

    unsigned nNumber = fb.uMap["pershot"];

    float fSpread = 1.F;

    if (nNumber > 1) {
      fSpread = nNumber * .05F;
      if (fSpread > .5F)
        fSpread = 1.F;
    }

    for (unsigned i = 0; i < nNumber; ++i) {
      Point pPos = GetPosition();
      if (!bFly)
        pPos += Point(-10, -25);
      else
        pPos += (fDir / fDir.Length() * 5.F).ToPnt();

      fPoint fShoot = fDir;
      if (nNumber > 1)
        fShoot = GetWedgeAngle(fDir, fSpread, i, nNumber);

      if (nNumber % 2 == 0 && (i == nNumber / 2 || i == nNumber / 2 - 1)) {
        fShoot = fDir;
        fPoint fNormalized = fDir / fDir.Length();
        if (i == nNumber / 2)
          pPos = Point(Crd(pPos.x + fNormalized.y * 6),
                       Crd(pPos.y - fNormalized.x * 6));
        else
          pPos = Point(Crd(pPos.x - fNormalized.y * 6),
                       Crd(pPos.y + fNormalized.x * 6));
      }

      SP<Fireball> pFb =
          new Fireball(pPos, fShoot, pAd, fb, Chain(), GetFireballChainNum(fb));
      pAd->AddBoth(pFb);
      if (i == nNumber / 2)
        PushBackASSP(this, lsBalls, pFb);
    }

    if (fb.bMap["laser"])
      pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("laser"));
    else
      pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("shoot"));
  }

  void Toggle() {
    if (!bFly) {
      pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("leave_tower"));

      bFly = true;
      bTookOff = true;

      pAd->tutOne.FlyOn();

      SimpleVisualEntity::seq = imgFly;
      SimpleVisualEntity::dPriority = 5;

      pCs->pDrag = 0;
      pCs = 0;

      fVel = pAd->pt.GetFlightDirection(
          GetPosition()); // GetDirection(GetPosition());

      if (fVel.Length() == 0)
        fVel = fPoint(0, -1);
      fVel.Normalize(leash.speed);

      return;
    }

    for (unsigned i = 0; i < pAd->vCs.size(); ++i)
      if (this->HitDetection(pAd->vCs[i])) {
        if (pAd->vCs[i]->pDrag != 0 || bTookOff || pAd->vCs[i]->bBroken)
          continue;

        pAd->pt.Off();

        bFly = false;

        pAd->tutOne.FlyOff();

        pCs = pAd->vCs[i];
        pCs->pDrag = this;

        if (cCarry == 'P') {
          pAd->tutOne.PrincessCaptured();
          pAd->vCs[i]->nPrincesses += nPrCr;

          unsigned j;
          for (j = 0; j < pAd->vCs.size(); ++j) {
            if (pAd->vCs[j]->nPrincesses < 4)
              break;
          }

          if (j != pAd->vCs.size()) {
            pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("princess_capture"));
          } else {
            // if(pAd->bGhostTime)
            //	pAd->pSc->plr.StopMusic();

            FlushBonuses();

            pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("win_level"));
            pAd->pGl->Next();
          }
        } else if (cCarry == 'T')
          AddBonus(GetBonus(RandomBonus(), nBonusTraderTime));
        else
          pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("return_tower"));

        bCarry = false;
        cCarry = ' ';
        nPrCr = 0;

        SimpleVisualEntity::dPriority = 3;

        SimpleVisualEntity::seq = imgStable;
        Critter::fPos = pAd->vCs[i]->GetPosition();
        Critter::fVel = Point();

        return;
      }

    CleanUp(pAd->lsPpl);

    // disables dropping when carrying princesses
    // if(bCarry && cCarry == 'P')
    //	return;

    // disable dropping altogether
    if (bCarry)
      return;

    for (std::list<ASSP<ConsumableEntity>>::iterator itr = pAd->lsPpl.begin();
         itr != pAd->lsPpl.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      // only pick up traders
      if ((**itr).GetType() != 'T')
        continue;

      if (this->HitDetection(*itr)) {
        /*
if((**itr).GetType() == 'G')
                continue;
        if((**itr).GetType() == 'W')
                continue;
        if((**itr).GetType() == 'E')
                continue;

        if((**itr).GetType() == 'M')
        {
                SummonSkeletons(pAd, GetPosition());
        }
*/

        if (!bCarry) {
          bCarry = true;
          imgCarry = (*itr)->GetImage();
          cCarry = (*itr)->GetType();

          pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("pickup"));
        } else {
          throw SimpleException("not supposed to drop things");
          /*
if(cCarry == 'T' || (**itr).GetType() == 'T')
                  AddBonus(GetBonus(RandomBonus(false), nBonusPickUpTime));
          else
          {
                  if(cCarry == 'K')
                          pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("knight_fall"));
                  else
                          pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("dropping"));
          }

bCarry = false;
cCarry = ' ';
          nPrCr = 0;
*/
        }

        (*itr)->bExist = false;

        return;
      }
    }
    /*
    if(bCarry)
    {
        if(cCarry == 'T')
                            AddBonus(GetBonus(RandomBonus(false),
    nBonusPickUpTime)); else
                            pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("dropping"));

        bCarry = false;
        cCarry = ' ';
                    nPrCr = 0;
    }
    */
  }
};

/** BasicController that advances (Next) when only background is left or on
 * input. */
struct AlmostBasicController : public BasicController {
  AlmostBasicController(const BasicController &b) : BasicController(b) {}

  /*virtual*/ void Update() {
    CleanUp(lsUpdate);
    CleanUp(lsDraw);

    if (lsDraw.size() == 1) {
      pGl->Next();
      return;
    }

    BasicController::Update();
  }

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp) {
    if (!bUp) // && c == '\\')
      pGl->Next();
  }

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
