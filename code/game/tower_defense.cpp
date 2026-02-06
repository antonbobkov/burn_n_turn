#include "tower_defense.h"

MessageWriter *pWr = 0;

int nSlimeMax = 100;

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

void BackgroundMusicPlayer::ToggleOff() {
  if (!bOff) {
    bOff = true;
    StopMusic();
  } else {
    bOff = false;
  }
}

void SoundInterfaceProxy::PlaySound(Index i, int nChannel, bool bLoop) {
  if (bSoundOn)
    pSndRaw->PlaySound(i, nChannel, bLoop);
}

SimpleController::SimpleController(SP<TwrGlobalController> pGraph,
                                   std::string strFileName)
    : GameController(pGraph) {
  nImage = pGl->pGraph->LoadImage(strFileName);
}

SimpleController::~SimpleController() { pGl->pGraph->DeleteImage(nImage); }

void SimpleController::Update() {
  pGl->pGraph->DrawImage(Gui::Point(0, 0), nImage);
}

void SimpleController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;

  pGl->Next();
}

FlashingController::FlashingController(SP<TwrGlobalController> pGraph,
                                       std::string strFileName,
                                       std::string strTextName)
    : GameController(pGraph), nTimer(0), bShow(true) {
  nImage = pGl->pGraph->LoadImage(strFileName);
  nText = pGl->pGraph->LoadImage(strTextName);

  pGl->pGraph->GetImage(nText)->ChangeColor(Color(255, 255, 255),
                                            Color(0, 0, 0, 0));
}

FlashingController::~FlashingController() {
  pGl->pGraph->DeleteImage(nImage);
  pGl->pGraph->DeleteImage(nText);
}

void FlashingController::Update() {
  ++nTimer;
  if (nTimer % nFramesInSecond == 0)
    bShow = !bShow;

  pGl->pGraph->DrawImage(Point(0, 0), nImage, false);
  if (bShow)
    pGl->pGraph->DrawImage(Point(0, 0), nText, false);
  pGl->pGraph->RefreshAll();
}

void FlashingController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;

  pGl->Next();
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

std::string OnOffString(bool b) {
  if (b)
    return "on";
  else
    return "off";
}

void TextDrawEntity::Draw(SP<ScalingDrawer> pDr) {
  Point p = pos;
  for (unsigned i = 0; i < vText.size(); ++i) {
    pNum->DrawWord(vText[i], p, bCenter);
    p.y += 7;
  }
}

void TutorialTextEntity::SetText(std::vector<std::string> v) {
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

void TutorialTextEntity::Draw(SP<ScalingDrawer> pDr) {
  if (pIsTutorialOn && (*pIsTutorialOn == false))
    return;

  for (unsigned i = 0; i < sText.size(); ++i) {
    Point p = pos;
    p.y -= nOffset;
    p.y += nTextVerticalSpacing * i + nTextVerticalSpacing / 3;
    pNum->DrawWord(sText[i], p, true);
  }
}

void TutorialTextEntity::Update() {
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

std::vector<std::string> TutorialLevelOne::GetText() {
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
}

void TutorialLevelOne::Update() {
  if (pTexter != 0)
    pTexter->SetText(GetText());
}

void TutorialLevelOne::KnightKilled() {
  if (bKilledKnight == false) {
    bKilledKnight = true;
    Update();
  }
}

void TutorialLevelOne::FlyOn() {
  bFlying = true;
  Update();
}

void TutorialLevelOne::FlyOff() {
  bFlying = false;
  Update();
}

void TutorialLevelOne::PrincessGenerate() {
  bPrincessGenerated = true;
  Update();
}

void TutorialLevelOne::PrincessCaptured() {
  bPrincessCaptured = true;
  Update();
}

std::vector<std::string> TutorialLevelTwo::GetText() {
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
}

void TutorialLevelTwo::Update() {
  if (pTexter != 0)
    pTexter->SetText(GetText());
}

void TutorialLevelTwo::TraderKilled() {
  if (bTraderKilled == false) {
    bTraderKilled = true;
    Update();
  }
}

void TutorialLevelTwo::TraderGenerate() {
  bTraderGenerated = true;
  Update();
}

void TutorialLevelTwo::BonusPickUp() {
  bBonusPickedUp = true;
  Update();
}

void SimpleVisualEntity::Update() {
  if (bTimer) {
    if (t.Tick()) {
      seq.Toggle();
      t = Timer(nPeriod * seq.GetTime());
    }
  } else if (bStep) {
    Point p = GetPosition();
    if (p != pPrev) {
      bImageToggle = !bImageToggle;

      if (bImageToggle)
        seq.Toggle();
    }
    pPrev = p;
  }
}

void SimpleVisualEntity::Draw(SP<ScalingDrawer> pDr) {
  pDr->Draw(seq.GetImage(), GetPosition(), bCenter);
}

void StaticImage::Draw(SP<ScalingDrawer> pDr) {
  pDr->Draw(img, GetPosition(), bCentered);
}

void StaticRectangle::Draw(SP<ScalingDrawer> pDr) {
  pDr->pGr->DrawRectangle(r, c, false);
}

void SimpleSoundEntity::Update() {
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

void AnimationOnce::Update() {
  SimpleVisualEntity::Update();

  if (SimpleVisualEntity::seq.nActive == 0) {
    if (!bOnce)
      bExist = false;
  } else {
    bOnce = false;
  }
}

void MenuDisplay::Draw(SP<ScalingDrawer> pDr) {
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

void MenuDisplay::OnMouseMove(Point pMouse) {
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

void Countdown::Update() {
  ++nCount;
  if (nCount % nFramesInSecond == 0)
    --nTime;
  if (nTime == 0)
    bExist = false;
}

void Countdown::Draw(SP<ScalingDrawer> pDr) {
  pNum->DrawNumber(nTime, Point(22, 2));
}

bool PhysicalEntity::HitDetection(SP<PhysicalEntity> pPh) {
  Point d = GetPosition() - pPh->GetPosition();
  unsigned r1 = GetRadius(), r2 = pPh->GetRadius();
  return unsigned(d.x * d.x + d.y * d.y) < (r1 * r1 + r2 * r2);
}

TrackballTracker::TrackballTracker() {
  nMaxLength = 3;
  for (int i = 0; i < nMaxLength; i++)
    lsMouse.push_back(Point(0, 0));
  trigFlag = false;
  threshold = 25;
}

void TrackballTracker::Update() {
  Point p = mtr.GetRelMovement();

  lsMouse.push_front(p);
  lsMouse.pop_back();
}

bool TrackballTracker::IsTrigger() {
  std::vector<int> v;
  for (std::list<Point>::iterator itr = lsMouse.begin(); v.size() < 3; ++itr)
    v.push_back(int(fPoint(*itr).Length()));

  return (v[1] > v[0]) && (v[1] > v[2]);
}

fPoint TrackballTracker::GetAvMovement() {
  fPoint p;
  for (std::list<Point>::iterator itr = lsMouse.begin(), etr = lsMouse.end();
       itr != etr; ++itr)
    p += *itr;

  p.x /= lsMouse.size();
  p.y /= lsMouse.size();

  return p;
}

int TrackballTracker::GetDerivative() {
  if (nMaxLength > 1)
    return GetLengthSq(lsMouse.front()) - GetLengthSq(*++lsMouse.begin());
  else
    return 0;
}

void Critter::Move() {
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

void FancyCritter::Move() {
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

Cutscene::Cutscene(SP<TwrGlobalController> pGl_, Rectangle rBound_,
                   std::string sRun, std::string sChase, bool bFlip)
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

void Cutscene::Update() {
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

void Cutscene::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp && c == '\\' && pGl->sbCheatsOn.Get())
    pGl->Next();
}

FireballBonus::FireballBonus(int nNum_, bool bDef) : nNum(nNum_) {
  if (bDef) {
    fMap["speed"] = 5;
    fMap["frequency"] = fInitialFrequency;
    uMap["pershot"] = 1;
    uMap["total"] = nInitialFireballs;
  }
}

FireballBonus &FireballBonus::operator+=(const FireballBonus &f) {
  Union(fMap, f.fMap);
  Union(uMap, f.uMap);
  Union(bMap, f.bMap);

  return *this;
}

std::ostream &operator<<(std::ostream &ofs, FireballBonus b) {
  Out(ofs, b.fMap) << "\n";
  Out(ofs, b.uMap) << "\n";
  Out(ofs, b.bMap) << "\n";

  return ofs;
}

Chain Chain::Evolve() {
  if (bInfinite)
    return Chain(true);
  else if (nGeneration == 0)
    return Chain();
  else
    return Chain(nGeneration - 1);
}

void ChainExplosion::Update() {
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

void KnightOnFire::RandomizeVelocity() {
  fVel = RandomAngle();
  fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fKnightFireSpeed);
}

BonusScore::BonusScore(SP<AdvancedController> pAc_, Point p_, unsigned nScore_)
    : p(p_ + Point(0, -5)), t(unsigned(.1F * nFramesInSecond)), nC(0),
      pAc(this, pAc_), c(255, 255, 0), nScore(nScore_), nScoreSoFar(0) {
  std::ostringstream ostr(sText);
  ostr << '+' << nScore;
  sText = ostr.str();
}

void SoundControls::Update() {
  if (nTheme != -1)
    plr.SwitchTheme(nTheme);
  else
    plr.StopMusic();
}

void PositionTracker::On() {
  bPressed = true;
  nCounter = 0;
}

int PositionTracker::Off() {
  bPressed = false;
  return nCounter;
}

void PositionTracker::Update() {
  if (bPressed)
    ++nCounter;
}

void PositionTracker::UpdateMouse(Point pMouse_) {
  pMouse = pMouse_;
  pMouse.x /= 2;
  pMouse.y /= 2;
}

void PositionTracker::UpdateLastDownPosition(Point pMouse_) {
  pLastDownPosition = pMouse_;
  pLastDownPosition.x /= 2;
  pLastDownPosition.y /= 2;
}

fPoint PositionTracker::GetFlightDirection(fPoint fDragonPos) {
  return pLastDownPosition - fDragonPos;
}

AdvancedController::AdvancedController(const AdvancedController &a)
    : BasicController(a), tr(a.tr), t(a.t), bCh(a.bCh), nLvl(a.nLvl),
      nSlimeNum(a.nSlimeNum), bPaused(a.bPaused), tLoseTimer(a.tLoseTimer),
      bGhostTime(a.bGhostTime), tBlink(a.tBlink), bBlink(a.bBlink), pGr(a.pGr),
      tStep(a.tStep), bLeft(a.bLeft), pSc(a.pSc), bLeftDown(a.bLeftDown),
      bRightDown(a.bRightDown), nLastDir(a.nLastDir),
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

void DragonScoreController::OnMouseDown(Point pPos) {
  if (bClickToExit)
    pGl->Next();
}

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

Fireball::Fireball(Point p, fPoint v, SP<AdvancedController> pBc_,
                   FireballBonus &fb_, Chain ch_, unsigned nChain_)
    : Critter(GetFireballRaduis(fb_), p, v, pBc_->rBound, 5.F, ImageSequence(),
              nFramesInSecond / 10),
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

void Fireball::Update() {
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
        bool bKeepGoing = (fb.uMap["through"] != 0) || fb.bMap["through_flag"];

        if (nChain != 0 || !bKeepGoing)
          bExist = false;

        if (bKeepGoing) {
          if (fb.bMap["through_flag"])
            fb.bMap["through_flag"] = false;
          else
            --fb.uMap["through"];
        }

        if (bKeepGoing) {
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

void TimedFireballBonus::Update() {
  if (t.Tick())
    bExist = false;
}

void CircularFireball::Update() {
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

void Road::Draw(SP<ScalingDrawer> pDr) {
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

void Road::RoadMap(Point &p, Point &v) {
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

void FancyRoad::Draw(SP<ScalingDrawer> pDr) {
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

void Princess::OnHit(char cWhat) {
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

void Princess::Draw(SP<ScalingDrawer> pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
#ifdef UNDERLINE_UNIT_TEXT
  if (sUnderText != "")
    pAc->pGl->pNum->DrawWord(sUnderText, p, true);
#endif
}

SkellyGenerator::SkellyGenerator(Point p_, SP<AdvancedController> pAdv_)
    : p(p_), t(unsigned(.7F * nFramesInSecond)), pAdv(this, pAdv_) {
  SP<AnimationOnce> pSlm =
      new AnimationOnce(2.F, pAdv->pGl->pr("skelly_summon"),
                        unsigned(.1F * nFramesInSecond), p_, true);
  pAdv_->AddBoth(pSlm);
}

Mage::Mage(const Critter &cr, SP<AdvancedController> pAc_, bool bAngry_)
    : Critter(cr), pAc(this, pAc_), bAngry(bAngry_), bCasting(false),
      tUntilSpell(GetTimeUntillSpell()), tSpell(3 * nFramesInSecond),
      tSpellAnimate(unsigned(.7F * nFramesInSecond)) {
  fMvVel = Critter::fVel;

  bAngry = true;
  pAc->pGl->bAngry = true;
}

void Mage::OnHit(char cWhat) {
  bExist = false;

  SP<AnimationOnce> pAn = new AnimationOnce(
      GetPriority(),
      fVel.x < 0 ? pAc->pGl->pr("mage_die_f") : pAc->pGl->pr("mage_die"),
      unsigned(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true);

  pAc->AddBoth(pAn);

  pAc->pGl->bAngry = true;

  if (pAc->nLvl > 6)
    SummonSlimes();
}

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

FireballBonusAnimation::FireballBonusAnimation(Point p_, unsigned n_,
                                               SP<AdvancedController> pAd_)
    : Animation(.5F, ImageSequence(), nFramesInSecond / 10, p_, true), n(n_),
      bBlink(false), pAd(this, pAd_), tm(nBonusOnGroundTime), sUnderText("") {
  seq = GetBonusImage(n, pAd->pGl->pr);
  coronaSeq = pAd->pGl->pr("corona");
}

void FireballBonusAnimation::Draw(SP<ScalingDrawer> pDr) {
  pDr->Draw(coronaSeq.GetImage(), GetPosition(), bCenter);
  Animation::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;

#ifdef UNDERLINE_UNIT_TEXT
  if (sUnderText != "")
    pAd->pGl->pNum->DrawWord(sUnderText, p, true);
#endif
}

void FireballBonusAnimation::Update() {
  coronaSeq.Toggle();
  Animation::Update();

  if (tm.Tick())
    bExist = false;

  if (!bBlink && tm.nPeriod && (tm.nPeriod - tm.nTimer) < 7 * nFramesInSecond) {
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

void Trader::OnHit(char cWhat) {
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

void Trader::Draw(SP<ScalingDrawer> pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
#ifdef UNDERLINE_UNIT_TEXT
  if (sUnderText != "")
    pAc->pGl->pNum->DrawWord(sUnderText, p, true);
#endif
}

void Knight::Draw(SP<ScalingDrawer> pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
#ifdef UNDERLINE_UNIT_TEXT
  if (sUnderText != "")
    pAc->pGl->pNum->DrawWord(sUnderText, p, true);
#endif
}

void Knight::KnockBack() {
  if (fVel != fPoint(0, 0))
    fPos -= fVel / fVel.Length();
}

void Knight::Update() {
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

    if (bImageToggle) {
      seq.Toggle();

      if (seq.nActive == 3)
        pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("step_left"));
      else if (seq.nActive == 6)
        pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("step_right"));
    }
  }
  pPrev = p;
}

MegaSlime::MegaSlime(fPoint fPos, Rectangle rBound, SP<AdvancedController> pAc_)
    : Critter(8, fPos, fPoint(0, 0), rBound, 3, pAc_->pGl->pr("megaslime"),
              nFramesInSecond / 5),
      pAc(this, pAc_), nHealth(nSlimeHealthMax) {
  bDieOnExit = false;
}

void MegaSlime::RandomizeVelocity() {
  fVel = RandomAngle();

  if (rand() % 7 == 0)
    fVel = fPoint(rBound.sz.x / 2, rBound.sz.y / 2) - fPoint(GetPosition());

  fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fSlimeSpeed);
}

void MegaSlime::Update() {
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
}

Ghostiness::Ghostiness(Point p_, SP<AdvancedController> pAdv_, Critter knCp_,
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

Slime::Slime(fPoint fPos, Rectangle rBound, SP<AdvancedController> pAc_,
             int nGeneration_)
    : Critter(5, fPos, fPoint(0, 0), rBound, 3, pAc_->pGl->pr("slime"), true),
      pAc(this, pAc_), t(nFramesInSecond / 2), nGeneration(nGeneration_) {
  RandomizeVelocity();
  ++pAc->nSlimeNum;
}

void Slime::RandomizeVelocity() {
  fVel = RandomAngle();

  if (rand() % 7 == 0)
    fVel = fPoint(rBound.sz.x / 2, rBound.sz.y / 2) - fPoint(GetPosition());

  fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fSlimeSpeed);
}

Slime::~Slime() {
  if (pAc != 0) {
    --pAc->nSlimeNum;
  }
}

Sliminess::Sliminess(Point p_, SP<AdvancedController> pAdv_, bool bFast_,
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

void Sliminess::Update() {
  if (t.Tick()) {
    bExist = false;

    SP<Slime> pSlm = new Slime(p, pAdv->rBound, pAdv, nGeneration);
    pAdv->AddBoth(pSlm);
    PushBackASSP(pAdv.GetRawPointer(), pAdv->lsPpl, pSlm);
    PushBackASSP(pAdv.GetRawPointer(), pAdv->lsSlimes, pSlm);
  }
}

void Sliminess::Kill() {
  bExist = false;
  pSlm->bExist = false;
}

Sliminess::~Sliminess() {
  if (pAdv != 0)
    --pAdv->nSlimeNum;
}

MegaSliminess::MegaSliminess(Point p_, SP<AdvancedController> pAdv_)
    : p(p_), pAdv(this, pAdv_), pSlm(this, 0) {
  ImageSequence seq = pAdv->pGl->pr("megaslime_reproduce");

  SP<AnimationOnce> pSlmTmp =
      new AnimationOnce(2.F, seq, unsigned(.1F * nFramesInSecond), p_, true);
  pSlm = pSlmTmp;
  pAdv_->AddBoth(pSlmTmp);

  pAdv->pGl->pSnd->PlaySound(pAdv->pGl->pr.GetSnd("slime_spawn"));
}

void MegaSliminess::Update() {
  if (pSlm->bExist == false) {
    bExist = false;

    SP<MegaSlime> pSlm = new MegaSlime(p, pAdv->rBound, pAdv);
    pAdv->AddBoth(pSlm);
    PushBackASSP(pAdv.GetRawPointer(), pAdv->lsPpl, pSlm);
  }
}

FloatingSlime::FloatingSlime(ImageSequence seq, Point pStart, Point pEnd,
                             unsigned nTime)
    : SimpleVisualEntity(2.F, seq, true, unsigned(.1F * nFramesInSecond)) {
  fPos = pStart;
  tTermination = Timer(nTime);
  fVel = (fPoint(pEnd) - fPoint(pStart)) / float(nTime);
}

void FloatingSlime::Update() {
  SimpleVisualEntity::Update();

  fPos += fVel;

  if (tTermination.Tick())
    bExist = false;
}

void BasicController::AddV(SP<VisualEntity> pVs) {
  lsDraw.push_back(ASSP<VisualEntity>(this, pVs));
}

void BasicController::AddE(SP<EventEntity> pEv) {
  lsUpdate.push_back(ASSP<EventEntity>(this, pEv));
}

void BasicController::AddBackground(Color c) {
  Rectangle r = rBound.sz;
  r.sz.x *= pGl->pDr->nFactor;
  r.sz.y *= pGl->pDr->nFactor;

  SP<StaticRectangle> pBkg = new StaticRectangle(r, c, -1.F);

  AddV(pBkg);
}

BasicController::BasicController(const BasicController &b)
    : GameController(b), bNoRefresh(b.bNoRefresh) {
  CopyArrayASSP(this, b.lsDraw, lsDraw);
  CopyArrayASSP(this, b.lsUpdate, lsUpdate);
  CopyArrayASSP(this, b.lsPpl, lsPpl);
}

BasicController::BasicController(SP<TwrGlobalController> pGl_, Rectangle rBound,
                                 Color c)
    : GameController(pGl_, rBound), bNoRefresh(false) {
  AddBackground(c);
}

void BasicController::Update() {
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

void BasicController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;

  pGl->Next();
}

MenuController::MenuController(SP<TwrGlobalController> pGl_, Rectangle rBound,
                               Color c, int nResumePosition_)
    : BasicController(pGl_, rBound, c), nResumePosition(nResumePosition_),
      pMenuDisplay(this, 0), mc(pGl->pr("claw"), Point(), pGl.GetRawPointer()),
      pHintText(this, 0), pOptionText(this, 0) {
  bNoRefresh = true;
}

void StartScreenController::Next() {
  pGl->Next();
  pGl->pSnd->PlaySound(pGl->pr.GetSnd("start_game"));
}

void StartScreenController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;

  if (c == GUI_ESCAPE)
    pGl->Menu();
  else
    Next();
}

BuyNowController::BuyNowController(SP<TwrGlobalController> pGl_,
                                   Rectangle rBound, Color c)
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

void BuyNowController::RandomizeVelocity(fPoint &fVel, fPoint pPos) {
  fVel = RandomAngle();

  if (rand() % 7 == 0)
    fVel = fPoint(rBound.sz.x / 2, rBound.sz.y / 2) - fPoint(pPos);

  fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fSlimeSpeed);
}

void BuyNowController::DrawSlimes() {
  for (unsigned i = 0; i < mSlimes.size(); i++)
    mSlimes[i]->Draw(pGl->pDr);
}

void BuyNowController::Update() {
  BasicController::Update();

  if (tVel.Tick()) {
    for (unsigned i = 0; i < mSlimes.size(); i++)
      if (float(rand()) / RAND_MAX < .25)
        RandomizeVelocity(mSlimeVel[i], mSlimePos[i]);
  }

  for (unsigned i = 0; i < mSlimes.size(); i++) {
    mSlimes[i]->Update();
    mSlimePos[i] += mSlimeVel[i];
    mSlimes[i]->pos = mSlimePos[i].ToPnt();
  }

  if (t >= 0)
    t--;

  pGl->pGraph->RefreshAll();
}

void BuyNowController::OnKey(GuiKeyType c, bool bUp) {
  if (t < 0)
    BasicController::OnKey(c, bUp);
}

void BuyNowController::OnMouseDown(Point pPos) {
  if (t < 0)
    BasicController::OnMouseDown(pPos);
}

void BrokenLine::CloseLast() {
  if (vEdges.empty() || vEdges.back().empty())
    throw SegmentSimpleException("CloseLast", "Invalid (empty) segment arrays");

  if (vEdges.back().front() != vEdges.back().back())
    vEdges.back().push_back(vEdges.back().front());
}

void BrokenLine::Add(fPoint p) {
  if (vEdges.empty())
    vEdges.push_back(VecLine());
  vEdges.back().push_back(p);
}

void BrokenLine::Join(const BrokenLine &b) {
  for (unsigned i = 0; i < b.vEdges.size(); ++i)
    vEdges.push_back(b.vEdges[i]);
}

BrokenLine::BrokenLine(fPoint p1, fPoint p2) {
  Add(p1);
  Add(p2);
}

BrokenLine::BrokenLine(fPoint p1, fPoint p2, fPoint p3) {
  Add(p1);
  Add(p2);
  Add(p3);
}

BrokenLine::BrokenLine(fPoint p1, fPoint p2, fPoint p3, fPoint p4) {
  Add(p1);
  Add(p2);
  Add(p3);
  Add(p4);
}

BrokenLine::BrokenLine(Rectangle r) {
  Add(r.p);
  Add(fPoint(float(r.Left()), float(r.Bottom())));
  Add(fPoint(float(r.Right()), float(r.Top())));
  Add(r.GetBottomRight());
  CloseLast();
}

fPoint BrokenLine::RandomByLength() {
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

fPoint BrokenLine::RandomBySegment() {
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

std::ostream &operator<<(std::ostream &ofs, const BrokenLine &bl) {
  for (unsigned i = 0; i < bl.vEdges.size(); ++i) {
    for (unsigned j = 0; j < bl.vEdges[i].size(); ++j)
      ofs << bl.vEdges[i][j] << " ";
    ofs << "| ";
  }
  ofs << "& ";
  return ofs;
}

std::istream &operator>>(std::istream &ifs, BrokenLine &bl) {
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

void LevelLayout::Convert(int n) {
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

std::ostream &operator<<(std::ostream &ofs, const LevelLayout &f) {
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

std::istream &operator>>(std::istream &ifs, LevelLayout &f) {
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

float KnightGenerator::GetRate() {
  if (pBc->bGhostTime)
    return dRate / fIncreaseKnightRate2;

  if (pBc->GetCompletionRate() < fIncreaseRateFraction1)
    return dRate;
  else if (pBc->GetCompletionRate() < fIncreaseRateFraction2)
    return dRate / fIncreaseKnightRate1;
  else
    return dRate / fIncreaseKnightRate2;
}

KnightGenerator::KnightGenerator(float dRate_, Rectangle rBound_,
                                 SP<AdvancedController> pBc_,
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

void KnightGenerator::Generate(bool bGolem) {
  Point p = bl.RandomByLength().ToPnt();

  unsigned n = unsigned(rand() % pBc->vCs.size());

  fPoint v = pBc->vCs[n]->GetPosition() - p;
  v.Normalize(fKnightSpeed);
  p += rBound.p;

  SP<Knight> pCr = new Knight(Critter(7, p, v, rBound, 3, seq, true), pBc, 'K');

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

void KnightGenerator::Update() {
  if (tm.Tick()) {
    tm = Timer(GetRandTimeFromRate(GetRate()));
    Generate();
  }
}

PrincessGenerator::PrincessGenerator(float dRate_, Rectangle rBound_,
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

void PrincessGenerator::Update() {
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

MageGenerator::MageGenerator(float dRate_, float dAngryRate_, Rectangle rBound_,
                             SP<AdvancedController> pBc_)
    : rBound(rBound_), pBc(this, pBc_) {
  if (pBc->pGl->bAngry)
    dRate = dAngryRate_;
  else
    dRate = dRate_;

  tm = Timer(GetRandTimeFromRate(dRate));
}

void MageGenerator::Update() {
  if (tm.Tick()) {
    if (dRate == 0)
      return;

    tm = Timer(GetRandTimeFromRate(dRate));

    MageGenerate();
  }
}

void MageGenerator::MageGenerate() {
  Point p, v;

  pBc->vRd[rand() % pBc->vRd.size()]->RoadMap(p, v);

  fPoint vel(v);

  vel.Normalize(fMageSpeed);

  SP<Mage> pCr = new Mage(
      Critter(7, p, vel, rBound, 3,
              vel.x < 0 ? pBc->pGl->pr("mage_f") : pBc->pGl->pr("mage"), true),
      pBc, pBc->pGl->bAngry);
  pBc->AddBoth(pCr);
  PushBackASSP(pBc.GetRawPointer(), pBc->lsPpl, pCr);
}

float TraderGenerator::GetRate() {
  if (pBc->GetCompletionRate() < fIncreaseRateFraction1)
    return dRate;
  else if (pBc->GetCompletionRate() < fIncreaseRateFraction2)
    return dRate / fIncreaseTraderRate1;
  else
    return dRate / fIncreaseTraderRate2;
}

TraderGenerator::TraderGenerator(float dRate_, Rectangle rBound_,
                                 SP<AdvancedController> pBc_)
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

void TraderGenerator::Update() {
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

void DragonLeash::ModifyTilt(Point trackball) {
  tilt -= tilt * naturalScaleFactor;
  tilt += trackball.x * trackballScaleFactor;

  if (tilt > maxTilt)
    tilt = maxTilt;
  if (tilt < -maxTilt)
    tilt = -maxTilt;
}

fPoint DragonLeash::GetNewVelocity(Point trackball) {
  ModifyTilt(trackball);
  Polar p = Polar(lastVel);
  p.r = speed;
  p.a += tilt;
  lastVel = p.TofPoint();
  return lastVel;
}

ButtonSet::ButtonSet(int q, int w, int e, int d, int c, int x, int z, int a,
                     int sp) {
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

Point ButtonSet::GetPoint(int nCode) {
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

unsigned RandomBonus(bool bInTower) {
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

SP<TimedFireballBonus> Dragon::GetBonus(unsigned n, unsigned nTime) {
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
  } else if (n == 5)
    pBonus = new TimedFireballBonus(FireballBonus(n, "explode", 1U), nTime);
  else if (n == 6) {
    pBonus = new TimedFireballBonus(FireballBonus(n, false), nTime);
    pBonus->Add("fireballchainnum", 1U);
    pBonus->Add("through_flag", true);
  } else if (n == 7)
    pBonus = new TimedFireballBonus(FireballBonus(n, "setonfire", 1U), nTime);
  else if (n == 8) {
    FireballBonus fb = GetAllBonuses();

    Point p = GetPosition();

    if (pCs != 0)
      p = pCs->GetPosition();

    int nNumCirc = fb.uMap["pershot"] + 1;

    fPoint fVel = RandomAngle();

    for (int i = 0; i < nNumCirc; ++i) {
      SP<CircularFireball> pFb = new CircularFireball(
          Fireball(p, GetWedgeAngle(fVel, 1.F, i, nNumCirc + 1), pAd, fb,
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
    pBonus = new TimedFireballBonus(FireballBonus(n, "frequency", .5F), nTime);
  }

  return pBonus;
}

void Dragon::FlushBonuses() {
  for (std::list<ASSP<TimedFireballBonus>>::iterator itr = lsBonuses.begin(),
                                                     etr = lsBonuses.end();
       itr != etr; ++itr)
    pAd->pGl->lsBonusesToCarryOver.push_back(*itr);
}

void Dragon::RecoverBonuses() {
  for (std::list<SP<TimedFireballBonus>>::iterator
           itr = pAd->pGl->lsBonusesToCarryOver.begin(),
           etr = pAd->pGl->lsBonusesToCarryOver.end();
       itr != etr; ++itr) {
    AddBonus(*itr, true);
  }
}

FireballBonus Dragon::GetAllBonuses() {
  CleanUp(lsBonuses);
  FireballBonus fbRet(-1, true);

  for (std::list<ASSP<TimedFireballBonus>>::iterator itr = lsBonuses.begin(),
                                                     etr = lsBonuses.end();
       itr != etr; ++itr)
    fbRet += **itr;

  fbRet.uMap["total"] += nExtraFireballs;

  return fbRet;
}

Dragon::Dragon(SP<Castle> pCs_, SP<AdvancedController> pAd_,
               ImageSequence imgStable_, ImageSequence imgFly_, ButtonSet bt_)
    : pAd(this, pAd_), imgStable(imgStable_), imgFly(imgFly_),
      Critter(13, pCs_ == 0 ? pAd_->vCs[0]->GetPosition() : pCs_->GetPosition(),
              Point(), pAd_->rBound, 1, ImageSequence()),
      bFly(), bCarry(false), cCarry(' '), nTimer(0), pCs(this, pCs_), bt(bt_),
      nFireballCount(0), tFireballRegen(1), bTookOff(false), nPrCr(0),
      nExtraFireballs(0), bRegenLocked(false),
      tRegenUnlock(nFramesInSecond * nRegenDelay / 10) {
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

void Dragon::Update() {
  if (bRegenLocked) {
    if (tRegenUnlock.Tick())
      bRegenLocked = false;
  } else {
    FireballBonus fb = GetAllBonuses();

    int nPeriod = nInitialRegen;
    if (nPeriod < 2)
      nPeriod = 2;

    nPeriod = int(float(nPeriod) * nFramesInSecond / 10);

    tFireballRegen.Tick();

    if (int(tFireballRegen.nTimer) >= nPeriod) {
      tFireballRegen.nTimer = 0;

      if (nFireballCount < int(fb.uMap["total"])) {
        if (nInitialRegen == 0)
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

Point Dragon::GetPosition() {
  if (pCs != 0)
    return (fPos + fPoint(0, -1)).ToPnt();
  return fPos.ToPnt();
}

void Dragon::Draw(SP<ScalingDrawer> pDr) {
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

void Dragon::AddBonus(SP<TimedFireballBonus> pBonus, bool bSilent) {
  if (!bSilent)
    pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("powerup"));

  if (pBonus == 0)
    return;

  PushBackASSP(this, lsBonuses, pBonus);
  pAd->AddE(pBonus);
}

void Dragon::Fire(fPoint fDir) {
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

void Dragon::Toggle() {
  if (!bFly) {
    pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("leave_tower"));

    bFly = true;
    bTookOff = true;

    pAd->tutOne.FlyOn();

    SimpleVisualEntity::seq = imgFly;
    SimpleVisualEntity::dPriority = 5;

    pCs->pDrag = 0;
    pCs = 0;

    fVel = pAd->pt.GetFlightDirection(GetPosition());

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

  if (bCarry)
    return;

  for (std::list<ASSP<ConsumableEntity>>::iterator itr = pAd->lsPpl.begin();
       itr != pAd->lsPpl.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    if ((**itr).GetType() != 'T')
      continue;

    if (this->HitDetection(*itr)) {
      if (!bCarry) {
        bCarry = true;
        imgCarry = (*itr)->GetImage();
        cCarry = (*itr)->GetType();

        pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("pickup"));
      } else {
        throw SimpleException("not supposed to drop things");
      }

      (*itr)->bExist = false;

      return;
    }
  }
}

void AlmostBasicController::Update() {
  CleanUp(lsUpdate);
  CleanUp(lsDraw);

  if (lsDraw.size() == 1) {
    pGl->Next();
    return;
  }

  BasicController::Update();
}

void AlmostBasicController::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp)
    pGl->Next();
}

void BackgroundMusicPlayer::SwitchTheme(int nTheme) {
  if (bOff)
    return;

  if (nCurrTheme == nTheme)
    return;

  if (nTheme < 0 || nTheme >= int(vThemes.size()))
    throw SimpleException("BackgroundMusicPlayer", "SwitchTheme",
                          "Bad theme index " + S(nTheme));

  StopMusic();
  pSnd->PlaySound(vThemes[nTheme], BG_MUSIC_CHANNEL, true);

  nCurrTheme = nTheme;
}

void BackgroundMusicPlayer::StopMusic() {
  if (nCurrTheme != -1)
    pSnd->StopSound(BG_MUSIC_CHANNEL);
  nCurrTheme = -1;
}

void DrawStuff(Rectangle rBound, SP<Graphic> pGraph, SP<Soundic> pSnd,
               Preloader &pr, int n) {
#ifdef LOADING_SCREEN
  rBound.sz.x *= 2;
  rBound.sz.y *= 2;

  Size sz1 = pGraph->GetImage(pr["splash"])->GetSize();
  Size sz2 = pGraph->GetImage(pr["loading"])->GetSize();

  Point p1(Crd(rBound.sz.x / 2), Crd(rBound.sz.y / 2));
#ifdef SMALL_SCREEN_VERSION
        Point p2(Crd(rBound.sz.x/2, Crd(rBound.sz.y*7.0f/10);
#else
  Point p2(Crd(rBound.sz.x / 2), Crd(rBound.sz.y * 6.5f / 10));
#endif

	p1.x -= sz1.x/2;
	p1.y -= sz1.y/2;
	p2.x -= sz2.x/2;
	p2.y -= sz2.y/2;

	pGraph->DrawImage(p1, pr["splash"], false);

	pGraph->DrawImage(p2, pr["loading"], Rectangle(0,0,144*n/9,32), false);

	pGraph->DrawImage(p2, pr["loading"], Rectangle(0,0,144*n/9,32), false);

	pGraph->RefreshAll();

#ifndef PC_VERSION
	if(n%2)
		pSnd->PlaySound(pr.GetSnd("beep"));
	else pSnd->PlaySound(pr.GetSnd("boop"));
#endif

#endif // LOADING_SCREEN
}

const std::string sFullScreenPath = "fullscreen.txt";

TwrGlobalController::TwrGlobalController(
    SP<ScalingDrawer> pDr_, SP<NumberDrawer> pNum_, SP<NumberDrawer> pBigNum_,
    SP<FontWriter> pFancyNum_, SP<Soundic> pSndRaw_, const LevelStorage &vLvl_,
    Rectangle rBound_, TowerDataWrap *pWrp_, FilePath fp)
    : nActive(1), pDr(this, pDr_), pGraph(this, pDr_->pGr), pNum(this, pNum_),
      pBigNum(this, pBigNum_), pr(pDr_->pGr, pSndRaw_, fp),
      pSndRaw(this, pSndRaw_), pSnd(this, new SoundInterfaceProxy(pSndRaw)),
      nScore(0), vLvl(vLvl_), rBound(rBound_), bAngry(false), nHighScore(0),
      pFancyNum(this, pFancyNum_), pWrp(pWrp_), pMenu(this, 0),
      vLevelPointers(3), sbTutorialOn("tutorial_on.txt", true, true),
      snProgress("stuff.txt", 0, true),
      sbFullScreen(sFullScreenPath, false, true),
      sbSoundOn("soundon.txt", true, true),
      sbMusicOn("musicon.txt", true, true),
      sbCheatsOn("cheat.txt", false, true),
      sbCheatsUnlocked("more_stuff.txt", false, true) {
  {
    std::ifstream ifs("high.txt");

    if (!ifs.fail())
      ifs >> nHighScore;

    ifs.close();
  }

  typedef ImagePainter::ColorMap ColorMap;

  std::vector<ColorMap> vColors;
  vColors.push_back(ColorMap(Color(0, 255, 0), Color(0, 0, 255)));
  vColors.push_back(ColorMap(Color(0, 128, 0), Color(0, 0, 128)));
  vColors.push_back(ColorMap(Color(0, 127, 0), Color(0, 0, 127)));
  vColors.push_back(ColorMap(Color(0, 191, 0), Color(0, 0, 191)));

  unsigned nScale = 2;

  pr.AddTransparent(Color(0, 0, 0));
  pr.SetScale(nScale);

  pr.LoadTS("icons.bmp", "loading");
  pr.LoadTS("robotbear.bmp", "splash");
  pr.LoadSnd("beep.wav", "beep");
  pr.LoadSnd("boop.wav", "boop");
  DrawStuff(rBound, pGraph, pSndRaw_, pr, 0);

  pr.LoadTS("road.bmp", "road");
  pr.LoadTS("turnandvorn.bmp", "logo");
#ifdef TRIAL_VERSION
  pr.LoadTS("trial.bmp", "trial");
  pr.LoadTS("buy_now.bmp", "buy");
#endif
  pr.LoadSeqTS("burn\\burn.txt", "burn");
  pr.LoadTS("empty.bmp", "empty");

  pr.LoadSeqTS("arrow\\sword.txt", "arrow");
  pr.LoadSeqTS("arrow\\claw.txt", "claw");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 1);

  pr.LoadSeqTS("corona\\crosshair.txt", "corona"); // (not used icon)
  pr.LoadSeqTS("bonus\\void.txt", "void_bonus");
  pr.LoadSeqTS("bonus\\pershot.txt", "pershot_bonus"); // 1 - pershot
  pr.LoadSeqTS("bonus\\laser.txt", "laser_bonus");     // 2 - laser
  pr.LoadSeqTS("bonus\\big.txt", "big_bonus");         // 3 - big
  pr.LoadSeqTS("bonus\\totnum.txt", "totnum_bonus");   // 4 - totnum
  pr.LoadSeqTS("bonus\\explode.txt", "explode_bonus"); // 5 - explode
  pr.LoadSeqTS("bonus\\split.txt", "split_bonus");     // 6 - split
  pr.LoadSeqTS("bonus\\burning.txt", "burning_bonus"); // 7 - burning
  pr.LoadSeqTS("bonus\\ring.txt", "ring_bonus");       // 8 - ring
  pr.LoadSeqTS("bonus\\nuke.txt", "nuke_bonus");       // 9 - nuke
  pr.LoadSeqTS("bonus\\speed.txt", "speed_bonus");     // 10 - speed
  // pr.LoadSeqTS("bonus\\frequency.txt", "frequency");		// 11 -
  // frequency (not used)

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 2);
  pr.LoadSeqTS("start.txt", "start");
  pr.LoadSeqTS("win\\win.txt", "win");
  pr.LoadSeqTS("win\\over.txt", "over");
  pr.LoadSeqTS("logo\\pluanbo.txt", "pluanbo", Color(0, 0, 0), nScale * 2);
  pr.LoadSeqTS("logo\\gengui.txt", "gengui", Color(0, 0, 0), nScale * 2);
  pr.LoadSeqTS("castle\\castle.txt", "castle", Color(0, 0, 0));
  pr.LoadSeqTS("castle\\destroy_castle_dust.txt", "destroy_castle",
               Color(0, 0, 0));

  pr.LoadSeqTS("dragon_fly\\fly.txt", "dragon_fly");
  pr.LoadSeqTS("dragon\\stable.txt", "dragon_stable");
  pr.LoadSeqTS("dragon\\walk.txt", "dragon_walk");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 3);

  pr.AddSequence(pr("dragon_fly"), "bdragon_fly");
  pr.AddSequence(pr("dragon_stable"), "bdragon_stable");
  pr.AddSequence(pr("dragon_walk"), "dragon_walk_f");

  ForEachImage(pr("bdragon_fly"), ImagePainter(pGraph, vColors));
  ForEachImage(pr("bdragon_stable"), ImagePainter(pGraph, vColors));
  ForEachImage(pr("dragon_walk_f"), ImageFlipper(pGraph));

  pr.LoadSeqTS("explosion\\explosion2.txt", "explosion");
  pr.LoadSeqTS("explosion\\laser_expl.txt", "laser_expl");

  pr.LoadSeqTS("explosion\\explosion_15.txt", "explosion_15");
  pr.LoadSeqTS("explosion\\laser_expl_15.txt", "laser_expl_15");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 4);

  pr.LoadSeqTS("explosion\\explosion2.txt", "explosion_2", Color(0, 0, 0, 0),
               nScale * 2);
  pr.LoadSeqTS("explosion\\laser_expl.txt", "laser_expl_2", Color(0, 0, 0, 0),
               nScale * 2);

  pr.LoadSeqTS("explosion\\explosion_15.txt", "explosion_3", Color(0, 0, 0, 0),
               nScale * 2);
  pr.LoadSeqTS("explosion\\laser_expl_15.txt", "laser_expl_3",
               Color(0, 0, 0, 0), nScale * 2);

  pr.LoadSeqTS("fireball\\fireball.txt", "fireball");
  pr.LoadSeqTS("fireball\\fireball_15.txt", "fireball_15");
  pr.LoadSeqTS("fireball\\fireball.txt", "fireball_2", Color(0, 0, 0, 0),
               nScale * 2);
  pr.LoadSeqTS("fireball\\fireball_15.txt", "fireball_3", Color(0, 0, 0, 0),
               nScale * 2);

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 5);

  pr.LoadSeqTS("fireball\\laser.txt", "laser");
  pr.LoadSeqTS("fireball\\laser_15.txt", "laser_15");
  pr.LoadSeqTS("fireball\\laser.txt", "laser_2", Color(0, 0, 0, 0), nScale * 2);
  pr.LoadSeqTS("fireball\\laser_15.txt", "laser_3", Color(0, 0, 0, 0),
               nScale * 2);
  pr.LoadSeqTS("fireball\\fireball_icon.txt", "fireball_icon",
               Color(255, 255, 255));

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 6);

  pr.LoadSeqTS("knight\\knight.txt", "knight");
  pr.LoadSeqTS("knight\\die.txt", "knight_die");
  pr.LoadSeqTS("knight\\fire.txt", "knight_fire");
  pr.LoadSeqTS("ghostnight\\ghost_knight_burn.txt", "ghost_knight_burn");
  pr.LoadSeqTS("ghostnight\\ghost_knight.txt", "ghost_knight");
  pr.LoadSeqTS("ghostnight\\ghost_burn.txt", "ghost_burn");
  pr.LoadSeqTS("ghostnight\\ghost.txt", "ghost");

  pr.LoadSeqTS("golem\\golem.txt", "golem");
  pr.LoadSeqTS("golem\\golem_death.txt", "golem_die");

  pr.AddSequence(pr("golem"), "golem_f");
  ForEachImage(pr("golem_f"), ImageFlipper(pGraph));

  pr.AddSequence(pr("golem_die"), "golem_die_f");
  ForEachImage(pr("golem_die_f"), ImageFlipper(pGraph));

  pr.LoadSeqTS("skelly\\skelly.txt", "skelly");
  pr.LoadSeqTS("skelly\\die.txt", "skelly_die");
  pr.LoadSeqTS("skelly\\summon.txt", "skelly_summon");

  pr.LoadSeqTS("trader\\trader.txt", "trader");
  pr.LoadSeqTS("trader\\die.txt", "trader_die");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 7);

  pr.AddSequence(pr("trader"), "trader_f");
  pr.AddSequence(pr("trader_die"), "trader_die_f");
  ForEachImage(pr("trader_f"), ImageFlipper(pGraph));
  ForEachImage(pr("trader_die_f"), ImageFlipper(pGraph));

  pr.LoadSeqTS("princess\\princess.txt", "princess");
  pr.LoadSeqTS("princess\\die.txt", "princess_die");

  pr.AddSequence(pr("princess"), "princess_f");
  pr.AddSequence(pr("princess_die"), "princess_die_f");
  ForEachImage(pr("princess_f"), ImageFlipper(pGraph));
  ForEachImage(pr("princess_die_f"), ImageFlipper(pGraph));

  pr.LoadSeqTS("mage\\mage.txt", "mage");
  pr.LoadSeqTS("mage\\spell.txt", "mage_spell");
  pr.LoadSeqTS("mage\\die.txt", "mage_die");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 8);

  pr.AddSequence(pr("mage"), "mage_f");
  pr.AddSequence(pr("mage_spell"), "mage_spell_f");
  pr.AddSequence(pr("mage_die"), "mage_die_f");
  ForEachImage(pr("mage_f"), ImageFlipper(pGraph));
  ForEachImage(pr("mage_spell_f"), ImageFlipper(pGraph));
  ForEachImage(pr("mage_die_f"), ImageFlipper(pGraph));

  pr.LoadSeqTS("slime\\slime_walk.txt", "slime");
  pr.LoadSeqTS("slime\\slime_die.txt", "slime_die");
  pr.LoadSeqTS("slime\\slime_poke.txt", "slime_poke");
  pr.LoadSeqTS("slime\\slime_reproduce.txt", "slime_reproduce");
  pr.LoadSeqTS("slime\\slime_reproduce_fast.txt", "slime_reproduce_fast");
  pr.LoadSeqTS("slime\\slime_cloud.txt", "slime_cloud");

  pr.LoadSeqTS("slime\\mega_slime_walk.txt", "megaslime");
  pr.LoadSeqTS("slime\\mega_slime_die.txt", "megaslime_die");
  pr.LoadSeqTS("slime\\mega_slime_reproduce.txt", "megaslime_reproduce");

  pr.LoadSndSeq("sound\\over.txt", "over");
  pr.LoadSndSeq("sound\\pluanbo.txt", "pluanbo");
  pr.LoadSndSeq("sound\\click.txt", "click");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 9);

  pr.LoadSnd("start_game.wav", "start_game");
  pr.LoadSnd("death01.wav", "death");
  pr.LoadSnd("golem_death2.wav", "golem_death");
  pr.LoadSnd("megaslime_spawn1.wav", "slime_spawn");
  pr.LoadSnd("megaslime_spawn2.wav", "megaslime_die");
  pr.LoadSnd("megaslime_hit.wav", "megaslime_hit");
  pr.LoadSnd("megaslime_bonus2.wav", "megaslime_bonus");
  pr.LoadSnd("megaslime_move4.wav", "megaslime_jump");
  pr.LoadSnd("megaslime_move3.wav", "megaslime_land");
  pr.LoadSnd("skeleton_bonus1.wav", "skeleton_bonus");

  pr.LoadSnd("explosion01.wav", "explosion");
  pr.LoadSnd("powerup03.wav", "powerup");
  pr.LoadSnd("laser04.wav", "laser");
  pr.LoadSnd("shoot2.wav", "shoot");
  pr.LoadSnd("dropping.wav", "knight_fall");
  pr.LoadSnd("knight_from_sky.wav", "dropping");
  pr.LoadSnd("pickup_sound_pxtone.wav", "pickup");
  pr.LoadSnd("princess_capture.wav", "princess_capture");
  pr.LoadSnd("win_level_2.wav", "win_level");
  pr.LoadSnd("all_princess_escape.wav", "all_princess_escape");
  pr.LoadSnd("destroy_castle_sound.wav", "destroy_castle_sound");
  pr.LoadSnd("one_princess.wav", "one_princess");
  pr.LoadSnd("leave_tower.wav", "leave_tower");
  pr.LoadSnd("return_tower.wav", "return_tower");
  pr.LoadSnd("step_left.wav", "step_left");
  pr.LoadSnd("step_right.wav", "step_right");
  pr.LoadSnd("hit_golem.wav", "hit_golem");
  pr.LoadSnd("slime_poke.wav", "slime_poke");
  pr.LoadSnd("slime_summon.wav", "slime_summon");
  pr.LoadSnd("princess_arrive.wav", "princess_arrive");

  pr.LoadSnd("sound/A.wav", "A");
  pr.LoadSnd("sound/B.wav", "B");
  pr.LoadSnd("sound/C.wav", "C");
  pr.LoadSnd("sound/D.wav", "D");
  pr.LoadSnd("sound/E.wav", "E");

  pr.LoadSnd("dddragon.wav", "background_music");
  pr.LoadSnd("_dddragon.wav", "background_music_slow");

#ifdef FULL_VERSION
  pr.LoadSnd("dddragon1.wav", "background_music2");
  pr.LoadSnd("_dddragon1.wav", "background_music_slow2");

  pr.LoadSnd("dddragon2.wav", "background_music3");
  pr.LoadSnd("_dddragon2.wav", "background_music_slow3");
#else // save memory on trial, though, *something* needs to be loaded otherwise
      // crashes.
  pr.LoadSnd("dddragon.wav", "background_music2");
  pr.LoadSnd("_dddragon.wav", "background_music_slow2");

  pr.LoadSnd("dddragon.wav", "background_music3");
  pr.LoadSnd("_dddragon.wav", "background_music_slow3");
#endif

  plr.pSnd = pSndRaw_;

  plr.vThemes.resize(6);
  plr.vThemes[BG_BACKGROUND] = pr.GetSnd("background_music");
  plr.vThemes[BG_SLOW_BACKGROUND] = pr.GetSnd("background_music_slow");
  plr.vThemes[BG_BACKGROUND2] = pr.GetSnd("background_music2");
  plr.vThemes[BG_SLOW_BACKGROUND2] = pr.GetSnd("background_music_slow2");
  plr.vThemes[BG_BACKGROUND3] = pr.GetSnd("background_music3");
  plr.vThemes[BG_SLOW_BACKGROUND3] = pr.GetSnd("background_music_slow3");

  plr.pSnd->SetVolume(.5);

  if (!sbMusicOn.Get())
    plr.ToggleOff();

  if (!sbSoundOn.Get())
    pSnd->Toggle();

  pNum->CacheColor(Color(205, 205, 0));
  pNum->CacheColor(Color(155, 155, 0));
  pNum->CacheColor(Color(105, 105, 0));
  pNum->CacheColor(Color(55, 55, 0));
  pNum->CacheColor(Color(5, 5, 0));
}

void TwrGlobalController::StartUp() {
  nScore = 0;
  bAngry = false;

  SP<Animation> pStr =
      new Animation(0, pr("start"), nFramesInSecond / 5,
                    Point(rBound.sz.x / 2, rBound.sz.y * 3 / 4), true);

  // menu
  SP<MenuController> pMenuHolder = new MenuController(
      this, rBound, Color(0, 0, 0), 3); // resume position shouldn't matter
  pMenu = pMenuHolder;

  // logo 1
  SP<BasicController> pCnt0_1 =
      new AlmostBasicController(BasicController(this, rBound, Color(0, 0, 0)));
  // logo 2
  SP<BasicController> pCnt0_2 =
      new AlmostBasicController(BasicController(this, rBound, Color(0, 0, 0)));
  // press start screen
  SP<BasicController> pCnt1 =
      new StartScreenController(this, rBound, Color(0, 0, 0));
  // game over
  SP<BasicController> pCnt2 =
      new AlmostBasicController(BasicController(this, rBound, Color(0, 0, 0)));
  // you win!
  SP<BasicController> pCnt3 = new BasicController(this, rBound, Color(0, 0, 0));
  // score
  SP<DragonScoreController> pScore =
      new DragonScoreController(this, rBound, Color(0, 0, 0), true);
  // intro tutorial screen (non PC version)
  SP<DragonScoreController> pIntro =
      new DragonScoreController(this, rBound, Color(0, 0, 0), false);

  // cutscenes
  SP<BasicController> pCut1 = new Cutscene(this, rBound, "princess", "knight");
  SP<BasicController> pCut2 =
      new Cutscene(this, rBound, "knight", "dragon_walk_f", true);
  SP<BasicController> pCut3 = new Cutscene(this, rBound, "dragon_walk", "mage");

  SP<SoundControls> pBckgMusic = new SoundControls(plr, BG_BACKGROUND);
  SP<SoundControls> pNoMusic = new SoundControls(plr, -1);

  SP<Animation> pWin = new Animation(
      0, pr("win"), 3, Point(rBound.sz.x / 2, rBound.sz.y / 2 - 20), true);
  SP<StaticImage> pL = new StaticImage(
      pr["logo"], Point(rBound.sz.x / 2, rBound.sz.y / 3), true);
  SP<Animation> pBurnL =
      new Animation(0, pr("burn"), 3,
                    Point(rBound.sz.x / 2 - 45, rBound.sz.y / 2 - 64), true);
  SP<Animation> pBurnR =
      new Animation(0, pr("burn"), 4,
                    Point(rBound.sz.x / 2 - 54, rBound.sz.y / 2 - 64), true);
  pBurnR->seq.nActive += 4;

  std::vector<std::string> vHintPref;
  vHintPref.push_back("hint: ");
  vHintPref.push_back("tip: ");
  vHintPref.push_back("secret: ");
  vHintPref.push_back("beware: ");
  vHintPref.push_back("fun fact: ");

  std::vector<std::string> vHints;
  vHints.push_back("you can shoot while flying");
  vHints.push_back("spooky things happen when time runs out");
  vHints.push_back("more princesses you capture - more knights show up");
  vHints.push_back("you can capture multiple princesses in one flight");
#ifndef KEYBOARD_CONTROLS
  vHints.push_back("pick up traders by clicking when flying over them\nbring "
                   "them to the tower to get longer bonuses");
#endif
  vHints.push_back("this game was originally called tower defense");

  std::string sHint = vHintPref.at(rand() % vHintPref.size()) +
                      vHints.at(rand() % vHints.size());

  SP<TextDrawEntity> pHintText = new TextDrawEntity(
      0, Point(rBound.sz.x / 2, rBound.sz.y * 7 / 8), true, sHint, pNum);
  SP<TextDrawEntity> pOptionText = new TextDrawEntity(
      0, Point(rBound.sz.x / 2, rBound.sz.y * 7 / 8), true, "sup", pNum);

  SP<AnimationOnce> pO =
      new AnimationOnce(0, pr("over"), nFramesInSecond / 2,
                        Point(rBound.sz.x / 2, Crd(rBound.sz.y / 2.5f)), true);
  SP<AnimationOnce> pPlu =
      new AnimationOnce(0, pr("pluanbo"), nFramesInSecond / 10,
                        Point(rBound.sz.x / 2, rBound.sz.y / 2), true);
  SP<AnimationOnce> pGen =
      new AnimationOnce(0, pr("gengui"), nFramesInSecond / 5,
                        Point(rBound.sz.x / 2, rBound.sz.y / 2), true);

  SP<SimpleSoundEntity> pOver =
      new SimpleSoundEntity(pr.GetSndSeq("over"), nFramesInSecond / 2, pSnd);
  SP<SimpleSoundEntity> pPluSnd = new SimpleSoundEntity(
      pr.GetSndSeq("pluanbo"), nFramesInSecond / 10, pSnd);
  SP<SimpleSoundEntity> pClkSnd =
      new SimpleSoundEntity(pr.GetSndSeq("click"), nFramesInSecond / 5, pSnd);

#ifdef TRIAL_VERSION
  SP<StaticImage> pTrial = new StaticImage(
      pr["trial"], Point(rBound.sz.x / 2 - 73, rBound.sz.y / 3 + 28), true);
  pCnt1->AddV(pTrial);
#endif

  SP<Animation> pMenuCaret =
      new Animation(2, pr("arrow"), 3, Point(0, 0), true);
  // menu entity
  SP<MenuDisplay> pMenuDisplay =
      new MenuDisplay(Point(rBound.sz.x / 2 - 8, rBound.sz.y / 2), pNum,
                      pMenuCaret, pMenu, sbCheatsUnlocked.Get());

  pMenu->AddBoth(pMenuDisplay);
  pMenu->pMenuDisplay = pMenuDisplay;

  pCnt1->AddBoth(pStr);

  pCnt1->AddV(pL);
  pCnt1->AddBoth(pBurnL);
  pCnt1->AddBoth(pBurnR);

  pMenu->AddV(pL);
  pMenu->AddBoth(pBurnL);
  pMenu->AddBoth(pBurnR);
  pMenu->AddBoth(pMenuCaret);

#ifdef PC_VERSION
  pMenu->pHintText = pHintText;
  pMenu->pOptionText = pOptionText;
#else
  pCnt1->AddV(pHintText);
#endif

  pCnt3->AddBoth(pWin);
  pCnt2->AddBoth(pO);
  pCnt2->AddE(pOver);

  pCnt0_1->AddBoth(pPlu);
  pCnt0_1->AddE(pPluSnd);

  pCnt0_2->AddBoth(pGen);
  pCnt0_2->AddE(pClkSnd);

  pCnt1->AddE(pBckgMusic);

  pMenu->AddE(pNoMusic);

  pCut1->AddE(pNoMusic);
  pCut2->AddE(pNoMusic);
  pCut3->AddE(pNoMusic);

  pCnt2->AddE(pNoMusic);
  pCnt3->AddE(pNoMusic);

  PushBackASSP(this, vCnt, pMenuHolder); // menu
  PushBackASSP(this, vCnt, pCnt0_1);     // logo 1
  PushBackASSP(this, vCnt, pCnt0_2);     // logo 2
  PushBackASSP(this, vCnt, pCnt1);       // press start screen
#ifndef PC_VERSION
  PushBackASSP(this, vCnt, pIntro); // tutorial screen
#endif

  for (unsigned i = 0; i < vLvl.size(); ++i) {
    SP<AdvancedController> pAd =
        new AdvancedController(this, rBound, Color(0, 0, 0), vLvl[i]);

    pAd->AddE(pBckgMusic);
    pAd->pSc = pBckgMusic;

    // game level
    PushBackASSP(this, vCnt, pAd);

    // chapters
    if (i == 0)
      vLevelPointers.at(0) = vCnt.size() - 1;
    else if (i == 3)
      vLevelPointers.at(1) = vCnt.size() - 1;
    else if (i == 6)
      vLevelPointers.at(2) = vCnt.size() - 1;

    // cutscene
    if (i == 2)
      PushBackASSP(this, vCnt, pCut1);
    if (i == 5)
      PushBackASSP(this, vCnt, pCut2);
    if (i == 8)
      PushBackASSP(this, vCnt, pCut3);
  }

#ifdef TRIAL_VERSION
  SP<BuyNowController> pBuy =
      new BuyNowController(this, rBound, Color(0, 0, 0));

  SP<Animation> pGolem =
      new Animation(0, pr("golem_f"), nFramesInSecond / 10,
                    Point(rBound.sz.x / 4, rBound.sz.y * 3 / 4 - 10), true);
  SP<Animation> pSkeleton1 =
      new Animation(0, pr("skelly"), nFramesInSecond / 4,
                    Point(rBound.sz.x * 3 / 4, rBound.sz.y * 3 / 4 - 5), true);
  SP<Animation> pSkeleton2 = new Animation(
      0, pr("skelly"), nFramesInSecond / 4 + 1,
      Point(rBound.sz.x * 3 / 4 - 10, rBound.sz.y * 3 / 4 - 15), true);
  SP<Animation> pSkeleton3 = new Animation(
      0, pr("skelly"), nFramesInSecond / 4 - 1,
      Point(rBound.sz.x * 3 / 4 + 10, rBound.sz.y * 3 / 4 - 15), true);
  SP<Animation> pMage =
      new Animation(0, pr("mage_spell"), nFramesInSecond / 2,
                    Point(rBound.sz.x / 2, rBound.sz.y * 3 / 4), true);
  SP<Animation> pGhost =
      new Animation(0, pr("ghost"), nFramesInSecond / 6,
                    Point(rBound.sz.x * 5 / 8, rBound.sz.y * 3 / 4 - 30), true);
  SP<Animation> pWhiteKnight =
      new Animation(0, pr("ghost_knight"), nFramesInSecond / 6,
                    Point(rBound.sz.x * 3 / 8, rBound.sz.y * 3 / 4 - 30), true);

  // SP<StaticImage> pBuyNow = new StaticImage(pr["buy"],Point(rBound.sz.x/2 -
  // 73,rBound.sz.y/3 + 33), true);
  SP<StaticImage> pBuyNow = new StaticImage(
      pr["buy"], Point(rBound.sz.x / 2, rBound.sz.y / 3 + 33), true);
  SP<VisualEntity> pSlimeUpd = new SlimeUpdater(pBuy.GetRawPointer());

  pBuy->AddV(pL);
  pBuy->AddV(pSlimeUpd);
  pBuy->AddBoth(pBurnL);
  pBuy->AddBoth(pBurnR);
  pBuy->AddV(pBuyNow);

  pBuy->AddBoth(pGolem);
  pBuy->AddBoth(pSkeleton1);
  pBuy->AddBoth(pSkeleton2);
  pBuy->AddBoth(pSkeleton3);
  pBuy->AddBoth(pMage);
  pBuy->AddBoth(pGhost);
  pBuy->AddBoth(pWhiteKnight);
#endif

#ifdef FULL_VERSION
  PushBackASSP(this, vCnt, pCnt3);  // you win
  PushBackASSP(this, vCnt, pCnt2);  // game over
  PushBackASSP(this, vCnt, pScore); // score
#else
  PushBackASSP(this, vCnt, pCnt2);
  PushBackASSP(this, vCnt, pBuy);
#endif
}

void TwrGlobalController::Next() {
  if (nActive == vCnt.size() - 1)
    Restart();
  else {
    ++nActive;

    for (unsigned i = 0; i < vLevelPointers.size(); ++i) {
      if (nActive == vLevelPointers.at(i) && snProgress.Get() < int(i)) {
        snProgress.Set(i);
        pMenu->pMenuDisplay->UpdateMenuEntries();
      }
    }
  }
}

void TwrGlobalController::Restart(int nActive_ /* = -1*/) {
  if (nActive_ == -1)
    nActive = 3;
  else
    nActive = nActive_;

  vCnt.clear();

  CleanIslandSeeded(this);

  StartUp();
}

void TwrGlobalController::Menu() {
  pMenu->nResumePosition = nActive;
  nActive = 0;
}

KnightOnFire::KnightOnFire(const Critter &cr, SP<BasicController> pBc_,
                           unsigned nTimer_, Chain c_)
    : Critter(cr), pBc(this, pBc_), nTimer(nTimer_), nTimer_i(nTimer_), c(c_),
      t(nFramesInSecond / 5) {
  Critter::seq = pBc->pGl->pr("knight_fire");
  RandomizeVelocity();
}

void KnightOnFire::Update() {
  Critter::Update();

  for (std::list<ASSP<ConsumableEntity>>::iterator itr = pBc->lsPpl.begin();
       itr != pBc->lsPpl.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    if (this->HitDetection(*itr)) {
      char cType = (*itr)->GetType();

      if (cType == 'W' || cType == 'E')
        continue;

      if (c.IsLast() || cType != 'K')
        (*itr)->OnHit('F');
      else {
        (*itr)->bExist = false;
        SP<KnightOnFire> pKn = new KnightOnFire(
            Critter(GetRadius(), (*itr)->GetPosition(), fPoint(), rBound,
                    GetPriority(), ImageSequence(), true),
            pBc, nTimer_i, c.Evolve());
        pBc->AddBoth(pKn);
      }
    }
  }

  if (nTimer != 0 && --nTimer == 0) {
    bExist = false;

    SP<AnimationOnce> pAn = new AnimationOnce(
        dPriority, pBc->pGl->pr("knight_die"),
        unsigned(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true);
    pBc->AddBoth(pAn);
  }

  if (t.Tick() && float(rand()) / RAND_MAX < .25)
    RandomizeVelocity();
}

struct AdNumberDrawer : public VisualEntity {
  SSP<AdvancedController> pAd;

  AdNumberDrawer() : pAd(this, 0) {}

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    unsigned n = pDr->nFactor;

#ifdef FULL_VERSION
    pAd->pGl->pNum->DrawNumber(pAd->pGl->nScore,
                               Point(pAd->rBound.sz.x - 27 * 4, 4), 7);

    if (pAd->bBlink) {
      Color c(255, 255, 0);

      if (pAd->bTimerFlash)
        c = Color(255, 0, 0);

      pAd->pGl->pNum->DrawColorNumber(
          (pAd->t.nPeriod - pAd->t.nTimer) / nFramesInSecond,
          Point(pAd->rBound.sz.x - 14 * 4, 4), c, 4);
      pAd->pGl->pNum->DrawColorWord(
          "time:", Point(pAd->rBound.sz.x - 19 * 4, 4), c);
    }

    pAd->pGl->pNum->DrawNumber(pAd->nLvl, Point(pAd->rBound.sz.x - 3 * 4, 4),
                               2);

    pAd->pGl->pNum->DrawWord("score:", Point(pAd->rBound.sz.x - 33 * 4, 4));
    pAd->pGl->pNum->DrawWord("level:", Point(pAd->rBound.sz.x - 9 * 4, 4));
    if (pAd->bCh) {
      pAd->pGl->pNum->DrawColorWord(
          "invincible", Point(pAd->rBound.sz.x - 44 * 4, 4), Color(0, 255, 0));
    }
#else
    pAd->pGl->pNum->DrawNumber(pAd->pGl->nScore,
                               Point(pAd->rBound.sz.x - 17 * 4, 4), 7);
    pAd->pGl->pNum->DrawNumber(pAd->nLvl, Point(pAd->rBound.sz.x - 3 * 4, 4),
                               2);

    pAd->pGl->pNum->DrawWord("score:", Point(pAd->rBound.sz.x - 23 * 4, 4));
    pAd->pGl->pNum->DrawWord("level:", Point(pAd->rBound.sz.x - 9 * 4, 4));
#endif
  }
  /*virtual*/ Point GetPosition() { return Point(); }
  /*virtual*/ float GetPriority() { return 10; }
};

struct BonusDrawer : public VisualEntity {
  typedef std::list<ASSP<TimedFireballBonus>> BonusList;

  SSP<AdvancedController> pAd;

  Timer t;
  int nAnimationCounter;

  BonusDrawer()
      : pAd(this, 0), t(unsigned(nFramesInSecond * .1F)), nAnimationCounter(0) {
  }

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    if (t.Tick())
      ++nAnimationCounter;

    Point p(1, 3);

    for (unsigned nDr = 0; nDr < pAd->vDr.size(); ++nDr) {
      BonusList &lst = pAd->vDr[nDr]->lsBonuses;

      for (BonusList::iterator itr = lst.begin(), etr = lst.end(); itr != etr;
           ++itr) {

        SP<TimedFireballBonus> pBns = *itr;

        if (pBns->t.nPeriod &&
            (pBns->t.nPeriod - pBns->t.nTimer) < 4 * nFramesInSecond)
          if (nAnimationCounter % 2 == 1) {
            p.x += 10;
            continue;
          }

        ImageSequence img = GetBonusImage(pBns->nNum, pAd->pGl->pr);

        pDr->Draw(img.vImage[nAnimationCounter % img.vImage.size()], p, false);

        p.x += 10;
      }

      p.y += 10;

      p.x = 3;

      for (int i = 0; i < pAd->vDr[nDr]->nFireballCount; ++i) {
        pDr->Draw(pAd->pGl->pr("fireball_icon").GetImage(), p, false);

        p.x += 7;
      }

      p.y += 7;
      p.x = 0;
    }
  }
  /*virtual*/ Point GetPosition() { return Point(); }
  /*virtual*/ float GetPriority() { return 10; }
};

AdvancedController::AdvancedController(SP<TwrGlobalController> pGl_,
                                       Rectangle rBound, Color c,
                                       const LevelLayout &lvl)
    : BasicController(pGl_, rBound, c), bCh(false), nLvl(lvl.nLvl),
      nSlimeNum(0), bPaused(false), bFirstUpdate(true), bLeftDown(false),
      bRightDown(false), nLastDir(0), bWasDirectionalInput(0),
      bGhostTime(false), bBlink(true), pGr(0), bLeft(false), pSc(0),
      bTakeOffToggle(false), pTutorialText(this, 0),
      mc(pGl->pr("claw"), Point(), pGl.GetRawPointer()), bTimerFlash(false) {
  bNoRefresh = true;

  tLoseTimer.nPeriod = 0;

  SP<AdNumberDrawer> pNm = new AdNumberDrawer();
  pNm->pAd = this;
  AddV(pNm);

  SP<BonusDrawer> pBd = new BonusDrawer();
  pBd->pAd = this;
  AddV(pBd);

  SP<KnightGenerator> pGen =
      new KnightGenerator(lvl.vFreq.at(0), rBound, this, lvl.blKnightGen);
  SP<PrincessGenerator> pPGen =
      new PrincessGenerator(lvl.vFreq.at(1), rBound, this);
  SP<TraderGenerator> pTGen =
      new TraderGenerator(lvl.vFreq.at(2), rBound, this);
  SP<MageGenerator> pMGen =
      new MageGenerator(lvl.vFreq.at(3), lvl.vFreq.at(4), rBound, this);

  pGr = pGen.GetRawPointer();
  pMgGen = pMGen.GetRawPointer();

  unsigned i;
  for (i = 0; i < lvl.vRoadGen.size(); ++i)
    PushBackASSP(this, vRd, new FancyRoad(lvl.vRoadGen[i], this));

  for (i = 0; i < lvl.vCastleLoc.size(); ++i)
    PushBackASSP(this, vCs, new Castle(lvl.vCastleLoc[i], rBound, this));

  t = Timer(lvl.nTimer);

  PushBackASSP(
      this, vDr,
      new Dragon(vCs[0], this, pGl->pr("dragon_stable"), pGl->pr("dragon_fly"),
                 ButtonSet('q', 'w', 'e', 'd', 'c', 'x', 'z', 'a', ' ')));

  Point pos(pGl->rBound.sz.x / 2, pGl->rBound.sz.y);
  SP<TutorialTextEntity> pTT = new TutorialTextEntity(
      1, pos, pGl->pNum, pGl->sbTutorialOn.GetConstPointer());
  pTutorialText = pTT;

#ifdef PC_VERSION

  if (nLvl == 1) {
    tutOne.pTexter = pTT;
    tutOne.Update();
    AddBoth(pTT);
  }

  if (nLvl == 2) {
    tutTwo.pTexter = pTT;
    tutTwo.Update();
    AddBoth(pTT);
  }

#endif

  AddE(pGen);
  AddE(pPGen);
  AddE(pTGen);
  AddE(pMGen);

  for (i = 0; i < vCs.size(); ++i)
    AddBoth(vCs[i]);
  for (i = 0; i < vRd.size(); ++i)
    AddV(vRd[i]);
  for (i = 0; i < vDr.size(); ++i)
    AddBoth(vDr[i]);
}

const float fSpreadFactor = 2.0f;

/*virtual*/ void AdvancedController::OnKey(GuiKeyType c, bool bUp) {

#ifdef KEYBOARD_CONTROLS
  if (!bUp) {
    if (c == GUI_LEFT)
      bLeftDown = true;
    else if (c == GUI_RIGHT)
      bRightDown = true;
  } else {
    if (c == GUI_LEFT)
      bLeftDown = false;
    else if (c == GUI_RIGHT)
      bRightDown = false;
  }
#endif

  if (bUp)
    return;

  if (pGl->sbCheatsOn.Get()) {
    if (c == '\\') {
      pGl->Next();
      return;
    }

    if (c == 'i')
      bCh = !bCh;

    if (c == 'g')
      pGr->Generate(true);

    if (c == 'l')
      MegaGeneration();

    if (c == 'm')
      pMgGen->MageGenerate();

    if (c == '6')
      std::cout << "Slimes: " << nSlimeNum << "\n";

    if (c == 't')
      t.nTimer = t.nPeriod - 1;

    if (c >= GUI_F1 && c <= GUI_F10)
      for (unsigned i = 0; i < vDr.size(); ++i)
        vDr[i]->AddBonus(vDr[i]->GetBonus(c - GUI_F1 + 1, nBonusCheatTime));
  }

#ifdef PC_VERSION
  if (c == GUI_ESCAPE)
    pGl->Menu();
#endif

  for (unsigned i = 0; i < vDr.size(); ++i)
    if (vDr[i]->bt.IsSpace(c)) {
      // toggle if at base, otherwise shoot!
      if (!vDr[i]->bFly)
        vDr[i]->Toggle();
      else {
        fPoint fFb = vDr[0]->fVel;
        fFb.Normalize(100);
        vDr[0]->Fire(fFb);
      }
    }

#ifdef KEYBOARD_CONTROLS
  // directional shooting using arrow keys
  if (nLastDir == 0) {
    bool flag = true;
    if (c == GUI_LEFT)
      nLastDir = 1;
    else if (c == GUI_RIGHT)
      nLastDir = 2;
    else if (c == GUI_DOWN)
      nLastDir = 3;
    else if (c == GUI_UP)
      nLastDir = 4;
    else
      flag = false;
    if (flag)
      bWasDirectionalInput = true;
  } else {
    int dir = 0;
    if (c == GUI_LEFT)
      dir = 1;
    else if (c == GUI_RIGHT)
      dir = 2;
    else if (c == GUI_DOWN)
      dir = 3;
    else if (c == GUI_UP)
      dir = 4;
    fPoint fp = ComposeDirection(nLastDir, dir);
    fp.x += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;
    fp.y += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;

    if (!vDr[0]->bFly) {
      vDr[0]->Fire(fp);
      pt.UpdateLastDownPosition(Point(fp.x * 10000, fp.y * 10000));
    }
    bWasDirectionalInput = false;
  }
#endif
}

void AdvancedController::OnMouse(Point pPos) {
  Size sz1 = GetProgramInfo().szScreenRez;
  Size sz2 = pGl->pWrp->szActualRez;

  float fX = float(sz1.x) / sz2.x;
  float fY = float(sz1.y) / sz2.y;

  pPos.x *= Crd(fX);
  pPos.y *= Crd(fY);

  pt.UpdateMouse(pPos);
  mc.SetCursorPos(pPos);
}

void AdvancedController::OnMouseDown(Point pPos) {
  Size sz1 = GetProgramInfo().szScreenRez;
  Size sz2 = pGl->pWrp->szActualRez;

  float fX = float(sz1.x) / sz2.x;
  float fY = float(sz1.y) / sz2.y;

  pPos.x *= Crd(fX);
  pPos.y *= Crd(fY);

  pt.UpdateMouse(pPos);
  pt.UpdateLastDownPosition(pPos);
  pt.On();

  if (bPaused)
    return;

  bool bHit = false;

  if (!vDr[0]->bFly)
    bHit = (pt.GetDirection(vDr[0]->pCs->GetPosition()).Length() <
            fTowerClickRadius);
  else
    bHit =
        (pt.GetDirection(vDr[0]->GetPosition()).Length() < fDragonClickRadius);

  if (bHit) {
    if (!vDr[0]->bFly)
      bTakeOffToggle = true;

    vDr[0]->Toggle();
  } else {
    if (!vDr[0]->bFly) {
      fPoint fFb = pt.GetDirection(vDr[0]->GetPosition() + Point(-10, -25));

      fFb.Normalize(100);
      vDr[0]->Fire(fFb);
    }
  }
}

void AdvancedController::OnMouseUp() {
  float fTime = float(pt.Off());
  fTime = fTime / nFramesInSecond;

  if (vDr[0]->bFly && fTime <= .2 && !bTakeOffToggle &&
      pt.GetDirection(vDr[0]->GetPosition()).Length() > vDr[0]->nRadius) {
    fPoint fFb = vDr[0]->fVel;

    fFb.Normalize(100);
    vDr[0]->Fire(fFb);
  }

  if (bTakeOffToggle)
    bTakeOffToggle = false;
}

void AdvancedController::Fire() {
  if (vDr[0]->bFly) {
    fPoint fFb = vDr[0]->fVel;

    fFb.Normalize(100);
    vDr[0]->Fire(fFb);
  } else {
    /* turning off multishoot on tower
    fPoint fFb = pt.GetDirection(vDr[0]->GetPosition() + Point(-10, -25));
    fFb.Normalize(100);
    vDr[0]->Fire(fFb);
    */
  }
}

float AdvancedController::GetCompletionRate() {
  float fCap = 0;
  for (unsigned i = 0; i < vCs.size(); ++i)
    fCap += vCs[i]->nPrincesses;

  fCap /= (4 * vCs.size());

  return fCap;
}

void AdvancedController::MegaGeneration() {
  Point p;
  p.x = GetRandNum(rBound.sz.x);
  p.y = GetRandNum(rBound.sz.y);
  MegaGeneration(p);
}

void AdvancedController::MegaGeneration(Point p) {
  SP<MegaSliminess> pSlm = new MegaSliminess(p, this);
  AddE(pSlm);
}

void MouseCursor::DrawCursor() {
  Index img;
  if (bPressed)
    img = imgCursor.vImage.at(1);
  else
    img = imgCursor.vImage.at(0);

  Size sz = pGl->pGraph->GetImage(img)->GetSize();
  Point p = pCursorPos;

  p.x -= sz.x / 2;
  p.y -= sz.y / 2;

  pGl->pGraph->DrawImage(p, img, false);
}

void MouseCursor::SetCursorPos(Point pPos) { pCursorPos = pPos; }

DragonScoreController::DragonScoreController(SP<TwrGlobalController> pGl_,
                                             Rectangle rBound, Color c,
                                             bool bScoreShow)
    : BasicController(pGl_, rBound, c), t(5 * nFramesInSecond),
      bClickToExit(false) {
  if (bScoreShow) {
    AddV(new HighScoreShower(pGl, rBound));
  } else
    AddV(new IntroTextShower(pGl, rBound));
}

/*virtual*/ void DragonScoreController::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp)
    pGl->Next();
}

void DragonScoreController::DoubleClick() { pGl->Next(); }

/*virtual*/ void DragonScoreController::Update() {
  if (pGl->nHighScore < pGl->nScore) {
    pGl->nHighScore = pGl->nScore;

    std::ofstream ofs("high.txt");
    ofs << pGl->nScore;
    ofs.close();
  }

  BasicController::Update();
}

/*virtual*/ void HighScoreShower::Draw(SP<ScalingDrawer> pDr) {
  int nScale = 2;
  int nCharWidth = 4;

  Point p1 = Point(rBound.sz.x / 2 / nScale, rBound.sz.y / 2 / nScale - 6);
  Point p2 = Point(rBound.sz.x / 2 / nScale, rBound.sz.y / 2 / nScale + 1);

  std::string s1 = "score: ";
  std::string s2 = "high:  ";

  p1.x -= (nCharWidth * (s1.size() + 7)) / 2;

  pGl->pBigNum->DrawWord(s1, p1, false);

  p1.x += nCharWidth * s1.size();

  pGl->pBigNum->DrawNumber(pGl->nScore, p1, 7);

  p2.x -= (nCharWidth * (s2.size() + 7)) / 2;

  pGl->pBigNum->DrawWord(s2, p2, false);

  p2.x += nCharWidth * s2.size();

  pGl->pBigNum->DrawNumber(pGl->nHighScore, p2, 7);

  // pGl->pBigNum->DrawWord("score:", Point(0, 0), false);
  // pGl->pBigNum->DrawWord("high:", Point(0, 0), false);
}

/*virtual*/ void IntroTextShower::Draw(SP<ScalingDrawer> pDr) {
  int nScale = 2;

  Point pCnt = Point(rBound.sz.x / 2 * nScale, rBound.sz.y / 2 * nScale);

  std::vector<std::string> vText;

  vText.push_back("I must bring the royal princesses back to my tower");
  vText.push_back("and defend it from the knights!");
  vText.push_back("---");
#ifdef PC_VERSION
  vText.push_back("CLICK screen to shoot, CLICK tower to fly");
  vText.push_back("CLICK + HOLD to steer");
  vText.push_back("---");
  vText.push_back("press SPACE to PLAY!");
#else

#ifndef SMALL_SCREEN_VERSION
  vText.push_back("TAP screen to shoot, TAP tower to fly");
  vText.push_back("TAP + HOLD to steer");
  vText.push_back("---");
  vText.push_back("double TAP to PLAY!");
#else
  vText.push_back("TAP screen to shoot, TAP tower to fly");
  vText.push_back("TAP + HOLD to steer");
  vText.push_back("OR use the GAMEPAD!");
  vText.push_back("---");
  vText.push_back("double TAP or press X to PLAY!");
#endif

#endif

  int nHeight = pGl->pFancyNum->GetSize(" ").y + 2;

  pCnt.y -= (vText.size() * nHeight) / 2;

  for (unsigned i = 0; i < vText.size(); ++i) {
    pGl->pFancyNum->DrawWord(vText[i], pCnt, true);
    pCnt.y += nHeight;
  }
}

Castle::Castle(Point p, Rectangle rBound_, SP<AdvancedController> pAv_)
    : Critter(15, p, Point(), rBound_, 3, pAv_->pGl->pr("castle")),
      nPrincesses(0), pAv(this, pAv_), pDrag(this, 0), bBroken(false) {}

void Castle::OnKnight(char cWhat) {
  if (pAv->bCh)
    return;

  if (!nPrincesses || cWhat == 'W') {
    if (!bBroken) {
      pAv->pGl->pSnd->PlaySound(pAv->pGl->pr.GetSnd("destroy_castle_sound"));
      pAv->pSc->nTheme = -1;
      Critter::seq = pAv->pGl->pr("destroy_castle");
    }

    if (pAv->tLoseTimer.nPeriod == 0)
      pAv->tLoseTimer = Timer(nFramesInSecond * 3);

    bBroken = true;
    nPrincesses = 0;

    if (pDrag != 0) {

      pDrag->bFly = true;
      pDrag->bTookOff = true;

      pDrag->SimpleVisualEntity::seq = pDrag->imgFly;
      pDrag->SimpleVisualEntity::dPriority = 5;

      pDrag->pCs = 0;

      pDrag->fVel = pAv->pt.GetDirection(GetPosition());
      if (pDrag->fVel.Length() == 0)
        pDrag->fVel = fPoint(0, -1);
      pDrag->fVel.Normalize(pDrag->leash.speed);

      pDrag = 0;
    }

    return;
  }

  if (pDrag != 0) {
    pAv->pGl->pSnd->PlaySound(pAv->pGl->pr.GetSnd("one_princess"));

    --nPrincesses;

    if (cWhat == 'K') {
      fPoint v = RandomAngle();
      v.Normalize(fPrincessSpeed * 3.F);

      SP<Princess> pCr =
          new Princess(Critter(7, GetPosition(), v, rBound, 0,
                               v.x < 0 ? pAv->pGl->pr("princess_f")
                                       : pAv->pGl->pr("princess"),
                               true),
                       pAv);
      pAv->AddBoth(pCr);
      PushBackASSP(pAv.GetRawPointer(), pAv->lsPpl, pCr);
    }
  } else {
    pAv->pGl->pSnd->PlaySound(pAv->pGl->pr.GetSnd("all_princess_escape"));

    if (cWhat == 'K') {
      float r = float(rand()) / RAND_MAX * 2 * 3.1415F;

      for (unsigned i = 0; i < nPrincesses; ++i) {
        fPoint v(sin(r + i * 2 * 3.1415F / nPrincesses),
                 cos(r + i * 2 * 3.1415F / nPrincesses));
        v.Normalize(fPrincessSpeed * 3.F);

        SP<Princess> pCr =
            new Princess(Critter(7, GetPosition(), v, rBound, 0,
                                 v.x < 0 ? pAv->pGl->pr("princess_f")
                                         : pAv->pGl->pr("princess"),
                                 true),
                         pAv);
        pAv->AddBoth(pCr);
        PushBackASSP(pAv.GetRawPointer(), pAv->lsPpl, pCr);
      }
    }

    nPrincesses = 0;
  }
}

/*unsigned*/ void Castle::Draw(SP<ScalingDrawer> pDr) {
  Critter::seq.nActive = nPrincesses;

  if (nPrincesses > 4)
    Critter::seq.nActive = 4;

  if (bBroken) {
    Critter::seq.nActive = pAv->tLoseTimer.nTimer / 2;
    if (seq.nActive > seq.vImage.size() - 1)
      seq.nActive = seq.vImage.size() - 1;
  }

  Critter::Draw(pDr);
}

/*virtual*/ void AdvancedController::Update() {
  CleanUp(lsSlimes);
  CleanUp(lsSliminess);

  pt.Update();

  if (bFirstUpdate) {
    bFirstUpdate = false;

    if (nLvl != 1 && nLvl != 4 && nLvl != 7 && nLvl != 10) {
      vDr[0]->RecoverBonuses();
    }
    pGl->lsBonusesToCarryOver.clear();
  }

  if (bPaused) {
    int nScale = 2;
    int nCharWidth = 4;

    Point p1 = Point(rBound.sz.x / 2 / nScale, rBound.sz.y / 2 / nScale);

    std::string s1 = "paused";

    nCharWidth *= nScale;

    Rectangle r(Point(rBound.sz.x / 2 - (s1.size() * nCharWidth) / 2 - 10,
                      rBound.sz.y / 2 - 10),
                Size(s1.size() * nCharWidth + 18, 21));

    r.p.x *= nScale;
    r.p.y *= nScale;
    r.sz.x *= nScale;
    r.sz.y *= nScale;

    pGl->pGraph->DrawRectangle(r, Color(0, 0, 0), false);
    pGl->pBigNum->DrawWord(s1, p1, true);
    pGl->pGraph->RefreshAll();

    return;
  }

  if (tLoseTimer.nPeriod == 0) {
    if (!bGhostTime) {
      if (nLvl <= 3)
        pSc->nTheme = BG_BACKGROUND;
      else if (nLvl <= 6)
        pSc->nTheme = BG_BACKGROUND2;
      else
        pSc->nTheme = BG_BACKGROUND3;
    } else {
      if (nLvl <= 3)
        pSc->nTheme = BG_SLOW_BACKGROUND;
      else if (nLvl <= 6)
        pSc->nTheme = BG_SLOW_BACKGROUND2;
      else
        pSc->nTheme = BG_SLOW_BACKGROUND3;
    }
  }

  BasicController::Update();

#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  mc.bPressed = pt.bPressed;
  mc.DrawCursor();
#endif
#endif
  pGl->pGraph->RefreshAll();

  if (tLoseTimer.nPeriod != 0 && tLoseTimer.Tick()) {
    pGl->nActive = pGl->vCnt.size() - 2;
    return;
  }

  tr.Update();
  if (!vDr[0]->bFly) {
    if (tr.IsTrigger()) {
      fPoint p = tr.GetMovement();

      if (p.Length() > 50) {
        if (p.Length() > 250)
          p.Normalize(250);

        vDr[0]->Fire(p);
      }
    }
  } else {
    if (pt.bPressed) {
      fPoint v = vDr[0]->fVel;
      fPoint d = pt.GetDirection(vDr[0]->GetPosition());

      if (d.Length() == 0)
        d = v;

      d.Normalize(v.Length());

      vDr[0]->fVel = v * fFlightCoefficient + d;
      vDr[0]->fVel.Normalize(vDr[0]->leash.speed);
    } else if (bLeftDown || bRightDown) {
      fPoint v = vDr[0]->fVel;
      fPoint d(v.y, v.x);
      // left precedents right
      if (bLeftDown)
        d.y *= -1;
      else
        d.x *= -1;
      vDr[0]->fVel = v * fFlightCoefficient * 1.2f + d;
      vDr[0]->fVel.Normalize(vDr[0]->leash.speed);
    }
  }

#ifdef FULL_VERSION
  if (!bGhostTime) {
    if (t.Tick()) {
      bGhostTime = true;

      if (!pGl->sbMusicOn.Get())
        pGl->pSnd->PlaySound(pGl->pr.GetSnd("E"));

      if (nLvl > 6)
        pGr->Generate(true);
    }
  }

  if (!bTimerFlash) {
    if (t.nPeriod - t.nTimer < 20 * nFramesInSecond) {
      bTimerFlash = true;
      tBlink = Timer(nFramesInSecond / 2);
    }
  } else {
    if (tBlink.Tick()) {
      if (!pGl->sbMusicOn.Get() && !bGhostTime && !bBlink)
        pGl->pSnd->PlaySound(pGl->pr.GetSnd("D"));

      bBlink = !bBlink;
    }
  }

#endif

#ifdef KEYBOARD_CONTROLS
  if (!bWasDirectionalInput)
    nLastDir = 0;
  else {
    fPoint fp = ComposeDirection(nLastDir, nLastDir);
    fp.x += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;
    fp.y += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;

    if (!vDr[0]->bFly) {
      vDr[0]->Fire(fp);
      pt.UpdateLastDownPosition(Point(fp.x * 10000, fp.y * 10000));
    }
  }

  bWasDirectionalInput = false;
#endif
}

/*virtual*/ void Slime::Update() {
  if (t.Tick() && float(rand()) / RAND_MAX < .25)
    RandomizeVelocity();

  for (std::list<ASSP<ConsumableEntity>>::iterator itr = pAc->lsPpl.begin();
       itr != pAc->lsPpl.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    if (this->HitDetection(*itr)) {
      if ((*itr)->GetType() == 'K') {
        pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("slime_poke"));

        bExist = false;

        SP<AnimationOnce> pAn =
            new AnimationOnce(dPriority, pAc->pGl->pr("slime_poke"),
                              nFramesInSecond / 5, GetPosition(), true);
        pAc->AddBoth(pAn);

        break;
      }
    }
  }

  Critter::Update(); // can walk off the screen
}

/*virtual*/ void Slime::OnHit(char cWhat) {
  if (pAc->nSlimeNum >= nSlimeMax && cWhat != 'M') {
    std::vector<Point> vDeadSlimes;

    for (std::list<ASSP<Slime>>::iterator itr = pAc->lsSlimes.begin();
         itr != pAc->lsSlimes.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      vDeadSlimes.push_back((*itr)->GetPosition());
      (*itr)->OnHit('M');
    }

    for (std::list<ASSP<Sliminess>>::iterator itr = pAc->lsSliminess.begin();
         itr != pAc->lsSliminess.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      vDeadSlimes.push_back((*itr)->GetPosition());
      (*itr)->Kill();
    }

    if (vDeadSlimes.empty())
      throw SimpleException("No slimes found!");

    fPoint fAvg(0, 0);
    for (unsigned i = 0; i < vDeadSlimes.size(); ++i) {
      fAvg += vDeadSlimes[i];
    }

    fAvg /= float(vDeadSlimes.size());

    pAc->MegaGeneration(fAvg.ToPnt());

    for (unsigned i = 0; i < vDeadSlimes.size(); ++i) {
      SP<FloatingSlime> pSlm =
          new FloatingSlime(pAc->pGl->pr("slime_cloud"), vDeadSlimes[i],
                            fAvg.ToPnt(), nFramesInSecond * 1);
      pAc->AddBoth(pSlm);
    }

    return;
  }

  bExist = false;

  bool bRevive =
      (cWhat != 'M'); // pAc->nSlimeNum < nSlimeMax;// && nGeneration != 0;

  if (cWhat != 'M') {
    SP<BonusScore> pB = new BonusScore(pAc, GetPosition(), 1);
    pAc->AddBoth(pB);
  }

  SP<AnimationOnce> pAn = new AnimationOnce(
      dPriority, pAc->pGl->pr(bRevive ? "slime_die" : "slime_poke"),
      nFramesInSecond / 5, GetPosition(), true);
  pAc->AddBoth(pAn);

  if (!bRevive)
    return;

  for (int i = 0; i < 2; ++i) {
    fPoint f = RandomAngle();
    f.Normalize(4);

    SP<Sliminess> pSlm =
        new Sliminess(GetPosition() + f.ToPnt(), pAc, false, nGeneration + 1);
    pAc->AddE(pSlm);
    PushBackASSP(pAc.GetRawPointer(), pAc->lsSliminess, pSlm);
  }
}

/*virtual*/ void BonusScore::Draw(SP<ScalingDrawer> pDr) {
  if (nC < 11)
    pAc->pGl->pNum->DrawWord(sText, p, true);
  else
    pAc->pGl->pNum->DrawColorWord(sText, p, c, true);
}

void BonusScore::Update() {
  if (pAc->bGhostTime)
    bExist = false;

  if (t.Tick()) {
    ++nC;
    if (nC < 11) {
      nScoreSoFar += nScore / 11;
      pAc->pGl->nScore += nScore / 11;
      --p.y;
    } else {
      c.R -= 50;
      c.G -= 50;
      // c.nTransparent -= 50;
    }

    if (nC == 11) {
      pAc->pGl->nScore += nScore - nScoreSoFar;
    }

    if (nC >= 15) {
      bExist = false;
    }
  }
}

bool AreWeFullScreen() {
  std::ifstream ifs(sFullScreenPath);

  bool bRet;
  ifs >> bRet;

  if (ifs.fail())
    return false;

  return bRet;
}

ProgramInfo GetProgramInfo() {
  ProgramInfo inf;

  // inf.szScreenRez = Size(640, 480);
  // inf.szScreenRez = Size(1024, 768);
  // inf.szScreenRez = Size(960, 640);

#ifdef SMALL_SCREEN_VERSION
  inf.szScreenRez = Size(854, 480);
#else
  inf.szScreenRez = Size(960, 600);
#endif

  inf.strTitle = "Tower Defense";
  inf.nFramerate = 1000 / nFramesInSecond;
  inf.bMouseCapture = false;

  static bool bFullScreen = AreWeFullScreen();

  if (bFullScreen) {
    inf.bFullScreen = true;
    inf.bFlexibleResolution = true;
    inf.bBlackBox = true;
  }

  return inf;
}

SP<GlobalController> GetGlobalController(ProgramEngine pe) {
  return new TowerGameGlobalController(pe);
}

TowerGameGlobalController::TowerGameGlobalController(ProgramEngine pe) {
  pData = new TowerDataWrap(pe);
}

TowerGameGlobalController::~TowerGameGlobalController() {
  SP_Info *pCleanUp = pData->pCnt.GetRawPointer();
  delete pData;
  CleanIslandSeeded(pCleanUp);
}

void ReadLevels(std::string sFile, Rectangle rBound, LevelStorage &vLvl) {
  std::istringstream istr;

  std::ifstream ifs(sFile.c_str());

  LevelLayout l(rBound);
  while (ifs >> l) {
    l.Convert();
    vLvl.push_back(l);
  }

  if (vLvl.size() == 0)
    throw SimpleException("<global>", "ReadLevels",
                          "Cannot read levels at " + sFile);
}

TowerDataWrap::TowerDataWrap(ProgramEngine pe) {
  szActualRez = pe.szActualRez;

  pExitProgram = pe.pExitProgram;

  pWr = pe.pMsg.GetRawPointer();

  {
    std::ifstream ifs("config.txt");
    if (ifs.fail())
      throw SimpleException("TowerDataWrap", "<constructor>",
                            "Need config.txt file!");
    ifs >> fp;
  }

  Rectangle sBound = Rectangle(pe.szScreenRez);
  unsigned nScale = 2;
  Rectangle rBound =
      Rectangle(0, 0, sBound.sz.x / nScale, sBound.sz.y / nScale);

  pGr = pe.pGr;
  pSm = pe.pSndMng;

  pDr = new ScalingDrawer(pGr, nScale);

  SP<ScalingDrawer> pBigDr = new ScalingDrawer(pGr, nScale * 2);

  std::string sPath = "dragonfont\\";
  fp.Parse(sPath);

  pNum = new NumberDrawer(pDr, sPath, "dragonfont");
  pBigNum = new NumberDrawer(pBigDr, sPath, "dragonfont");
  pFancyNum = new FontWriter(fp, "dragonfont\\dragonfont2.txt", pGr, 2);

#ifdef FULL_VERSION
#ifdef SMALL_SCREEN_VERSION
  ReadLevels(fp.sPath + "levels_small.txt", rBound, vLvl);
#else
  ReadLevels(fp.sPath + "levels.txt", rBound, vLvl);
#endif
#else
  ReadLevels(fp.sPath + "levels_trial.txt", rBound, vLvl);
#endif

  pCnt = new TwrGlobalController(pDr, pNum, pBigNum, pFancyNum, pSm, vLvl,
                                 rBound, this, fp);
  pCnt->StartUp();
}

void TowerGameGlobalController::Update() {
  pData->pCnt->vCnt[pData->pCnt->nActive]->Update();
}

void TowerGameGlobalController::KeyDown(GuiKeyType nCode) {
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnKey(nCode, false);

#ifndef PC_VERSION
  if (nCode == GUI_ESCAPE)
    Trigger(pData->pExitProgram);
#endif
}

void TowerGameGlobalController::KeyUp(GuiKeyType nCode) {
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnKey(nCode, true);
}

void TowerGameGlobalController::MouseMove(Point pPos) {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnMouse(pPos);
#endif
#endif
}

void TowerGameGlobalController::MouseDown(Point pPos) {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnMouseDown(pPos);
#endif
#endif
}

void TowerGameGlobalController::MouseUp() {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnMouseUp();
#endif
#endif
}

void TowerGameGlobalController::DoubleClick() {
  pData->pCnt->vCnt[pData->pCnt->nActive]->DoubleClick();
}

void TowerGameGlobalController::Fire() {
  pData->pCnt->vCnt[pData->pCnt->nActive]->Fire();
}

/*virutal*/ void SkellyGenerator::Update() {
  if (t.Tick()) {
    bExist = false;

    unsigned n = unsigned(rand() % pAdv->vCs.size());

    fPoint v = pAdv->vCs[n]->GetPosition() - p;
    v.Normalize(fSkeletonSpeed);

    SP<Knight> pCr = new Knight(
        Critter(7, p, v, pAdv->rBound, 3, pAdv->pGl->pr("skelly"), true), pAdv,
        'S');
    pAdv->AddBoth(pCr);
    PushBackASSP(pAdv.GetRawPointer(), pAdv->lsPpl, pCr);
  }
}

/*virtual*/ void Mage::Update() {
  if (bAngry) {
    if (!bCasting) {
      unsigned i = 0;
      for (; i < pAc->vCs.size(); ++i) {
        fPoint p = pAc->vCs[i]->GetPosition() - fPos;
        if (p.Length() < nSummonRadius)
          break;
      }
      if (i == pAc->vCs.size())
        if (rand() % nSummonChance == 0) {
          bCasting = true;
          Critter::seq = pAc->pGl->pr("mage_spell");
          Critter::fVel = fPoint(0, 0);
        }
    } else {
      if (tSpellAnimate.Tick()) {
        Critter::seq.Toggle();
      }

      if (tSpell.UntilTick() == int(1.F * nFramesInSecond)) {
        SummonSkeletons(pAc, GetPosition());
      }

      if (tSpell.Tick()) {
        bCasting = false;
        Critter::fVel = fMvVel;
        Critter::seq =
            fMvVel.x < 0 ? pAc->pGl->pr("mage_f") : pAc->pGl->pr("mage");
      }
    }
  }

  Critter::Update();
}

void Mage::SummonSlimes() {
  for (int i = 0; i < 2; ++i) {
    fPoint f = RandomAngle();
    f.Normalize(10);

    SP<Sliminess> pSlm = new Sliminess(GetPosition() + f.ToPnt(), pAc, true, 0);
    pAc->AddE(pSlm);
  }
}

/*virutal*/ void Ghostiness::Update() {
  if (t.Tick()) {
    bExist = false;

    if (nGhostHit == 0)
      return;

    SP<Knight> pCr = new Knight(knCp, pAdv, 'G');
    if (nGhostHit == 1)
      pCr->seq = pAdv->pGl->pr("ghost");
    else
      pCr->seq = pAdv->pGl->pr("ghost_knight");
    pCr->nGhostHit = nGhostHit - 1;

    pAdv->AddBoth(pCr);
    PushBackASSP(pAdv.GetRawPointer(), pAdv->lsPpl, pCr);
  }
}

/*virtual*/ void Knight::OnHit(char cWhat) {
  if (cType == 'W') {
    KnockBack();
    if (nGolemHealth > 0) {
      --nGolemHealth;
      pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("hit_golem"));
      return;
    }

    pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("golem_death"));

    // pAc->pGr->Generate(true);
    SP<BonusScore> pB = new BonusScore(pAc, GetPosition(), 5000);
  }

  bExist = false;

  pAc->tutOne.KnightKilled();

  if (cType != 'G') {
    SP<BonusScore> pB = new BonusScore(pAc, GetPosition(), 100);
    pAc->AddBoth(pB);

    ImageSequence seqDead = pAc->pGl->pr("knight_die");

    if (cType == 'S')
      seqDead = pAc->pGl->pr("skelly_die");
    else if (cType == 'W') {
      if (this->fVel.x < 0)
        seqDead = pAc->pGl->pr("golem_die");
      else
        seqDead = pAc->pGl->pr("golem_die_f");
    }

    SP<AnimationOnce> pAn = new AnimationOnce(
        dPriority, seqDead, unsigned(nFramesInSecond / 5 / fDeathMultiplier),
        GetPosition(), true);
    pAc->AddBoth(pAn);
  } else {
    SP<Ghostiness> pGhs = new Ghostiness(GetPosition(), pAc, *this, nGhostHit);
    pAc->AddE(pGhs);
  }
}

/*virtual*/ void MegaSlime::OnHit(char cWhat) {
  if (nHealth > 0) {
    --nHealth;
    pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_hit"));
    return;
  }

  bExist = false;

  SP<BonusScore> pB = new BonusScore(pAc, GetPosition(), 500);
  pAc->AddBoth(pB);

  ImageSequence seqDead = pAc->pGl->pr("megaslime_die");
  pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_die"));

  SP<AnimationOnce> pAn = new AnimationOnce(
      dPriority, seqDead, unsigned(nFramesInSecond / 5 / fDeathMultiplier),
      GetPosition(), true);
  pAc->AddBoth(pAn);
}

void SlimeUpdater::Draw(SP<ScalingDrawer> pDr) { pBuy->DrawSlimes(); }

void MenuController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;
  if (c == GUI_ESCAPE) {
    pMenuDisplay->Escape();
    return;
  }
  /*
  else if(c == '1')
  {
          pGl->Restart();
  }
  else if(c == '2')
  {
          pGl->pSnd->Toggle();
          std::cout << "Sound toggled\n";
  }
  else if(c == '3')
  {
          pGl->plr.ToggleOff();
          std::cout << "Music toggled\n";
  }
  else if(c == '4')
  {
          BoolToggle(pGl->sbTutorialOn);
          std::cout << "Tutorial toggled\n";
  }
  */
  else if (c == GUI_UP) {
    pMenuDisplay->PositionIncrement(false);
  } else if (c == GUI_DOWN) {
    pMenuDisplay->PositionIncrement(true);
  } else if (c == GUI_RETURN || c == ' ') {
    pMenuDisplay->Boop();
  }
}

void MenuController::OnMouse(Point pPos) {}

void MenuController::OnMouseDown(Point pPos) {}

void MenuController::Update() {
  BasicController::Update();

  if (pMenuDisplay->pCurr == &(pMenuDisplay->memOptions)) {
    if (pOptionText != 0)
      pOptionText->Draw(pGl->pDr);
  } else {
    if (pHintText != 0)
      pHintText->Draw(pGl->pDr);
  }

  pGl->pGraph->RefreshAll();
}

MenuDisplay::MenuDisplay(Point pLeftTop_, SP<NumberDrawer> pNum_,
                         SP<Animation> pMenuCaret_,
                         SP<MenuController> pMenuController_,
                         bool bCheatsUnlocked_)
    : pLeftTop(pLeftTop_), pNum(this, pNum_), pMenuCaret(this, pMenuCaret_),
      pMenuController(this, pMenuController_), pCurr(&memMain),
      bCheatsUnlocked(bCheatsUnlocked_) {
  Size szSpacing(50, 10);
  memMain.vEntries.push_back(
      MenuEntry(szSpacing, "continue", &MenuDisplay::Continue));
  memMain.vEntries.push_back(
      MenuEntry(szSpacing, "restart", &MenuDisplay::Restart));
  memMain.vEntries.push_back(
      MenuEntry(szSpacing, "load chapter", &MenuDisplay::LoadChapterSubmenu));
  memMain.vEntries.push_back(
      MenuEntry(szSpacing, "options", &MenuDisplay::OptionsSubmenu));
  memMain.vEntries.push_back(MenuEntry(szSpacing, "exit", &MenuDisplay::Exit));

  memLoadChapter.vEntries.push_back(
      MenuEntry(szSpacing, "chapter 1", &MenuDisplay::Chapter1));
  memLoadChapter.vEntries.push_back(
      MenuEntry(szSpacing, "chapter 2", &MenuDisplay::Chapter2));
  memLoadChapter.vEntries.push_back(
      MenuEntry(szSpacing, "chapter 3", &MenuDisplay::Chapter3));
  memLoadChapter.vEntries.push_back(
      MenuEntry(szSpacing, "back", &MenuDisplay::Escape));

  memOptions.vEntries.push_back(
      MenuEntry(szSpacing, "", &MenuDisplay::MusicToggle));
  nMusic = memOptions.vEntries.size() - 1;

  memOptions.vEntries.push_back(
      MenuEntry(szSpacing, "", &MenuDisplay::SoundToggle));
  nSound = memOptions.vEntries.size() - 1;

  memOptions.vEntries.push_back(
      MenuEntry(szSpacing, "", &MenuDisplay::TutorialToggle));
  nTutorial = memOptions.vEntries.size() - 1;

  memOptions.vEntries.push_back(
      MenuEntry(szSpacing, "", &MenuDisplay::FullScreenToggle));
  nFullScreen = memOptions.vEntries.size() - 1;

  if (bCheatsUnlocked) {
    memOptions.vEntries.push_back(
        MenuEntry(szSpacing, "", &MenuDisplay::CheatsToggle));
    nCheats = memOptions.vEntries.size() - 1;
  } else
    nCheats = -1;

  memOptions.vEntries.push_back(
      MenuEntry(szSpacing, "back", &MenuDisplay::Escape));

  vOptionText.resize(memOptions.vEntries.size());

  if (bCheatsUnlocked)
    vOptionText.at(nCheats) =
        "f1-f10 to get bonuses\n\\ to skip level\ni for invincibility";

  UpdateMenuEntries();
}

void MenuDisplay::PositionIncrement(bool bUp) {
  pMenuController->pGl->pSnd->PlaySound(pMenuController->pGl->pr.GetSnd("B"));

  int nDelta = bUp ? 1 : -1;

  while (true) {
    pCurr->nMenuPosition += nDelta;
    pCurr->nMenuPosition += pCurr->vEntries.size();
    pCurr->nMenuPosition %= pCurr->vEntries.size();

    if (pCurr->nMenuPosition == 0 ||
        !pCurr->vEntries[pCurr->nMenuPosition].bDisabled)
      break;
  }

  if (pMenuController->pOptionText != 0)
    pMenuController->pOptionText->SetText(
        vOptionText.at(memOptions.nMenuPosition));
}

void MenuDisplay::Boop() {
  if (pCurr->vEntries.at(pCurr->nMenuPosition).pTriggerEvent ==
      &MenuDisplay::Escape)
    pMenuController->pGl->pSnd->PlaySound(pMenuController->pGl->pr.GetSnd("C"));
  else
    pMenuController->pGl->pSnd->PlaySound(pMenuController->pGl->pr.GetSnd("A"));
  (this->*(pCurr->vEntries.at(pCurr->nMenuPosition).pTriggerEvent))();
}

void MenuDisplay::Restart() { pMenuController->pGl->Restart(); }

void MenuDisplay::Continue() {
  pMenuController->pGl->nActive = pMenuController->nResumePosition;
}

void MenuDisplay::MusicToggle() {
  pMenuController->pGl->plr.ToggleOff();
  pMenuController->pGl->sbMusicOn.Set(!pMenuController->pGl->plr.bOff);
  UpdateMenuEntries();
}

void MenuDisplay::SoundToggle() {
  pMenuController->pGl->pSnd->Toggle();
  pMenuController->pGl->sbSoundOn.Set(pMenuController->pGl->pSnd->Get());
  UpdateMenuEntries();
}

void MenuDisplay::TutorialToggle() {
  BoolToggle(pMenuController->pGl->sbTutorialOn);
  UpdateMenuEntries();
}

void MenuDisplay::FullScreenToggle() {
  BoolToggle(pMenuController->pGl->sbFullScreen);
  UpdateMenuEntries();
}

void MenuDisplay::CheatsToggle() {
  BoolToggle(pMenuController->pGl->sbCheatsOn);
  UpdateMenuEntries();
}

void MenuDisplay::Exit() { Trigger(pMenuController->pGl->pWrp->pExitProgram); }

void MenuDisplay::Escape() {
  if (pCurr != &memMain)
    pCurr = &memMain;
  else
    Continue();
}

void MenuDisplay::LoadChapterSubmenu() { pCurr = &memLoadChapter; }

void MenuDisplay::OptionsSubmenu() { pCurr = &memOptions; }

void MenuDisplay::UpdateMenuEntries() {
  memOptions.vEntries.at(nMusic).sText =
      MusicString() + OnOffString(pMenuController->pGl->sbMusicOn.Get());
  memOptions.vEntries.at(nSound).sText =
      SoundString() + OnOffString(pMenuController->pGl->sbSoundOn.Get());
  memOptions.vEntries.at(nTutorial).sText =
      TutorialString() + OnOffString(pMenuController->pGl->sbTutorialOn.Get());

  bool bFullScreenNow = GetProgramInfo().bFullScreen;
  bool bFullScreenSetting = pMenuController->pGl->sbFullScreen.Get();
  std::string sExtra = "";
  if (bFullScreenNow != bFullScreenSetting)
    sExtra = "changes will take effect next launch";

  memOptions.vEntries.at(nFullScreen).sText =
      FullTextString() + OnOffString(bFullScreenSetting); // + sExtra;

  vOptionText.at(nFullScreen) = sExtra;

  bool bCheatsOn = pMenuController->pGl->sbCheatsOn.Get();

  if (bCheatsUnlocked)
    memOptions.vEntries.at(nCheats).sText = "cheats: " + OnOffString(bCheatsOn);

  memLoadChapter.vEntries.at(1).bDisabled =
      (pMenuController->pGl->snProgress.Get() < 1);
  memLoadChapter.vEntries.at(2).bDisabled =
      (pMenuController->pGl->snProgress.Get() < 2);

  if (pMenuController->pOptionText != 0)
    pMenuController->pOptionText->SetText(
        vOptionText.at(memOptions.nMenuPosition));
}

void MenuDisplay::Chapter1() {
  pMenuController->pGl->Restart(pMenuController->pGl->vLevelPointers.at(0));
}

void MenuDisplay::Chapter2() {
  pMenuController->pGl->Restart(pMenuController->pGl->vLevelPointers.at(1));
}

void MenuDisplay::Chapter3() {
  pMenuController->pGl->Restart(pMenuController->pGl->vLevelPointers.at(2));
}
