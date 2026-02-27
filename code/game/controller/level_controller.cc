#include "game/controller/level_controller.h"
#include "game/controller/dragon_game_controller.h"
#include "game/critter_generators.h"
#include "game/critters.h"
#include "game/dragon.h"
#include "game/dragon_constants.h"
#include "game/dragon_game_runner.h"
#include "game/dragon_macros.h"
#include "game/entities.h"
#include "game/fireball.h"
#include "game/level.h"
#include "game/tutorial.h"
#include "game_utils/draw_utils.h"
#include "game_utils/game_runner_interface.h"
#include "game_utils/image_sequence.h"
#include "game_utils/sound_utils.h"
#include "utils/random_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/color.h"
#include "wrappers/geometry.h"

/* Local helpers for level UI: score/time/level and bonus icons. */
struct AdNumberDrawer : public VisualEntity {
  LevelController *pAd;

  AdNumberDrawer() : pAd(0) {}
  explicit AdNumberDrawer(LevelController *ad) : pAd(ad) {}

  /*virtual*/ void Draw(ScalingDrawer *pDr) {
    unsigned n = pDr->nFactor;

#ifdef FULL_VERSION
    pAd->pGl->GetNumberDrawer()->DrawNumber(
        pAd->pGl->GetScore(), Point(pAd->rBound.sz.x - 27 * 4, 4), 7);

    if (pAd->bBlink) {
      Color c(255, 255, 0);

      if (pAd->bTimerFlash)
        c = Color(255, 0, 0);

      pAd->pGl->GetNumberDrawer()->DrawColorNumber(
          (pAd->t.nPeriod - pAd->t.nTimer) / nFramesInSecond,
          Point(pAd->rBound.sz.x - 14 * 4, 4), c, 4);
      pAd->pGl->GetNumberDrawer()->DrawColorWord(
          "time:", Point(pAd->rBound.sz.x - 19 * 4, 4), c);
    }

    pAd->pGl->GetNumberDrawer()->DrawNumber(
        pAd->nLvl, Point(pAd->rBound.sz.x - 3 * 4, 4), 2);

    pAd->pGl->GetNumberDrawer()->DrawWord("score:",
                                          Point(pAd->rBound.sz.x - 33 * 4, 4));
    pAd->pGl->GetNumberDrawer()->DrawWord("level:",
                                          Point(pAd->rBound.sz.x - 9 * 4, 4));
    if (pAd->bCh) {
      pAd->pGl->GetNumberDrawer()->DrawColorWord(
          "invincible", Point(pAd->rBound.sz.x - 44 * 4, 4), Color(0, 255, 0));
    }
#else
    pAd->pGl->GetNumberDrawer()->DrawNumber(
        pAd->pGl->GetScore(), Point(pAd->rBound.sz.x - 17 * 4, 4), 7);
    pAd->pGl->GetNumberDrawer()->DrawNumber(
        pAd->nLvl, Point(pAd->rBound.sz.x - 3 * 4, 4), 2);

    pAd->pGl->GetNumberDrawer()->DrawWord("score:",
                                          Point(pAd->rBound.sz.x - 23 * 4, 4));
    pAd->pGl->GetNumberDrawer()->DrawWord("level:",
                                          Point(pAd->rBound.sz.x - 9 * 4, 4));
#endif
  }
  /*virtual*/ Point GetPosition() { return Point(); }
  /*virtual*/ float GetPriority() { return 10; }
};

struct BonusDrawer : public VisualEntity {
  LevelController *pAd;

  Timer t;
  int nAnimationCounter;

  BonusDrawer()
      : pAd(0), t(unsigned(nFramesInSecond * .1F)), nAnimationCounter(0) {}
  explicit BonusDrawer(LevelController *ad)
      : pAd(ad), t(unsigned(nFramesInSecond * .1F)), nAnimationCounter(0) {}

  /*virtual*/ void Draw(ScalingDrawer *pDr) {
    if (t.Tick())
      ++nAnimationCounter;

    Point p(1, 3);

    for (unsigned nDr = 0; nDr < pAd->vDr.size(); ++nDr) {
      auto &lst = pAd->vDr[nDr]->lsBonuses;

      for (auto itr = lst.begin(), etr = lst.end(); itr != etr; ++itr) {
        TimedFireballBonus *pBns = itr->get();

        if (pBns->t.nPeriod &&
            (pBns->t.nPeriod - pBns->t.nTimer) < 4 * nFramesInSecond)
          if (nAnimationCounter % 2 == 1) {
            p.x += 10;
            continue;
          }

        ImageSequence img = pAd->pGl->GetImgSeq(GetBonusImage(pBns->nNum));

        pDr->Draw(img.vImage[nAnimationCounter % img.vImage.size()], p, false);

        p.x += 10;
      }

      p.y += 10;

      p.x = 3;

      for (int i = 0; i < pAd->vDr[nDr]->nFireballCount; ++i) {
        pDr->Draw(pAd->pGl->GetImgSeq("fireball_icon").GetImage(), p, false);

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

LevelController::LevelController(DragonGameController *pGl_, Rectangle rBound,
                                 Color c, const LevelLayout &lvl)
    : EntityListController(pGl_, rBound, c), bCh(false), nLvl(lvl.nLvl),
      nSlimeNum(0), bFirstUpdate(true), bLeftDown(false), bRightDown(false),
      nLastDir(0), bWasDirectionalInput(0), bGhostTime(false), bBlink(true),
      pGr(0), bLeft(false), bTakeOffToggle(false),
      tutOne(std::make_unique<TutorialLevelOne>()),
      tutTwo(std::make_unique<TutorialLevelTwo>()), pTutorialText(),
      mc(pGl->GetImgSeq("claw"), Point()), bTimerFlash(false) {}

Dragon *LevelController::FindDragon(Dragon *p) {
  for (size_t i = 0; i < vDr.size(); ++i)
    if (vDr[i].get() == p)
      return vDr[i].get();
  return nullptr;
}

void LevelController::Init(LevelController *pSelf_, const LevelLayout &lvl) {
  pSelf = pSelf_;
  bNoRefresh = true;

  tLoseTimer.nPeriod = 0;

  AddOwnedVisualEntity(std::make_unique<AdNumberDrawer>(pSelf));
  AddOwnedVisualEntity(std::make_unique<BonusDrawer>(pSelf));

  pKnightGen = std::make_unique<KnightGenerator>(lvl.vFreq.at(0), rBound, pSelf,
                                                 lvl.blKnightGen);
  pPGen = std::make_unique<PrincessGenerator>(lvl.vFreq.at(1), rBound, pSelf);
  pTGen = std::make_unique<TraderGenerator>(lvl.vFreq.at(2), rBound, pSelf);
  pMGen = std::make_unique<MageGenerator>(lvl.vFreq.at(3), lvl.vFreq.at(4),
                                          rBound, pSelf);

  pGr = pKnightGen.get();
  pMgGen = pMGen.get();

  unsigned i;
  for (i = 0; i < lvl.vRoadGen.size(); ++i)
    vRd.push_back(std::make_unique<FancyRoad>(lvl.vRoadGen[i], pSelf));

  for (i = 0; i < lvl.vCastleLoc.size(); ++i)
    vCs.push_back(std::make_unique<Castle>(lvl.vCastleLoc[i], rBound, pSelf));

  t = Timer(lvl.nTimer);

  vDr.push_back(std::make_unique<Dragon>(
      vCs[0].get(), pSelf, pGl->GetImgSeq("dragon_stable"),
      pGl->GetImgSeq("dragon_fly"),
      ButtonSet('q', 'w', 'e', 'd', 'c', 'x', 'z', 'a', ' ')));
  if (vDr.back()->pCs != nullptr)
    vDr.back()->pCs->pDrag = vDr.back().get();

  Point pos(pGl->GetBounds().sz.x / 2, pGl->GetBounds().sz.y);
  pTutorialText =
      std::make_unique<TutorialTextEntity>(1, pos, pGl->GetNumberDrawer(), pGl);

#ifdef PC_VERSION

  if (nLvl == 1) {
    tutOne->pTexter = pTutorialText.get();
    tutOne->Update();
  }

  if (nLvl == 2) {
    tutTwo->pTexter = pTutorialText.get();
    tutTwo->Update();
  }

#endif
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

  if (pGl->AreCheatsOnSetting()) {
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
    pGl->EnterMenu();
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
  Size sz2 = pGl->GetActualResolution();

  float fX = float(sz1.x) / sz2.x;
  float fY = float(sz1.y) / sz2.y;

  pPos.x *= Crd(fX);
  pPos.y *= Crd(fY);

  pt.UpdateMouse(pPos);
  mc.SetCursorPos(pPos);
}

void LevelController::OnMouseDown(Point pPos) {
  Size sz1 = GetProgramInfo().szScreenRez;
  Size sz2 = pGl->GetActualResolution();

  float fX = float(sz1.x) / sz2.x;
  float fY = float(sz1.y) / sz2.y;

  pPos.x *= Crd(fX);
  pPos.y *= Crd(fY);

  pt.UpdateMouse(pPos);
  pt.UpdateLastDownPosition(pPos);
  pt.On();

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

void LevelController::AddBonusAnimation(
    std::unique_ptr<FireballBonusAnimation> p) {
  lsBonus.push_back(std::move(p));
}

std::vector<FireballBonusAnimation *> LevelController::GetBonusAnimations() {
  std::vector<FireballBonusAnimation *> out;
  for (auto &u : lsBonus)
    out.push_back(u.get());
  return out;
}

void LevelController::AddSlime(std::unique_ptr<Slime> p) {
  lsSlimes.push_back(std::move(p));
}

void LevelController::AddSliminess(std::unique_ptr<Sliminess> p) {
  lsSliminess.push_back(std::move(p));
}

void LevelController::AddMegaSlime(std::unique_ptr<MegaSlime> p) {
  lsMegaSlimes.push_back(std::move(p));
}

void LevelController::AddMegaSliminess(std::unique_ptr<MegaSliminess> p) {
  lsMegaSliminess.push_back(std::move(p));
}

void LevelController::AddSpawnedGenerator(std::unique_ptr<EventEntity> p) {
  lsSpawnedGenerators.push_back(std::move(p));
}

std::vector<ConsumableEntity *> LevelController::GetConsumablePointers() {
  std::vector<ConsumableEntity *> out =
      EntityListController::GetConsumablePointers();
  for (auto &u : lsSlimes)
    out.push_back(u.get());
  for (auto &u : lsMegaSlimes)
    out.push_back(u.get());
  return out;
}

std::vector<EventEntity *> LevelController::GetNonOwnedUpdateEntities() {
  std::vector<EventEntity *> out;
  for (size_t i = 0; i < vCs.size(); ++i)
    out.push_back(vCs[i].get());
  if (pTutorialText)
    out.push_back(pTutorialText.get());
  if (pSc)
    out.push_back(pSc.get());
  for (auto &u : lsBonus)
    out.push_back(u.get());
  for (auto &u : lsSlimes)
    out.push_back(u.get());
  for (auto &u : lsMegaSlimes)
    out.push_back(u.get());
  for (auto &u : lsSliminess)
    out.push_back(u.get());
  for (auto &u : lsMegaSliminess)
    out.push_back(u.get());
  for (size_t i = 0; i < vDr.size(); ++i)
    out.push_back(vDr[i].get());
  for (size_t i = 0; i < vDr.size(); ++i)
    for (auto &u : vDr[i]->lsBonuses)
      out.push_back(u.get());
  if (pKnightGen)
    out.push_back(pKnightGen.get());
  if (pPGen)
    out.push_back(pPGen.get());
  if (pTGen)
    out.push_back(pTGen.get());
  if (pMGen)
    out.push_back(pMGen.get());
  for (auto &u : lsSpawnedGenerators)
    out.push_back(u.get());
  return out;
}

std::vector<VisualEntity *> LevelController::GetNonOwnedDrawEntities() {
  std::vector<VisualEntity *> out;
  for (size_t i = 0; i < vCs.size(); ++i)
    out.push_back(vCs[i].get());
  for (size_t i = 0; i < vRd.size(); ++i)
    out.push_back(vRd[i].get());
  if (pTutorialText)
    out.push_back(pTutorialText.get());
  for (auto &u : lsBonus)
    out.push_back(u.get());
  for (auto &u : lsSlimes)
    out.push_back(u.get());
  for (auto &u : lsMegaSlimes)
    out.push_back(u.get());
  for (size_t i = 0; i < vDr.size(); ++i)
    out.push_back(vDr[i].get());
  /* Sliminess / MegaSliminess are EventEntity only, not VisualEntity */
  return out;
}

void LevelController::MegaGeneration() {
  Point p;
  p.x = GetRandNum(rBound.sz.x);
  p.y = GetRandNum(rBound.sz.y);
  MegaGeneration(p);
}

void LevelController::MegaGeneration(Point p) {
  AddMegaSliminess(std::make_unique<MegaSliminess>(p, pSelf));
}

/*virtual*/ void LevelController::Update() {
  CleanUp(lsBonus);
  CleanUp(lsSlimes);
  CleanUp(lsMegaSlimes);
  CleanUp(lsSliminess);
  CleanUp(lsMegaSliminess);
  CleanUp(lsSpawnedGenerators);

  pt.Update();

  if (bFirstUpdate) {
    bFirstUpdate = false;

    if (nLvl != 1 && nLvl != 4 && nLvl != 7 && nLvl != 10) {
      vDr[0]->RecoverBonuses();
    }
    pGl->ClearBonusesToCarryOver();
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
  mc.DrawCursor(pGl->GetGraphics());
#endif
#endif
  pGl->RefreshAll();

  if (tLoseTimer.nPeriod != 0 && tLoseTimer.Tick()) {
    pGl->ShowGameOverScreen();
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

      if (!pGl->IsMusicOnSetting())
        pGl->PlaySound("E");

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
      if (!pGl->IsMusicOnSetting() && !bGhostTime && !bBlink)
        pGl->PlaySound("D");

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
