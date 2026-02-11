#ifndef TOWER_DEFENSE_CORE_H
#define TOWER_DEFENSE_CORE_H

#include "common.h"

namespace Gui {
class FilePath;
}

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

fPoint ComposeDirection(int dir1, int dir2);

fPoint GetWedgeAngle(fPoint fDir, float dWidth, unsigned nWhich,
                     unsigned nHowMany);

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

  ScalingDrawer(smart_pointer<Graphic> pGr_, unsigned nFactor_,
                Color cTr_ = Color(0, 255, 255))
      : nFactor(nFactor_), cTr(cTr_) {
    pGr = pGr_;
  }

  void Scale(Index &pImg, int nFactor_ = -1);

  /*virtual*/ void Draw(Index nImg, Point p, bool bCentered = true);

  /*virtual*/ Index LoadImage(std::string strFile);
};

/** Draws digits/words from a font bitmap; CacheColor/DrawColorWord for
 * recolored text. */
struct NumberDrawer : virtual public SP_Info {
  SSP<ScalingDrawer> pDr;
  std::vector<int> vImgIndx;
  std::vector<Index> vImg;

  std::map<Color, std::vector<Index>> mpCachedRecolorings;

  void CacheColor(Color c);

  NumberDrawer(smart_pointer<ScalingDrawer> pDr_, Gui::FilePath *fp,
               std::string sFontPath, std::string sFontName);

  std::string GetNumber(unsigned n, unsigned nDigits = 0);

  void DrawNumber(unsigned n, Point p, unsigned nDigits = 0) {
    DrawWord(GetNumber(n, nDigits), p);
  }

  void DrawColorNumber(unsigned n, Point p, Color c, unsigned nDigits = 0) {
    DrawColorWord(GetNumber(n, nDigits), p, c);
  }

  void DrawWord(std::string s, Point p, bool bCenter = false);

  void DrawColorWord(std::string s, Point p, Color c, bool bCenter = false);
};

/** Wraps Soundic and gates playback on bSoundOn (Toggle/Get). */
class SoundInterfaceProxy : virtual public SP_Info {
  bool bSoundOn;
  SSP<Soundic> pSndRaw;

public:
  SoundInterfaceProxy(smart_pointer<Soundic> pSndRaw_)
      : pSndRaw(this, pSndRaw_), bSoundOn(true) {}

  void PlaySound(Index i, int nChannel = -1, bool bLoop = false);

  void Toggle() { bSoundOn = !bSoundOn; }
  bool Get() { return bSoundOn; }
};

std::vector<std::string> BreakUpString(std::string s);

inline ImageSequence Reset(ImageSequence imgSeq) {
  imgSeq.nActive = 0;
  return imgSeq;
}

inline unsigned DiscreetAngle(float a, unsigned nDiv) {
  return unsigned((-a / 2 / 3.1415 + 2 - 1.0 / 4 + 1.0 / 2 / nDiv) * nDiv) %
         nDiv;
}

inline Point Center(Size sz) { return Point(sz.x / 2, sz.y / 2); }

inline int GetRandTimeFromRate(float fRate) {
  return 1 + int((float(rand()) / RAND_MAX * 1.5 + .25) * fRate);
}

unsigned GetRandNum(unsigned nRange);
unsigned GetRandFromDistribution(std::vector<float> vProb);

struct TowerDataWrap;
struct MenuController;

/** Plays level music from vThemes; SwitchTheme/StopMusic/ToggleOff control
 * playback. */
struct BackgroundMusicPlayer {
  int nCurrTheme;
  std::vector<Index> vThemes;
  smart_pointer<Soundic> pSnd;

  bool bOff;

  BackgroundMusicPlayer() : nCurrTheme(-1), pSnd(), bOff(false) {}

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

struct TwrGlobalController;
struct GameController;

extern const std::string sFullScreenPath;

#endif
