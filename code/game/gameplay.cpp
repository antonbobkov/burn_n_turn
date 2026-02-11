#include "game.h"

void BackgroundMusicPlayer::ToggleOff() {
  if (!bOff) {
    bOff = true;
    StopMusic();
  } else {
    bOff = false;
  }
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

BonusScore::BonusScore(smart_pointer<AdvancedController> pAc_, Point p_,
                       unsigned nScore_)
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

struct AdNumberDrawer : public VisualEntity {
  SSP<AdvancedController> pAd;

  AdNumberDrawer() : pAd(this, 0) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr) {
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

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr) {
    if (t.Tick())
      ++nAnimationCounter;

    Point p(1, 3);

    for (unsigned nDr = 0; nDr < pAd->vDr.size(); ++nDr) {
      BonusList &lst = pAd->vDr[nDr]->lsBonuses;

      for (BonusList::iterator itr = lst.begin(), etr = lst.end(); itr != etr;
           ++itr) {

        smart_pointer<TimedFireballBonus> pBns = *itr;

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

AdvancedController::AdvancedController(smart_pointer<TwrGlobalController> pGl_,
                                       Rectangle rBound, Color c,
                                       const LevelLayout &lvl)
    : BasicController(pGl_, rBound, c), bCh(false), nLvl(lvl.nLvl),
      nSlimeNum(0), bPaused(false), bFirstUpdate(true), bLeftDown(false),
      bRightDown(false), nLastDir(0), bWasDirectionalInput(0),
      bGhostTime(false), bBlink(true), pGr(0), bLeft(false), pSc(),
      bTakeOffToggle(false), pTutorialText(this, 0),
      mc(pGl->pr("claw"), Point(), pGl.GetRawPointer()), bTimerFlash(false) {}

void AdvancedController::Init(smart_pointer<AdvancedController> pSelf_,
                              const LevelLayout &lvl) {
  pSelf = pSelf_;
  bNoRefresh = true;

  tLoseTimer.nPeriod = 0;

  smart_pointer<AdNumberDrawer> pNm = make_smart(new AdNumberDrawer());
  pNm->pAd = pSelf.GetRawPointer();
  AddV(pNm);

  smart_pointer<BonusDrawer> pBd = make_smart(new BonusDrawer());
  pBd->pAd = pSelf.GetRawPointer();
  AddV(pBd);

  smart_pointer<KnightGenerator> pGen = make_smart(
      new KnightGenerator(lvl.vFreq.at(0), rBound, pSelf, lvl.blKnightGen));
  smart_pointer<PrincessGenerator> pPGen =
      make_smart(new PrincessGenerator(lvl.vFreq.at(1), rBound, pSelf));
  smart_pointer<TraderGenerator> pTGen =
      make_smart(new TraderGenerator(lvl.vFreq.at(2), rBound, pSelf));
  smart_pointer<MageGenerator> pMGen = make_smart(
      new MageGenerator(lvl.vFreq.at(3), lvl.vFreq.at(4), rBound, pSelf));

  pGr = pGen.GetRawPointer();
  pMgGen = pMGen.GetRawPointer();

  unsigned i;
  for (i = 0; i < lvl.vRoadGen.size(); ++i)
    PushBackASSP(pSelf.GetRawPointer(), vRd,
                 make_smart(new FancyRoad(lvl.vRoadGen[i], pSelf)));

  for (i = 0; i < lvl.vCastleLoc.size(); ++i)
    PushBackASSP(pSelf.GetRawPointer(), vCs,
                 make_smart(new Castle(lvl.vCastleLoc[i], rBound, pSelf)));

  t = Timer(lvl.nTimer);

  PushBackASSP(
      pSelf.GetRawPointer(), vDr,
      make_smart(new Dragon(
          vCs[0], pSelf, pGl->pr("dragon_stable"), pGl->pr("dragon_fly"),
          ButtonSet('q', 'w', 'e', 'd', 'c', 'x', 'z', 'a', ' '))));

  Point pos(pGl->rBound.sz.x / 2, pGl->rBound.sz.y);
  smart_pointer<TutorialTextEntity> pTT = make_smart(new TutorialTextEntity(
      1, pos, pGl->pNum, pGl->sbTutorialOn.GetConstPointer()));
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
  smart_pointer<MegaSliminess> pSlm = make_smart(new MegaSliminess(p, pSelf));
  AddE(pSlm);
}

/*virtual*/ void HighScoreShower::Draw(smart_pointer<ScalingDrawer> pDr) {
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

/*virtual*/ void IntroTextShower::Draw(smart_pointer<ScalingDrawer> pDr) {
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

/*virtual*/ void BonusScore::Draw(smart_pointer<ScalingDrawer> pDr) {
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
