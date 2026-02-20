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
  pFancyNum = std::make_unique<FontWriter>(
      fp_.get(), "dragonfont\\dragonfont2.txt", pGr, 2);

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
      pDr, pNum, pBigNum, pFancyNum.get(), pSm, vLvl, rBound, this, fp_.get());
  pCnt->StartUp(pCnt.get());
}

DragonGameController *DragonGameRunner::GetTowerController() const {
  return pData ? pData->pCnt.get() : nullptr;
}

std::string DragonGameRunner::GetActiveControllerName() const {
  DragonGameController *twr = GetTowerController();
  return twr ? twr->GetActiveControllerName() : "";
}

bool DragonGameRunner::IsOnGameOverScreen() const {
  DragonGameController *twr = GetTowerController();
  return twr && twr->IsOnGameOverScreen();
}

void DragonGameRunner::Update() {
  GameController *p = GetTowerController() ? GetTowerController()->GetActiveController() : nullptr;
  if (p)
    p->Update();
}

void DragonGameRunner::KeyDown(GuiKeyType nCode) {
  GameController *p = GetTowerController() ? GetTowerController()->GetActiveController() : nullptr;
  if (p)
    p->OnKey(nCode, false);

#ifndef PC_VERSION
  if (nCode == GUI_ESCAPE)
    Trigger(pData->pExitProgram);
#endif
}

void DragonGameRunner::KeyUp(GuiKeyType nCode) {
  GameController *p = GetTowerController() ? GetTowerController()->GetActiveController() : nullptr;
  if (p)
    p->OnKey(nCode, true);
}

void DragonGameRunner::MouseMove(Point pPos) {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  GameController *p = GetTowerController() ? GetTowerController()->GetActiveController() : nullptr;
  if (p)
    p->OnMouse(pPos);
#endif
#endif
}

void DragonGameRunner::MouseDown(Point pPos) {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  GameController *p = GetTowerController() ? GetTowerController()->GetActiveController() : nullptr;
  if (p)
    p->OnMouseDown(pPos);
#endif
#endif
}

void DragonGameRunner::MouseUp() {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  GameController *p = GetTowerController() ? GetTowerController()->GetActiveController() : nullptr;
  if (p)
    p->OnMouseUp();
#endif
#endif
}

void DragonGameRunner::DoubleClick() {
  GameController *p = GetTowerController() ? GetTowerController()->GetActiveController() : nullptr;
  if (p)
    p->DoubleClick();
}

void DragonGameRunner::Fire() {
  GameController *p = GetTowerController() ? GetTowerController()->GetActiveController() : nullptr;
  if (p)
    p->Fire();
}
