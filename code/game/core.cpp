#include "core.h"

Polar::Polar(fPoint p) : r(p.Length()) {
  if (p.y == 0 && p.x == 0)
    a = 0;
  else
    a = atan2(p.y, p.x);
}

fPoint ComposeDirection(int dir1, int dir2) {
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

fPoint GetWedgeAngle(fPoint fDir, float dWidth, unsigned nWhich,
                     unsigned nHowMany) {
  if (nHowMany == 1)
    return fDir;

  float d = 3.1415F * 2 * dWidth / (nHowMany - 1) * nWhich;

  return (Polar(fDir) * Polar(d - 3.1415F * dWidth, 1)).TofPoint();
}

fPoint RandomAngle(fPoint fDir, float fRange) {
  return (Polar(fDir) *
          Polar((float(rand()) / RAND_MAX - .5F) * fRange * 2 * 3.1415F, 1))
      .TofPoint();
}

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

NumberDrawer::NumberDrawer(SP<ScalingDrawer> pDr_, std::string sFontPath,
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

void SoundInterfaceProxy::PlaySound(Index i, int nChannel, bool bLoop) {
  if (bSoundOn)
    pSndRaw->PlaySound(i, nChannel, bLoop);
}

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

unsigned GetRandNum(unsigned nRange) {
  return unsigned(float(rand()) / (float(RAND_MAX) + 1) * nRange);
}

unsigned GetRandFromDistribution(std::vector<float> vProb) {
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
