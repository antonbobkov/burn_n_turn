#include "game/dragon_game_runner.h"
#include "game_utils/MessageWriter.h"
#include "game/controller/dragon_game_controller.h"
#include "game/controller/game_controller_interface.h"
#include "game/dragon_macros.h"
#include "game/fireball.h"
#include "game/level.h"
#include "game_utils/event.h"
#include "utils/smart_pointer.h"
#include "wrappers/font_writer.h"

MessageWriter *pWr = 0;

DragonGameRunner::DragonGameRunner(ProgramEngine const& pe) {
  pData = new TowerDataWrap(pe);
}

DragonGameRunner::~DragonGameRunner() { delete pData; }

TowerDataWrap::TowerDataWrap(ProgramEngine const& pe) {
  szActualRez = pe.szActualRez;

  pExitProgram = pe.pExitProgram.get();

  pWr = pe.pMsg.get();

  p_fm_ = pe.GetFileManager();
  {
    std::unique_ptr<InStreamHandler> ih = p_fm_->ReadFile("config.txt");
    fp_ = FilePath::CreateFromStream(ih->GetStream(), p_fm_);
  }

  Rectangle sBound = Rectangle(pe.szScreenRez);
  unsigned nScale = 2;
  Rectangle rBound =
      Rectangle(0, 0, sBound.sz.x / nScale, sBound.sz.y / nScale);

  pGr = pe.pGr;
  pSm = pe.pSndMng;

  pDr = make_smart(new ScalingDrawer(pGr, nScale));

  smart_pointer<ScalingDrawer> pBigDr =
      make_smart(new ScalingDrawer(pGr, nScale * 2));

  std::string sFontPath = "dragonfont\\";

  pNum = make_smart(new NumberDrawer(pDr, fp_.get(), sFontPath, "dragonfont"));
  pBigNum =
      make_smart(new NumberDrawer(pBigDr, fp_.get(), sFontPath, "dragonfont"));
  pFancyNum = make_smart(
      new FontWriter(fp_.get(), "dragonfont\\dragonfont2.txt", pGr, 2));

  std::string levelsFile;
#ifdef FULL_VERSION
#ifdef SMALL_SCREEN_VERSION
  levelsFile = "levels_small.txt";
#else
  levelsFile = "levels.txt";
#endif
#else
  levelsFile = "levels_trial.txt";
#endif
  ReadLevels(fp_.get(), levelsFile, rBound, vLvl);

  pCnt = std::make_unique<DragonGameController>(
      pDr, pNum, pBigNum, pFancyNum, pSm, vLvl, rBound, this, fp_.get());
  pCnt->StartUp(pCnt.get());
}

DragonGameController *DragonGameRunner::GetTowerController() const {
  return pData ? pData->pCnt.get() : nullptr;
}

unsigned DragonGameRunner::GetActiveControllerIndex() const {
  DragonGameController *twr = GetTowerController();
  return twr ? twr->nActive : 0;
}

unsigned DragonGameRunner::GetControllerCount() const {
  DragonGameController *twr = GetTowerController();
  return twr ? twr->vCnt.size() : 0;
}

std::string DragonGameRunner::GetActiveControllerName() const {
  DragonGameController *twr = GetTowerController();
  if (!twr || twr->nActive >= twr->vCnt.size())
    return "";
  return twr->vCnt[twr->nActive]->GetControllerName();
}

void DragonGameRunner::Update() {
  pData->pCnt->vCnt[pData->pCnt->nActive]->Update();
}

void DragonGameRunner::KeyDown(GuiKeyType nCode) {
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnKey(nCode, false);

#ifndef PC_VERSION
  if (nCode == GUI_ESCAPE)
    Trigger(pData->pExitProgram);
#endif
}

void DragonGameRunner::KeyUp(GuiKeyType nCode) {
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnKey(nCode, true);
}

void DragonGameRunner::MouseMove(Point pPos) {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnMouse(pPos);
#endif
#endif
}

void DragonGameRunner::MouseDown(Point pPos) {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnMouseDown(pPos);
#endif
#endif
}

void DragonGameRunner::MouseUp() {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnMouseUp();
#endif
#endif
}

void DragonGameRunner::DoubleClick() {
  pData->pCnt->vCnt[pData->pCnt->nActive]->DoubleClick();
}

void DragonGameRunner::Fire() {
  pData->pCnt->vCnt[pData->pCnt->nActive]->Fire();
}
