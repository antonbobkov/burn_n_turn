#include "basic_controllers.h"
#include "dragon_game_controller.h"
#include "game_controller_interface.h"
#include "../dragon_constants.h"
#include "../dragon_game_runner.h"
#include "../entities.h"
#include "../level.h"
#include "../../game_utils/game_runner_interface.h"
#include "../../game_utils/image_sequence.h"
#include "../../wrappers/color.h"
#include "../../wrappers/geometry.h"
#include "../../wrappers/gui_key_type.h"
#include <memory>

std::vector<ConsumableEntity *> EntityListController::GetConsumablePointers() {
  std::vector<ConsumableEntity *> out;
  for (auto &p : lsPpl)
    out.push_back(p.get());
  return out;
}

void EntityListController::AddOwnedEntity(std::unique_ptr<Entity> p) {
  Entity *raw = p.get();
  owned_entities.push_back(std::move(p));
  owned_entity_list.push_back(raw);
}

void EntityListController::AddBackground(Color c) {
  Rectangle r = rBound.sz;
  r.sz.x *= pGl->GetDrawScaleFactor();
  r.sz.y *= pGl->GetDrawScaleFactor();

  AddOwnedEntity(std::make_unique<StaticRectangle>(r, c, -1.F));
}

EntityListController::EntityListController(DragonGameController *pGl_,
                                           Rectangle rBound, Color c)
    : GameController(pGl_, rBound), bNoRefresh(false) {
  AddBackground(c);
}

void EntityListController::Update() {
  /* Clean raw-pointer view first so no raw ptr outlives its owning unique_ptr.
   * lsPpl and owned_entities are cleaned after their raw-ptr view. */
  CleanUp(owned_entity_list);
  CleanUp(lsPpl);
  CleanUp(owned_entities);

  auto nonOwned = GetNonOwnedEntities();

  for (Entity *pEx : owned_entity_list) {
    if (pEx->bExist)
      pEx->Move();
  }

  for (Entity *pEx : nonOwned) {
    if (pEx->bExist)
      pEx->Move();
  }

  for (Entity *pEx : owned_entity_list) {
    if (pEx->bExist)
      pEx->Update();
  }

  for (Entity *pEx : nonOwned) {
    if (pEx->bExist)
      pEx->Update();
  }

  {
    typedef std::multimap<ScreenPos, Entity *> DrawMap;
    DrawMap mmp;

    for (Entity *pOw : owned_entity_list) {
      if (pOw->bExist && pOw->ShouldDraw())
        mmp.insert(std::pair<ScreenPos, Entity *>(
            ScreenPos(pOw->GetPriority(), pOw->GetPosition()), pOw));
    }

    for (Entity *pEx : nonOwned) {
      if (pEx && pEx->bExist && pEx->ShouldDraw())
        mmp.insert(std::pair<ScreenPos, Entity *>(
            ScreenPos(pEx->GetPriority(), pEx->GetPosition()), pEx));
    }

    for (auto &entry : mmp)
      entry.second->Draw(pGl->GetDrawer());
  }

  if (!bNoRefresh)
    pGl->RefreshAll();
}

void EntityListController::OnKey(GuiKeyType /*c*/, bool bUp) {
  if (bUp)
    return;

  pGl->Next();
}

void EntityListController::OnMouseDown(Point /*pPos*/) { pGl->Next(); }

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
      pCrFollow(), tm(nFramesInSecond / 5), Beepy(true), bRelease(false) {
  ImageSequence seq1 = pGl_->GetImgSeq(sRun);

  int xPos = 5;
  if (bFlip)
    xPos = rBound_.sz.x - 5;
  float m = 1;
  if (bFlip)
    m = -1;

  pCrRun = std::make_unique<FancyCritter>(
      7, fPoint(xPos, rBound_.sz.y / 2), fPoint(m * 10, 0), rBound, 3, seq1,
      nFramesInSecond / 5);

  ImageSequence seq2 = pGl_->GetImgSeq(sChase);

  pCrFollow = std::make_unique<FancyCritter>(
      7, fPoint(xPos, rBound_.sz.y / 2), fPoint(m * 12, 0), rBound, 3, seq2,
      nFramesInSecond / 5);
}

Cutscene::~Cutscene() = default;

void Cutscene::Update() {
  if (!bRelease && pCrRun->GetPosition().x >= rBound.sz.x / 3 &&
      pCrRun->GetPosition().x <= rBound.sz.x * 2 / 3) {
    bRelease = true;
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

std::vector<Entity *> Cutscene::GetNonOwnedEntities() {
  std::vector<Entity *> out;
  if (pCrRun)
    out.push_back(pCrRun.get());
  if (bRelease && pCrFollow)
    out.push_back(pCrFollow.get());
  return out;
}

void Cutscene::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp && c == '\\' && pGl->AreCheatsOnSetting())
    pGl->Next();
}

void DragonScoreController::OnMouseDown(Point /*pPos*/) {
  if (bClickToExit)
    pGl->Next();
}

DragonScoreController::DragonScoreController(DragonGameController *pGl_,
                                             Rectangle rBound, Color c,
                                             bool bScoreShow)
    : EntityListController(pGl_, rBound, c), t(5 * nFramesInSecond),
      bClickToExit(false) {
  if (bScoreShow) {
    AddOwnedEntity(std::make_unique<HighScoreShower>(pGl, rBound));
  } else
    AddOwnedEntity(std::make_unique<IntroTextShower>(pGl, rBound));
}

void DragonScoreController::OnKey(GuiKeyType /*c*/, bool bUp) {
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
  /* Clean raw-ptr view before counting so dead entities don't count. */
  CleanUp(owned_entity_list);

  int nDrawable = 0;
  for (Entity *p : owned_entity_list)
    if (p->ShouldDraw())
      ++nDrawable;

  if (nDrawable == 1) {
    pGl->Next();
    return;
  }

  EntityListController::Update();
}

void AutoAdvanceController::OnKey(GuiKeyType /*c*/, bool bUp) {
  if (!bUp)
    pGl->Next();
}

void AutoAdvanceController::OnMouseDown(Point /*pPos*/) { pGl->Next(); }
