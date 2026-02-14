#include "game_utils/draw_utils.h"
#include "utils/file_utils.h"
#include "wrappers/GuiGen.h"
#include "wrappers/color.h"

#include <algorithm>

void ScalingDrawer::Scale(Index &pImg, int nFactor_) {
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

void ScalingDrawer::Draw(Index nImg, Point p, bool bCentered) {
  p.x *= nFactor;
  p.y *= nFactor;
  if (bCentered) {
    Size sz = pGr->GetImage(nImg)->GetSize();
    p.x -= sz.x / 2;
    p.y -= sz.y / 2;
  }

  pGr->DrawImage(p, nImg, false);
}

Index ScalingDrawer::LoadImage(std::string strFile) {
  Index n = pGr->LoadImage(strFile);
  pGr->GetImage(n)->ChangeColor(Color(0, 0, 0), Color(0, 0, 0, 0));
  return n;
}

void NumberDrawer::CacheColor(Color c) {
  std::vector<Index> vNewColors;

  for (size_t i = 0, sz = vImg.size(); i < sz; ++i) {
    Index vColImg = pDr->pGr->CopyImage(vImg[i]);
    pDr->pGr->GetImage(vColImg)->ChangeColor(Color(255, 255, 0), c);
    vNewColors.push_back(vColImg);
  }

  mpCachedRecolorings[c] = vNewColors;
}

NumberDrawer::NumberDrawer(smart_pointer<ScalingDrawer> pDr_, FilePath *fp,
                          std::string sFontPath, std::string sFontName)
    : pDr(pDr_), vImgIndx(256, -1) {
  std::string txtPath = fp->GetRelativePath(sFontPath + sFontName + ".txt");
  std::unique_ptr<InStreamHandler> ih = fp->ReadFile(txtPath);
  std::istream &ifs = ih->GetStream();

  unsigned n;
  unsigned char c;
  for (n = 0; c = ifs.get(), !ifs.fail(); ++n)
    vImgIndx[c] = n;

  std::string bmpPath = fp->GetRelativePath(sFontPath + sFontName + ".bmp");
  Index nImg = pDr->LoadImage(bmpPath);
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

std::string NumberDrawer::GetNumber(unsigned n, unsigned nDigits) {
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

void NumberDrawer::DrawWord(std::string s, Point p, bool bCenter) {
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

void NumberDrawer::DrawColorWord(std::string s, Point p, Color c,
                                 bool bCenter) {
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
