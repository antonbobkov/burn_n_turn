#include "basic_controllers.h"
#include "dragon_game_controller.h"
#include "../dragon_constants.h"
#include "../entities.h"
#include "../../game_utils/image_sequence.h"
#include <memory>

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

  if (!pCrFollow->IsAlive()) {
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
  if (CountDrawable() == 1) {
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
