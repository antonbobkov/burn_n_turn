#include "game/controller/basic_controllers.h"
#include "game/controller/dragon_game_controller.h"
#include "game/controller/game_controller_interface.h"
#include "game/dragon_constants.h"
#include "game/dragon_game_runner.h"
#include "game/entities.h"
#include "game/level.h"
#include "game_utils/game_runner_interface.h"
#include "game_utils/image_sequence.h"
#include "utils/smart_pointer.h"
#include "wrappers/color.h"
#include "wrappers/geometry.h"
#include "wrappers/gui_key_type.h"
#include <memory>

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
  r.sz.x *= pGl->GetDrawScaleFactor();
  r.sz.y *= pGl->GetDrawScaleFactor();

  smart_pointer<StaticRectangle> pBkg =
      make_smart(new StaticRectangle(r, c, -1.F));

  AddV(pBkg);
}

EntityListController::EntityListController(DragonGameController *pGl_,
                                           Rectangle rBound, Color c)
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
      entry.second->Draw(pGl->GetDrawer());
  }

  if (!bNoRefresh)
    pGl->RefreshAll();
}

void EntityListController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;

  pGl->Next();
}

void EntityListController::OnMouseDown(Point pPos) { pGl->Next(); }

void StartScreenController::Next() {
  pGl->Next();
  pGl->PlaySound("start_game");
}

void StartScreenController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;

  if (c == GUI_ESCAPE)
    pGl->EnterMenu();
  else
    Next();
}

Cutscene::Cutscene(DragonGameController *pGl_, Rectangle rBound_,
                   std::string sRun, std::string sChase, bool bFlip)
    : EntityListController(pGl_, rBound_, Color(0, 0, 0)), pCrRun(),
      pCrFollow(), bRelease(false), tm(nFramesInSecond / 5), Beepy(true) {
  ImageSequence seq1 = pGl_->GetImgSeq(sRun);

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

  ImageSequence seq2 = pGl_->GetImgSeq(sChase);

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
      pGl->PlaySound("beep");
    else
      pGl->PlaySound("boop");

    Beepy = !Beepy;
  }

  EntityListController::Update();
}

void Cutscene::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp && c == '\\' && pGl->AreCheatsOnSetting())
    pGl->Next();
}

void DragonScoreController::OnMouseDown(Point pPos) {
  if (bClickToExit)
    pGl->Next();
}

DragonScoreController::DragonScoreController(DragonGameController *pGl_,
                                             Rectangle rBound, Color c,
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
  if (pGl->GetHighScore() < pGl->GetScore()) {
    pGl->UpdateHighScoreIfNeeded();
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
