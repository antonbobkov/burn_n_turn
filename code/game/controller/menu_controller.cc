#include "game/game.h"
#include "game/controller/menu_controller.h"

#include "wrappers/color.h"
#include "utils/file_utils.h"
#include "game_utils/game_runner_interface.h"
#include "game_utils/event.h"

std::string OnOffString(bool b) {
  if (b)
    return "on";
  else
    return "off";
}

std::string SoundString() { return "sound: "; }
std::string MusicString() { return "music: "; }
std::string TutorialString() { return "tutorial: "; }
std::string FullTextString() { return "full screen: "; }

void MenuDisplay::Draw(smart_pointer<ScalingDrawer> pDr) {
  Point p = pLeftTop;
  for (unsigned i = 0; i < pCurr->vEntries.size(); ++i) {
    if (!pCurr->vEntries[i].bDisabled)
      pNum->DrawWord(pCurr->vEntries[i].sText, p, false);
    else
      pNum->DrawColorWord(pCurr->vEntries[i].sText, p, Color(125, 125, 125),
                          false);

    if (pCurr->nMenuPosition == i)
      pMenuCaret->pos = p + Point(-11, pCurr->vEntries[i].szSize.y / 4);

    p.y += pCurr->vEntries[i].szSize.y;
  }
}

void MenuDisplay::OnMouseMove(Point pMouse) {
  pMouse.x /= 4;
  pMouse.y /= 4;

  Point p = pLeftTop;
  for (unsigned i = 0; i < pCurr->vEntries.size(); ++i) {
    if (!pCurr->vEntries[i].bDisabled &&
        InsideRectangle(Rectangle(p, pCurr->vEntries[i].szSize), pMouse)) {
      pCurr->nMenuPosition = i;
      return;
    }
    p.y += pCurr->vEntries[i].szSize.y;
  }
}

MenuController::MenuController(DragonGameControllerList *pGl_,
                               Rectangle rBound, Color c, int nResumePosition_)
    : EntityListController(pGl_, rBound, c), nResumePosition(nResumePosition_),
      pMenuDisplay(), mc(pGl_->pr("claw"), Point()),
      pHintText(), pOptionText() {
  bNoRefresh = true;
}

void MenuController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;
  if (c == GUI_ESCAPE) {
    pMenuDisplay->Escape();
    return;
  } else if (c == GUI_UP) {
    pMenuDisplay->PositionIncrement(false);
  } else if (c == GUI_DOWN) {
    pMenuDisplay->PositionIncrement(true);
  } else if (c == GUI_RETURN || c == ' ') {
    pMenuDisplay->Boop();
  }
}

void MenuController::OnMouse(Point pPos) {}

void MenuController::OnMouseDown(Point pPos) {}

void MenuController::Update() {
  EntityListController::Update();

  if (pMenuDisplay->pCurr == &(pMenuDisplay->memOptions)) {
    if (!pOptionText.is_null())
      pOptionText->Draw(pGl->pDr);
  } else {
    if (!pHintText.is_null())
      pHintText->Draw(pGl->pDr);
  }

  pGl->pGraph->RefreshAll();
}

MenuDisplay::MenuDisplay(Point pLeftTop_, smart_pointer<NumberDrawer> pNum_,
                         smart_pointer<Animation> pMenuCaret_,
                         smart_pointer<MenuController> pMenuController_,
                         bool bCheatsUnlocked_)
    : pLeftTop(pLeftTop_), pNum(pNum_), pMenuCaret(pMenuCaret_),
      pMenuController(pMenuController_.get()), pCurr(&memMain),
      bCheatsUnlocked(bCheatsUnlocked_) {
  Size szSpacing(50, 10);
  memMain.vEntries.push_back(
      MenuEntry(szSpacing, "continue", &MenuDisplay::Continue));
  memMain.vEntries.push_back(
      MenuEntry(szSpacing, "restart", &MenuDisplay::Restart));
  memMain.vEntries.push_back(
      MenuEntry(szSpacing, "load chapter", &MenuDisplay::LoadChapterSubmenu));
  memMain.vEntries.push_back(
      MenuEntry(szSpacing, "options", &MenuDisplay::OptionsSubmenu));
  memMain.vEntries.push_back(MenuEntry(szSpacing, "exit", &MenuDisplay::Exit));

  memLoadChapter.vEntries.push_back(
      MenuEntry(szSpacing, "chapter 1", &MenuDisplay::Chapter1));
  memLoadChapter.vEntries.push_back(
      MenuEntry(szSpacing, "chapter 2", &MenuDisplay::Chapter2));
  memLoadChapter.vEntries.push_back(
      MenuEntry(szSpacing, "chapter 3", &MenuDisplay::Chapter3));
  memLoadChapter.vEntries.push_back(
      MenuEntry(szSpacing, "back", &MenuDisplay::Escape));

  memOptions.vEntries.push_back(
      MenuEntry(szSpacing, "", &MenuDisplay::MusicToggle));
  nMusic = memOptions.vEntries.size() - 1;

  memOptions.vEntries.push_back(
      MenuEntry(szSpacing, "", &MenuDisplay::SoundToggle));
  nSound = memOptions.vEntries.size() - 1;

  memOptions.vEntries.push_back(
      MenuEntry(szSpacing, "", &MenuDisplay::TutorialToggle));
  nTutorial = memOptions.vEntries.size() - 1;

  memOptions.vEntries.push_back(
      MenuEntry(szSpacing, "", &MenuDisplay::FullScreenToggle));
  nFullScreen = memOptions.vEntries.size() - 1;

  if (bCheatsUnlocked) {
    memOptions.vEntries.push_back(
        MenuEntry(szSpacing, "", &MenuDisplay::CheatsToggle));
    nCheats = memOptions.vEntries.size() - 1;
  } else
    nCheats = -1;

  memOptions.vEntries.push_back(
      MenuEntry(szSpacing, "back", &MenuDisplay::Escape));

  vOptionText.resize(memOptions.vEntries.size());

  if (bCheatsUnlocked)
    vOptionText.at(nCheats) =
        "f1-f10 to get bonuses\n\\ to skip level\ni for invincibility";

  UpdateMenuEntries();
}

void MenuDisplay::PositionIncrement(bool bUp) {
  pMenuController->pGl->pSnd->PlaySound(pMenuController->pGl->pr.GetSnd("B"));

  int nDelta = bUp ? 1 : -1;

  while (true) {
    pCurr->nMenuPosition += nDelta;
    pCurr->nMenuPosition += pCurr->vEntries.size();
    pCurr->nMenuPosition %= pCurr->vEntries.size();

    if (pCurr->nMenuPosition == 0 ||
        !pCurr->vEntries[pCurr->nMenuPosition].bDisabled)
      break;
  }

  if (!pMenuController->pOptionText.is_null())
    pMenuController->pOptionText->SetText(
        vOptionText.at(memOptions.nMenuPosition));
}

void MenuDisplay::Boop() {
  if (pCurr->vEntries.at(pCurr->nMenuPosition).pTriggerEvent ==
      &MenuDisplay::Escape)
    pMenuController->pGl->pSnd->PlaySound(pMenuController->pGl->pr.GetSnd("C"));
  else
    pMenuController->pGl->pSnd->PlaySound(pMenuController->pGl->pr.GetSnd("A"));
  (this->*(pCurr->vEntries.at(pCurr->nMenuPosition).pTriggerEvent))();
}

void MenuDisplay::Restart() { pMenuController->pGl->Restart(); }

void MenuDisplay::Continue() {
  pMenuController->pGl->nActive = pMenuController->nResumePosition;
}

void MenuDisplay::MusicToggle() {
  pMenuController->pGl->plr.ToggleOff();
  pMenuController->pGl->sbMusicOn.Set(!pMenuController->pGl->plr.bOff);
  UpdateMenuEntries();
}

void MenuDisplay::SoundToggle() {
  pMenuController->pGl->pSnd->Toggle();
  pMenuController->pGl->sbSoundOn.Set(pMenuController->pGl->pSnd->Get());
  UpdateMenuEntries();
}

void MenuDisplay::TutorialToggle() {
  BoolToggle(pMenuController->pGl->sbTutorialOn);
  UpdateMenuEntries();
}

void MenuDisplay::FullScreenToggle() {
  BoolToggle(pMenuController->pGl->sbFullScreen);
  UpdateMenuEntries();
}

void MenuDisplay::CheatsToggle() {
  BoolToggle(pMenuController->pGl->sbCheatsOn);
  UpdateMenuEntries();
}

void MenuDisplay::Exit() { Trigger(pMenuController->pGl->pWrp->pExitProgram); }

void MenuDisplay::Escape() {
  if (pCurr != &memMain)
    pCurr = &memMain;
  else
    Continue();
}

void MenuDisplay::LoadChapterSubmenu() { pCurr = &memLoadChapter; }

void MenuDisplay::OptionsSubmenu() { pCurr = &memOptions; }

void MenuDisplay::UpdateMenuEntries() {
  memOptions.vEntries.at(nMusic).sText =
      MusicString() + OnOffString(pMenuController->pGl->sbMusicOn.Get());
  memOptions.vEntries.at(nSound).sText =
      SoundString() + OnOffString(pMenuController->pGl->sbSoundOn.Get());
  memOptions.vEntries.at(nTutorial).sText =
      TutorialString() + OnOffString(pMenuController->pGl->sbTutorialOn.Get());

  bool bFullScreenNow = GetProgramInfo().bFullScreen;
  bool bFullScreenSetting = pMenuController->pGl->sbFullScreen.Get();
  std::string sExtra = "";
  if (bFullScreenNow != bFullScreenSetting)
    sExtra = "changes will take effect next launch";

  memOptions.vEntries.at(nFullScreen).sText =
      FullTextString() + OnOffString(bFullScreenSetting);

  vOptionText.at(nFullScreen) = sExtra;

  bool bCheatsOn = pMenuController->pGl->sbCheatsOn.Get();

  if (bCheatsUnlocked)
    memOptions.vEntries.at(nCheats).sText = "cheats: " + OnOffString(bCheatsOn);

  memLoadChapter.vEntries.at(1).bDisabled =
      (pMenuController->pGl->snProgress.Get() < 1);
  memLoadChapter.vEntries.at(2).bDisabled =
      (pMenuController->pGl->snProgress.Get() < 2);

  if (!pMenuController->pOptionText.is_null())
    pMenuController->pOptionText->SetText(
        vOptionText.at(memOptions.nMenuPosition));
}

void MenuDisplay::Chapter1() {
  pMenuController->pGl->Restart(pMenuController->pGl->vLevelPointers.at(0));
}

void MenuDisplay::Chapter2() {
  pMenuController->pGl->Restart(pMenuController->pGl->vLevelPointers.at(1));
}

void MenuDisplay::Chapter3() {
  pMenuController->pGl->Restart(pMenuController->pGl->vLevelPointers.at(2));
}
