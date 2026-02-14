#include "game.h"
#include "controller/level_controller.h"

#include "color.h"
#include "common.h"
#include "core.h"
#include "critters.h"
#include "dragon.h"
#include "fireball.h"
#include "game_runner_interface.h"
#include "level.h"
#include "smart_pointer.h"
#include "tutorial.h"

/* Local helpers for level UI: score/time/level and bonus icons. */
struct AdNumberDrawer : public VisualEntity {
  LevelController *pAd;

  AdNumberDrawer() : pAd(0) {}

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
  typedef std::list<smart_pointer<TimedFireballBonus>> BonusList;

  LevelController *pAd;

  Timer t;
  int nAnimationCounter;

  BonusDrawer()
      : pAd(0), t(unsigned(nFramesInSecond * .1F)), nAnimationCounter(0) {}

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

static const float fSpreadFactor = 2.0f;

LevelController::LevelController(DragonGameControllerList *pGl_,
                                 Rectangle rBound, Color c,
                                 const LevelLayout &lvl)
    : EntityListController(pGl_, rBound, c), bCh(false), nLvl(lvl.nLvl),
      nSlimeNum(0), bPaused(false), bFirstUpdate(true), bLeftDown(false),
      bRightDown(false), nLastDir(0), bWasDirectionalInput(0),
      bGhostTime(false), bBlink(true), pGr(0), bLeft(false), pSc(),
      bTakeOffToggle(false), pTutorialText(),
      mc(pGl->pr("claw"), Point()), bTimerFlash(false) {}

smart_pointer<Dragon> LevelController::FindDragon(Dragon *p) {
  for (size_t i = 0; i < vDr.size(); ++i)
    if (vDr[i].get() == p)
      return vDr[i];
  return smart_pointer<Dragon>();
}

void LevelController::Init(LevelController *pSelf_, const LevelLayout &lvl) {
  pSelf = pSelf_;
  bNoRefresh = true;

  tLoseTimer.nPeriod = 0;

  smart_pointer<AdNumberDrawer> pNm = make_smart(new AdNumberDrawer());
  pNm->pAd = pSelf;
  AddV(pNm);

  smart_pointer<BonusDrawer> pBd = make_smart(new BonusDrawer());
  pBd->pAd = pSelf;
  AddV(pBd);

  smart_pointer<KnightGenerator> pGen = make_smart(
      new KnightGenerator(lvl.vFreq.at(0), rBound, pSelf, lvl.blKnightGen));
  smart_pointer<PrincessGenerator> pPGen =
      make_smart(new PrincessGenerator(lvl.vFreq.at(1), rBound, pSelf));
  smart_pointer<TraderGenerator> pTGen =
      make_smart(new TraderGenerator(lvl.vFreq.at(2), rBound, pSelf));
  smart_pointer<MageGenerator> pMGen = make_smart(
      new MageGenerator(lvl.vFreq.at(3), lvl.vFreq.at(4), rBound, pSelf));

  pGr = pGen.get();
  pMgGen = pMGen.get();

  unsigned i;
  for (i = 0; i < lvl.vRoadGen.size(); ++i)
    vRd.push_back(
        make_smart(new FancyRoad(lvl.vRoadGen[i], pSelf)));

  for (i = 0; i < lvl.vCastleLoc.size(); ++i)
    vCs.push_back(
        std::make_unique<Castle>(lvl.vCastleLoc[i], rBound, pSelf));

  t = Timer(lvl.nTimer);

  vDr.push_back(make_smart(new Dragon(
      vCs[0].get(), pSelf, pGl->pr("dragon_stable"), pGl->pr("dragon_fly"),
      ButtonSet('q', 'w', 'e', 'd', 'c', 'x', 'z', 'a', ' '))));
  if (vDr.back()->pCs != nullptr)
    vDr.back()->pCs->pDrag = vDr.back();

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

  for (i = 0; i < vRd.size(); ++i)
    AddV(vRd[i]);
  for (i = 0; i < vDr.size(); ++i)
    AddBoth(vDr[i]);
}

/*virtual*/ void LevelController::OnKey(GuiKeyType c, bool bUp) {

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
      if (!vDr[i]->bFly)
        vDr[i]->Toggle();
      else {
        fPoint fFb = vDr[0]->fVel;
        fFb.Normalize(100);
        vDr[0]->Fire(fFb);
      }
    }

#ifdef KEYBOARD_CONTROLS
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

void LevelController::OnMouse(Point pPos) {
  Size sz1 = GetProgramInfo().szScreenRez;
  Size sz2 = pGl->pWrp->szActualRez;

  float fX = float(sz1.x) / sz2.x;
  float fY = float(sz1.y) / sz2.y;

  pPos.x *= Crd(fX);
  pPos.y *= Crd(fY);

  pt.UpdateMouse(pPos);
  mc.SetCursorPos(pPos);
}

void LevelController::OnMouseDown(Point pPos) {
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

void LevelController::OnMouseUp() {
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

void LevelController::Fire() {
  if (vDr[0]->bFly) {
    fPoint fFb = vDr[0]->fVel;

    fFb.Normalize(100);
    vDr[0]->Fire(fFb);
  }
}

float LevelController::GetCompletionRate() {
  float fCap = 0;
  for (unsigned i = 0; i < vCs.size(); ++i)
    fCap += vCs[i]->nPrincesses;

  fCap /= (4 * vCs.size());

  return fCap;
}

std::vector<EventEntity *> LevelController::GetNonOwnedUpdateEntities() {
  std::vector<EventEntity *> out;
  for (size_t i = 0; i < vCs.size(); ++i)
    out.push_back(vCs[i].get());
  return out;
}

std::vector<VisualEntity *> LevelController::GetNonOwnedDrawEntities() {
  std::vector<VisualEntity *> out;
  for (size_t i = 0; i < vCs.size(); ++i)
    out.push_back(vCs[i].get());
  return out;
}

void LevelController::MegaGeneration() {
  Point p;
  p.x = GetRandNum(rBound.sz.x);
  p.y = GetRandNum(rBound.sz.y);
  MegaGeneration(p);
}

void LevelController::MegaGeneration(Point p) {
  smart_pointer<MegaSliminess> pSlm =
      make_smart(new MegaSliminess(p, pSelf));
  AddE(pSlm);
}

/*virtual*/ void LevelController::Update() {
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

  EntityListController::Update();

#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  mc.bPressed = pt.bPressed;
  mc.DrawCursor(pGl->pGraph.get());
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
