#include "game/controller/menu_controller.h"
#include "game/controller/dragon_game_controller.h"
#include "game/controller/game_controller_interface.h"
#include "game/dragon_game_runner.h"
#include "game_utils/draw_utils.h"
#include "game_utils/event.h"
#include "game_utils/game_runner_interface.h"
#include "utils/file_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/color.h"
#include "wrappers/geometry.h"
#include "wrappers/gui_key_type.h"

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

void MenuDisplay::Draw(ScalingDrawer *pDr) {
  Point p = pLeftTop;
  for (unsigned i = 0; i < pCurr->vEntries.size(); ++i) {
    if (!pCurr->vEntries[i].bDisabled)
      pNum->DrawWord(pCurr->vEntries[i].sText, p, false);
    else
      pNum->DrawColorWord(pCurr->vEntries[i].sText, p, Color(125, 125, 125),
                          false);

    if (pCurr->nMenuPosition == i && pMenuCaret)
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

MenuController::MenuController(DragonGameController *pGl_,
                               DragonGameSettings *settings_, Rectangle rBound,
                               Color c)
    : EntityListController(pGl_, rBound, c), pMenuCaret(), pMenuDisplay(),
      mc(pGl_->GetImgSeq("claw"), Point()), pHintText(), pOptionText(),
      settings(settings_) {
  bNoRefresh = true;
}

void MenuController::SetMenuCaret(std::unique_ptr<Animation> p) {
  pMenuCaret = std::move(p);
}

void MenuController::SetHintText(std::unique_ptr<TextDrawEntity> p) {
  pHintText = std::move(p);
}

void MenuController::SetOptionText(std::unique_ptr<TextDrawEntity> p) {
  pOptionText = std::move(p);
}

void MenuController::SetMenuDisplay(std::unique_ptr<MenuDisplay> p) {
  pMenuDisplay = std::move(p);
}

std::vector<EventEntity *> MenuController::GetNonOwnedUpdateEntities() {
  std::vector<EventEntity *> out;
  if (pMenuCaret)
    out.push_back(pMenuCaret.get());
  if (pMenuDisplay)
    out.push_back(pMenuDisplay.get());
  return out;
}

std::vector<VisualEntity *> MenuController::GetNonOwnedDrawEntities() {
  std::vector<VisualEntity *> out;
  if (pMenuCaret)
    out.push_back(pMenuCaret.get());
  if (pMenuDisplay)
    out.push_back(pMenuDisplay.get());
  return out;
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

  if (!pMenuDisplay)
    return;
  if (pMenuDisplay->pCurr == &(pMenuDisplay->memOptions)) {
    if (pOptionText)
      pOptionText->Draw(pGl->GetDrawer());
  } else {
    if (pHintText)
      pHintText->Draw(pGl->GetDrawer());
  }

  pGl->RefreshAll();
}

MenuDisplay::MenuDisplay(Point pLeftTop_, NumberDrawer *pNum_,
                         Animation *pMenuCaret_,
                         MenuController *pMenuController_,
                         bool bCheatsUnlocked_)
    : pLeftTop(pLeftTop_), pNum(pNum_), pMenuCaret(pMenuCaret_),
      pMenuController(pMenuController_), pCurr(&memMain),
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
  pMenuController->pGl->PlaySound("B");

  int nDelta = bUp ? 1 : -1;

  while (true) {
    pCurr->nMenuPosition += nDelta;
    pCurr->nMenuPosition += pCurr->vEntries.size();
    pCurr->nMenuPosition %= pCurr->vEntries.size();

    if (pCurr->nMenuPosition == 0 ||
        !pCurr->vEntries[pCurr->nMenuPosition].bDisabled)
      break;
  }

  if (pMenuController->pOptionText)
    pMenuController->pOptionText->SetText(
        vOptionText.at(memOptions.nMenuPosition));
}

void MenuDisplay::Boop() {
  if (pCurr->vEntries.at(pCurr->nMenuPosition).pTriggerEvent ==
      &MenuDisplay::Escape)
    pMenuController->pGl->PlaySound("C");
  else
    pMenuController->pGl->PlaySound("A");
  (this->*(pCurr->vEntries.at(pCurr->nMenuPosition).pTriggerEvent))();
}

void MenuDisplay::Restart() { pMenuController->pGl->Restart(); }

void MenuDisplay::Continue() {
  pMenuController->pGl->ExitMenuResume();
}

void MenuDisplay::MusicToggle() {
  pMenuController->pGl->ToggleMusicPlayback();
  pMenuController->settings->sbMusicOn.Set(
      !pMenuController->pGl->IsMusicPlaybackOff());
  UpdateMenuEntries();
}

void MenuDisplay::SoundToggle() {
  pMenuController->pGl->ToggleSoundOutput();
  pMenuController->settings->sbSoundOn.Set(
      pMenuController->pGl->IsSoundOutputOn());
  UpdateMenuEntries();
}

void MenuDisplay::TutorialToggle() {
  BoolToggle(pMenuController->settings->sbTutorialOn);
  UpdateMenuEntries();
}

void MenuDisplay::FullScreenToggle() {
  pMenuController->pGl->SetFullScreenSetting(
      !pMenuController->pGl->IsFullScreenSetting());
  UpdateMenuEntries();
}

void MenuDisplay::CheatsToggle() {
  BoolToggle(pMenuController->settings->sbCheatsOn);
  UpdateMenuEntries();
}

void MenuDisplay::Exit() { pMenuController->pGl->ExitProgram(); }

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
      MusicString() + OnOffString(pMenuController->settings->sbMusicOn.Get());
  memOptions.vEntries.at(nSound).sText =
      SoundString() + OnOffString(pMenuController->settings->sbSoundOn.Get());
  memOptions.vEntries.at(nTutorial).sText =
      TutorialString() +
      OnOffString(pMenuController->settings->sbTutorialOn.Get());

  bool bFullScreenNow = GetProgramInfo().bFullScreen;
  bool bFullScreenSetting = pMenuController->pGl->IsFullScreenSetting();
  std::string sExtra = "";
  if (bFullScreenNow != bFullScreenSetting)
    sExtra = "changes will take effect next launch";

  memOptions.vEntries.at(nFullScreen).sText =
      FullTextString() + OnOffString(bFullScreenSetting);

  vOptionText.at(nFullScreen) = sExtra;

  bool bCheatsOn = pMenuController->settings->sbCheatsOn.Get();

  if (bCheatsUnlocked)
    memOptions.vEntries.at(nCheats).sText = "cheats: " + OnOffString(bCheatsOn);

  memLoadChapter.vEntries.at(1).bDisabled =
      (pMenuController->settings->snProgress.Get() < 1);
  memLoadChapter.vEntries.at(2).bDisabled =
      (pMenuController->settings->snProgress.Get() < 2);

  if (pMenuController->pOptionText)
    pMenuController->pOptionText->SetText(
        vOptionText.at(memOptions.nMenuPosition));
}

void MenuDisplay::Chapter1() {
  pMenuController->pGl->RestartFromChapter(0);
}

void MenuDisplay::Chapter2() {
  pMenuController->pGl->RestartFromChapter(1);
}

void MenuDisplay::Chapter3() {
  pMenuController->pGl->RestartFromChapter(2);
}
