#include "game/game.h"
#include "utils/smart_pointer.h"
#include "game_utils/image_sequence.h"

#include "wrappers/color.h"
#include <memory>

SimpleController::SimpleController(DragonGameControllerList *pGraph,
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

FlashingController::FlashingController(
    DragonGameControllerList *pGraph, std::string strFileName,
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

void EntityListController::AddV(smart_pointer<VisualEntity> pVs) {
  lsDraw.push_back(pVs);
}

void EntityListController::AddE(smart_pointer<EventEntity> pEv) {
  lsUpdate.push_back(pEv);
}

void EntityListController::AddOwnedVisualEntity(
    std::unique_ptr<VisualEntity> p) {
  VisualEntity *raw = p.get();
  owned_entities.push_back(std::unique_ptr<Entity>(p.release()));
  owned_visual_entities.push_back(raw);
}

void EntityListController::AddOwnedEventEntity(std::unique_ptr<EventEntity> p) {
  EventEntity *raw = p.get();
  owned_entities.push_back(std::unique_ptr<Entity>(p.release()));
  owned_event_entities.push_back(raw);
}

void EntityListController::AddBackground(Color c) {
  Rectangle r = rBound.sz;
  r.sz.x *= pGl->pDr->nFactor;
  r.sz.y *= pGl->pDr->nFactor;

  smart_pointer<StaticRectangle> pBkg =
      make_smart(new StaticRectangle(r, c, -1.F));

  AddV(pBkg);
}

EntityListController::EntityListController(
    DragonGameControllerList *pGl_, Rectangle rBound, Color c)
    : GameController(pGl_, rBound), bNoRefresh(false) {
  AddBackground(c);
}

void EntityListController::Update() {
  CleanUp(lsUpdate);
  CleanUp(lsDraw);
  CleanUp(lsPpl);
  CleanUp(owned_visual_entities);
  CleanUp(owned_event_entities);
  CleanUp(owned_entities);

  for (auto &p : lsUpdate) {
    if (p->bExist)
      p->Move();
  }

  for (EventEntity *pEx : owned_event_entities) {
    if (pEx->bExist)
      pEx->Move();
  }

  for (EventEntity *pEx : GetNonOwnedUpdateEntities()) {
    if (pEx->bExist)
      pEx->Move();
  }

  for (auto &p : lsUpdate) {
    if (p->bExist)
      p->Update();
  }

  for (EventEntity *pEx : owned_event_entities) {
    if (pEx->bExist)
      pEx->Update();
  }

  for (EventEntity *pEx : GetNonOwnedUpdateEntities()) {
    if (pEx->bExist)
      pEx->Update();
  }

  {
    typedef std::multimap<ScreenPos, VisualEntity *> DrawMap;
    DrawMap mmp;

    for (auto &p : lsDraw) {
      if (p->bExist)
        mmp.insert(std::pair<ScreenPos, VisualEntity *>(
            ScreenPos(p->GetPriority(), p->GetPosition()), p.get()));
    }

    for (VisualEntity *pOw : owned_visual_entities) {
      if (pOw->bExist)
        mmp.insert(std::pair<ScreenPos, VisualEntity *>(
            ScreenPos(pOw->GetPriority(), pOw->GetPosition()), pOw));
    }

    for (VisualEntity *pEx : GetNonOwnedDrawEntities()) {
      if (pEx->bExist)
        mmp.insert(std::pair<ScreenPos, VisualEntity *>(
            ScreenPos(pEx->GetPriority(), pEx->GetPosition()), pEx));
    }

    for (auto &entry : mmp)
      entry.second->Draw(pGl->pDr);
  }

  if (!bNoRefresh)
    pGl->pGraph->RefreshAll();
}

void EntityListController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;

  pGl->Next();
}

void EntityListController::OnMouseDown(Point pPos) { pGl->Next(); }

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

BuyNowController::BuyNowController(DragonGameControllerList *pGl_,
                                   Rectangle rBound, Color c)
    : EntityListController(pGl_, rBound, c), t(120), nSlimeCount(50),
      tVel(nFramesInSecond / 2) {
  for (int i = 0; i < nSlimeCount; i++) {
    mSlimes.push_back(make_smart(
        new Animation(0, pGl->pr("slime"), nFramesInSecond / 10,
                      Point(rBound.sz.x / 2, rBound.sz.y / 2 + 25), true)));
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
  EntityListController::Update();

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
    EntityListController::OnKey(c, bUp);
}

void BuyNowController::OnMouseDown(Point pPos) {
  if (t < 0)
    EntityListController::OnMouseDown(pPos);
}

Cutscene::Cutscene(DragonGameControllerList *pGl_, Rectangle rBound_,
                   std::string sRun, std::string sChase, bool bFlip)
    : EntityListController(pGl_, rBound_, Color(0, 0, 0)), pCrRun(),
      pCrFollow(), bRelease(false), tm(nFramesInSecond / 5), Beepy(true) {
  ImageSequence seq1 = pGl_->pr(sRun);

  int xPos = 5;
  if (bFlip)
    xPos = rBound_.sz.x - 5;
  float m = 1;
  if (bFlip)
    m = -1;

  smart_pointer<FancyCritter> pCr1 = make_smart(
      new FancyCritter(7, fPoint(xPos, rBound_.sz.y / 2), fPoint(m * 10, 0),
                       rBound, 3, seq1, nFramesInSecond / 5));
  AddBoth(pCr1);

  pCrRun = pCr1;

  ImageSequence seq2 = pGl_->pr(sChase);

  smart_pointer<FancyCritter> pCr2 = make_smart(
      new FancyCritter(7, fPoint(xPos, rBound_.sz.y / 2), fPoint(m * 12, 0),
                       rBound, 3, seq2, nFramesInSecond / 5));

  pCrFollow = pCr2;
}

void Cutscene::Update() {
  if (!bRelease && pCrRun->GetPosition().x >= rBound.sz.x / 3 &&
      pCrRun->GetPosition().x <= rBound.sz.x * 2 / 3) {
    bRelease = true;

    smart_pointer<FancyCritter> pCr2 = pCrFollow;
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

  EntityListController::Update();
}

void Cutscene::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp && c == '\\' && pGl->sbCheatsOn.Get())
    pGl->Next();
}

void DragonScoreController::OnMouseDown(Point pPos) {
  if (bClickToExit)
    pGl->Next();
}

DragonScoreController::DragonScoreController(
    DragonGameControllerList *pGl_, Rectangle rBound, Color c,
    bool bScoreShow)
    : EntityListController(pGl_, rBound, c), t(5 * nFramesInSecond),
      bClickToExit(false) {
  if (bScoreShow) {
    AddV(make_smart(new HighScoreShower(pGl, rBound)));
  } else
    AddV(make_smart(new IntroTextShower(pGl, rBound)));
}

void DragonScoreController::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp)
    pGl->Next();
}

void DragonScoreController::DoubleClick() { pGl->Next(); }

void DragonScoreController::Update() {
  if (pGl->nHighScore < pGl->nScore) {
    pGl->nHighScore = pGl->nScore;

    FilePath *fp = pGl->pWrp->GetFilePath();
    if (fp) {
      std::unique_ptr<Gui::OutStreamHandler> oh = fp->WriteFile("high.txt");
      if (oh)
        oh->GetStream() << pGl->nScore;
    }
  }

  EntityListController::Update();
}

void AutoAdvanceController::Update() {
  CleanUp(lsUpdate);
  CleanUp(lsDraw);

  if (lsDraw.size() == 1) {
    pGl->Next();
    return;
  }

  EntityListController::Update();
}

void AutoAdvanceController::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp)
    pGl->Next();
}

void AutoAdvanceController::OnMouseDown(Point pPos) { pGl->Next(); }
