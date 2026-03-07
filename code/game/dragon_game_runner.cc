#include "dragon_game_runner.h"
#include "../game_utils/MessageWriter.h"
#include "controller/dragon_game_controller.h"
#include "controller/game_controller_interface.h"
#include "dragon_macros.h"
#include "fireball.h"
#include "level.h"
#include "../game_utils/event.h"
#include "../utils/configuration_file.h"
#include "../utils/file_utils.h"
#include "../utils/smart_pointer.h"
#include "../wrappers/font_writer.h"

MessageWriter *pWr = 0;

DragonGameRunner::DragonGameRunner(ProgramEngine const& pe) {
  szActualRez = pe.szActualRez;

  pExitProgram = pe.pExitProgram.get();

  pWr = pe.pMsg.get();

  p_fm_ = pe.GetFileManager();
  config_ = std::make_unique<ConfigurationFile>(p_fm_, "config.txt");
  {
    std::string systemVal = config_->GetEntry("SYSTEM");
    bool inLinux = (systemVal == "linux" || systemVal == "1");
    std::string pathVal = config_->GetEntry("PATH");
    if (pathVal.empty())
      pathVal = ".";
    fp_ = FilePath::Create(inLinux, pathVal, p_fm_);
  }
  game_data_ = std::make_unique<ConfigurationFile>(p_fm_, "game_data.txt");

  Rectangle sBound = Rectangle(pe.szScreenRez);
  int nScale = 2;
  Rectangle rBound =
      Rectangle(0, 0, sBound.sz.x / nScale, sBound.sz.y / nScale);

  pGr = pe.pGr;
  pSm = pe.pSndMng;

  pDr = std::make_unique<ScalingDrawer>(pGr, nScale);
  pBigDr = std::make_unique<ScalingDrawer>(pGr, nScale * 2);

  std::string sFontPath = "dragonfont\\";

  pNum = std::make_unique<NumberDrawer>(pDr.get(), fp_.get(), sFontPath,
                                        "dragonfont");
  pBigNum = std::make_unique<NumberDrawer>(pBigDr.get(), fp_.get(), sFontPath,
                                           "dragonfont");
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
      pDr.get(), pNum.get(), pBigNum.get(), pFancyNum.get(), pSm, vLvl,
      rBound, szActualRez, pExitProgram, fp_.get(), config_.get(),
      game_data_.get());
  pCnt->StartUp(pCnt.get());
}

DragonGameRunner::~DragonGameRunner() = default;

DragonGameController *DragonGameRunner::GetTowerController() const {
  return pCnt.get();
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
  GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
  if (p)
    p->Update();
}

void DragonGameRunner::KeyDown(GuiKeyType nCode) {
  GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
  if (p)
    p->OnKey(nCode, false);

#ifndef PC_VERSION
  if (nCode == GUI_ESCAPE)
    Trigger(pExitProgram);
#endif
}

void DragonGameRunner::KeyUp(GuiKeyType nCode) {
  GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
  if (p)
    p->OnKey(nCode, true);
}

void DragonGameRunner::MouseMove(Point pPos) {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
  if (p)
    p->OnMouse(pPos);
#endif
#endif
}

void DragonGameRunner::MouseDown(Point pPos) {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
  if (p)
    p->OnMouseDown(pPos);
#endif
#endif
}

void DragonGameRunner::MouseUp() {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
  if (p)
    p->OnMouseUp();
#endif
#endif
}

void DragonGameRunner::DoubleClick() {
  GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
  if (p)
    p->DoubleClick();
}

void DragonGameRunner::Fire() {
  GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
  if (p)
    p->Fire();
}
