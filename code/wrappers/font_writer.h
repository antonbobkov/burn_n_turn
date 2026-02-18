/*
 * font_writer.h - FontWriter: renders text using a font (FilePath + name),
 * bitmap symbols, and a GraphicalInterface<Index>; GetSize, DrawWord/
 * DrawColorWord, Recolor, SetGap.
 */

#ifndef TOWER_DEFENSE_FONT_WRITER_H
#define TOWER_DEFENSE_FONT_WRITER_H

#include "GuiGen.h"
#include "color.h"

struct FontWriter {
  std::vector<int> vImgIndx;
  std::vector<Index> vImg;
  GraphicalInterface<Index> *pGr;

  Size szSymbol;
  Color clSymbol;
  int nGap;

  FontWriter(FilePath *fp, std::string sFont,
             GraphicalInterface<Index> *pGr_, unsigned nZoom = 1);

  Size GetSize(std::string s);

  std::string GetNumber(int n, unsigned nDigits = 0);

  // NOTE: bug - colored words have non-transparent background
  void DrawColorWord(std::string s, Point p, Color c = Color(0, 0, 0, 0),
                     bool bCenter = false, bool bRefresh = false);

  void DrawWord(std::string s, Point p, bool bCenter = false,
                bool bRefresh = false) {
    DrawColorWord(s, p, Color(0, 0, 0, 0), bCenter, bRefresh);
  }

  void Recolor(Color c);
  void SetGap(int nG);
};

#endif
