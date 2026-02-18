#include "font_writer.h"

#include "GuiGen.h"
#include "color.h"
#include "utils/file_utils.h"

#include <fstream>
#include <sstream>

FontWriter::FontWriter(FilePath *fp, std::string sFont,
                       GraphicalInterface<Index> *pGr_,
                       unsigned nZoom)
    : vImgIndx(256, -1), pGr(pGr_) {
  sFont = fp->GetRelativePath(sFont);

  std::string sFontPath;
  std::string sFontName = sFont;

  Separate(sFontName, sFontPath);

  std::ifstream ifs(sFont.c_str());

  std::string sFontPicture;
  std::getline(ifs, sFontPicture);

  sFontPicture = fp->Format(sFontPicture);

  Color cTransp;
  int na, nb, nc;

  ifs >> na >> nb >> nc;
  cTransp.R = na;
  cTransp.G = nb;
  cTransp.B = nc;
  ifs >> na >> nb >> nc;
  clSymbol.R = na;
  clSymbol.G = nb;
  clSymbol.B = nc;

  ifs >> szSymbol.x >> szSymbol.y >> nGap;

  while (ifs.get() != '\n' && !ifs.fail())
    ;

  szSymbol.x *= nZoom;
  szSymbol.y *= nZoom;
  nGap *= nZoom;

  unsigned n;
  unsigned char c;
  for (n = 0; c = ifs.get(), !ifs.fail(); ++n)
    vImgIndx[c] = n;

  Index nImg = pGr->LoadImage(sFontPath + sFontPicture);
  nImg = pGr->ScaleImage(nImg, nZoom);
  Image *pImg = pGr->GetImage(nImg);
  pImg->ChangeColor(cTransp, Color(0, 0, 0, 0));

  for (unsigned i = 0; i < n; ++i) {
    Index nCurr = pGr->GetBlankImage(szSymbol);
    Image *pCurr = pGr->GetImage(nCurr);

    Point p;
    for (p.y = 0; p.y < szSymbol.y; ++p.y)
      for (p.x = 0; p.x < szSymbol.x; ++p.x)
        pCurr->SetPixel(
            p, pImg->GetPixelSafe(Point(i * (szSymbol.x + nGap) + p.x, p.y)));
    vImg.push_back(nCurr);
  }
}

std::string FontWriter::GetNumber(int n, unsigned nDigits /* = 0*/) {
  std::string s;
  std::ostringstream ostr(s);
  ostr << n;
  std::string sRes = ostr.str();
  if (sRes.length() >= nDigits)
    return sRes;
  return std::string(nDigits - sRes.length(), '0') + sRes;
}

Size FontWriter::GetSize(std::string s) {
  if (s.length() == 0)
    return Size(0, szSymbol.y);

  return Size(szSymbol.x * s.length() + nGap * (s.length() - 1), szSymbol.y);
}

void FontWriter::DrawColorWord(std::string s, Point p,
                               Color c /* = Color(0,0,0,0)*/,
                               bool bCenter /* = false*/,
                               bool bRefresh /* = false*/) {
  if (bCenter) {
    Size szWord = GetSize(s);
    p.x -= szWord.x / 2;
    p.y -= szWord.y / 2;
  }

  for (unsigned i = 0; i < s.length(); ++i) {
    int n = int(s[i]);

    if (vImgIndx[n] == -1)
      continue;

    Point pnt(p.x + (szSymbol.x + nGap) * i, p.y);
    if (c.nTransparent == 0)
      pGr->DrawImage(pnt, vImg[vImgIndx[n]], bRefresh);
    else {
      Index vColImg = pGr->CopyImage(vImg[vImgIndx[n]]);
      pGr->GetImage(vColImg)->ChangeColor(clSymbol, c);
      pGr->DrawImage(pnt, vColImg, bRefresh);
    }
  }
}

void FontWriter::Recolor(Color c) {
  for (unsigned i = 0; i < vImg.size(); ++i)
    pGr->GetImage(vImg[i])->ChangeColor(clSymbol, c);
  clSymbol = c;
}

void FontWriter::SetGap(int nG) { nGap = nG; }
