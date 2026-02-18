#ifndef TOWER_DEFENSE_DRAW_UTILS_H
#define TOWER_DEFENSE_DRAW_UTILS_H

/* draw_utils.h - Drawing helpers: Drawer base, ScalingDrawer for scaled
 * images with color key, NumberDrawer for font-based digits and text.
 *
 * Dependencies (include required): SP_Info -> utils/smart_pointer.h;
 * Color -> wrappers/color.h; Point -> wrappers/geometry.h;
 * Index -> utils/index.h; smart_pointer -> utils/smart_pointer.h. */

#include "utils/index.h"
#include "utils/smart_pointer.h"
#include "wrappers/color.h"
#include "wrappers/geometry.h"

class FilePath;
template <class T> class GraphicalInterface;

/** Base for drawing an image at a point; ScalingDrawer adds scale and color
 * key. */
struct Drawer : virtual public SP_Info {
  GraphicalInterface<Index> *pGr;

  Drawer() : pGr(nullptr) {}
  std::string get_class_name() override { return "Drawer"; }
  virtual void Draw(Index nImg, Point p, bool bCentered = true) = 0;
};

/** Drawer that scales images by nFactor and uses cTr as transparency key. */
struct ScalingDrawer : public Drawer {
  std::string get_class_name() override { return "ScalingDrawer"; }
  unsigned nFactor;
  Color cTr;

  ScalingDrawer(GraphicalInterface<Index> *pGr_,
                unsigned nFactor_,
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
  std::string get_class_name() override { return "NumberDrawer"; }
  smart_pointer<ScalingDrawer> pDr;
  std::vector<int> vImgIndx;
  std::vector<Index> vImg;

  std::map<Color, std::vector<Index>> mpCachedRecolorings;

  void CacheColor(Color c);

  NumberDrawer(smart_pointer<ScalingDrawer> pDr_, FilePath *fp,
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

#endif
