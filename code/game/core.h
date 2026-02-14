#ifndef TOWER_DEFENSE_CORE_H
#define TOWER_DEFENSE_CORE_H

#include "common.h"
#include "game_utils/draw_utils.h"
#include "game_utils/image_sequence.h"
#include "game_utils/sound_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/color.h"

class FilePath;

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

struct DragonGameControllerList;
struct GameController;

extern const std::string sFullScreenPath;

#endif
