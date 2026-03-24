#include "dragon_game_runner.h"
#include "../game_utils/MessageWriter.h"
#include "controller/dragon_game_controller.h"
#include "controller/game_controller_interface.h"
#include "fireball.h"
#include "level.h"
#include "../game_utils/event.h"
#include "../utils/configuration_file.h"
#include "../utils/file_utils.h"
#include "../wrappers/font_writer.h"

MessageWriter *pWr = 0;

DragonGameRunner::DragonGameRunner(ProgramEngine const& pe)
    : p_fm_(pe.p_fm),
      config_(std::make_unique<ConfigurationFile>(p_fm_, "config.txt")),
      game_config_(ReadGameConfig(*config_)) {
  szActualRez = pe.szActualRez;

  pExitProgram = pe.pExitProgram.get();

  pWr = pe.pMsg.get();

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
  if (game_config_.IsTrialVersion())
    levelsFile = "levels_trial.txt";
  else if (game_config_.IsSmallScreenVersion())
    levelsFile = "levels_small.txt";
  else
    levelsFile = "levels.txt";
  ReadLevels(fp_.get(), levelsFile, rBound, vLvl);

  pCnt = std::make_unique<DragonGameController>(
      pDr.get(), pNum.get(), pBigNum.get(), pFancyNum.get(), pSm, vLvl,
      rBound, szActualRez, pExitProgram, fp_.get(), config_.get(),
      game_data_.get(), game_config_);
  pCnt->StartUp();
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

  if (!game_config_.IsPcVersion() && nCode == GUI_ESCAPE)
    Trigger(pExitProgram);
}

void DragonGameRunner::KeyUp(GuiKeyType nCode) {
  GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
  if (p)
    p->OnKey(nCode, true);
}

void DragonGameRunner::MouseMove(Point pPos) {
  if (game_config_.IsPcVersion() && !game_config_.IsKeyboardControls()) {
    GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
    if (p)
      p->OnMouse(pPos);
  }
}

void DragonGameRunner::MouseDown(Point pPos) {
  if (game_config_.IsPcVersion() && !game_config_.IsKeyboardControls()) {
    GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
    if (p)
      p->OnMouseDown(pPos);
  }
}

void DragonGameRunner::MouseUp() {
  if (game_config_.IsPcVersion() && !game_config_.IsKeyboardControls()) {
    GameController *p = pCnt ? pCnt->GetActiveController() : nullptr;
    if (p)
      p->OnMouseUp();
  }
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
