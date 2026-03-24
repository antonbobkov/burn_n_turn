#include "menu_controller.h"
#include "dragon_game_controller.h"
#include "game_controller_interface.h"
#include "../dragon_game_runner.h"
#include "../../game_utils/draw_utils.h"
#include "../../game_utils/event.h"
#include "../../game_utils/game_runner_interface.h"
#include "../../utils/file_utils.h"
#include "../../wrappers/color.h"
#include "../../wrappers/geometry.h"
#include "../../wrappers/gui_key_type.h"

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

void MenuDisplay::Draw(ScalingDrawer * /*pDr*/) {
  Point p = pLeftTop;
  for (int i = 0; i < pCurr->Size(); ++i) {
    if (!(*pCurr)[i].bDisabled)
      pNum->DrawWord((*pCurr)[i].sText, p, false);
    else
      pNum->DrawColorWord((*pCurr)[i].sText, p, Color(125, 125, 125), false);

    if (pCurr->GetPosition() == i && pMenuCaret)
      pMenuCaret->SetPos(p + Point(-11, (*pCurr)[i].szSize.y / 4));

    p.y += (*pCurr)[i].szSize.y;
  }
}

void MenuDisplay::OnMouseMove(Point pMouse) {
  pMouse.x /= 4;
  pMouse.y /= 4;

  Point p = pLeftTop;
  for (int i = 0; i < pCurr->Size(); ++i) {
    if (!(*pCurr)[i].bDisabled &&
        InsideRectangle(Rectangle(p, (*pCurr)[i].szSize), pMouse)) {
      pCurr->SetPosition(i);
      return;
    }
    p.y += (*pCurr)[i].szSize.y;
  }
}

MenuController::MenuController(DragonGameController *pGl_,
                               DragonGameSettings *settings_, Rectangle rBound,
                               Color c)
    : EntityListController(pGl_, rBound, c), pMenuCaret(), pMenuDisplay(),
      mc(pGl_->GetImgSeq("claw"), Point()), pHintText(), pOptionText(),
      settings(settings_) {
  SuppressRefresh();
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

std::vector<Entity *> MenuController::GetNonOwnedEntities() {
  std::vector<Entity *> out;
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

void MenuController::OnMouse(Point /*pPos*/) {}

void MenuController::OnMouseDown(Point /*pPos*/) {}

void MenuController::Update() {
  EntityListController::Update();

  if (!pMenuDisplay)
    return;
  if (pMenuDisplay->IsInOptionsMenu()) {
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
    : pCurr(&memMain), bCheatsUnlocked(bCheatsUnlocked_),
      pLeftTop(pLeftTop_), pNum(pNum_), pMenuCaret(pMenuCaret_),
      pMenuController(pMenuController_) {
  Size szSpacing(50, 10);
  memMain.AddEntry(MenuEntry(szSpacing, "continue", &MenuDisplay::Continue));
  memMain.AddEntry(MenuEntry(szSpacing, "restart", &MenuDisplay::Restart));
  memMain.AddEntry(
      MenuEntry(szSpacing, "load chapter", &MenuDisplay::LoadChapterSubmenu));
  memMain.AddEntry(MenuEntry(szSpacing, "options", &MenuDisplay::OptionsSubmenu));
  memMain.AddEntry(MenuEntry(szSpacing, "exit", &MenuDisplay::Exit));

  memLoadChapter.AddEntry(
      MenuEntry(szSpacing, "chapter 1", &MenuDisplay::Chapter1));
  memLoadChapter.AddEntry(
      MenuEntry(szSpacing, "chapter 2", &MenuDisplay::Chapter2));
  memLoadChapter.AddEntry(
      MenuEntry(szSpacing, "chapter 3", &MenuDisplay::Chapter3));
  memLoadChapter.AddEntry(MenuEntry(szSpacing, "back", &MenuDisplay::Escape));

  memOptions.AddEntry(MenuEntry(szSpacing, "", &MenuDisplay::MusicToggle));
  nMusic = memOptions.Size() - 1;

  memOptions.AddEntry(MenuEntry(szSpacing, "", &MenuDisplay::SoundToggle));
  nSound = memOptions.Size() - 1;

  memOptions.AddEntry(MenuEntry(szSpacing, "", &MenuDisplay::TutorialToggle));
  nTutorial = memOptions.Size() - 1;

  memOptions.AddEntry(
      MenuEntry(szSpacing, "", &MenuDisplay::FullScreenToggle));
  nFullScreen = memOptions.Size() - 1;

  if (bCheatsUnlocked) {
    memOptions.AddEntry(
        MenuEntry(szSpacing, "", &MenuDisplay::CheatsToggle));
    nCheats = memOptions.Size() - 1;
  } else
    nCheats = -1;

  memOptions.AddEntry(MenuEntry(szSpacing, "back", &MenuDisplay::Escape));

  vOptionText.resize(memOptions.Size());

  if (bCheatsUnlocked)
    vOptionText.at(nCheats) =
        "f1-f10 to get bonuses\n\\ to skip level\ni for invincibility";

  UpdateMenuEntries();
}

void MenuDisplay::PositionIncrement(bool bUp) {
  pMenuController->GetGl()->PlaySound("B");

  int nDelta = bUp ? 1 : -1;

  while (true) {
    pCurr->SetPosition(
        (pCurr->GetPosition() + nDelta + pCurr->Size()) % pCurr->Size());

    if (pCurr->GetPosition() == 0 ||
        !(*pCurr)[pCurr->GetPosition()].bDisabled)
      break;
  }

  pMenuController->SetOptionTextContent(
      vOptionText.at(memOptions.GetPosition()));
}

void MenuDisplay::Boop() {
  if (pCurr->At(pCurr->GetPosition()).pTriggerEvent == &MenuDisplay::Escape)
    pMenuController->GetGl()->PlaySound("C");
  else
    pMenuController->GetGl()->PlaySound("A");
  (this->*(pCurr->At(pCurr->GetPosition()).pTriggerEvent))();
}

void MenuDisplay::Restart() { pMenuController->GetGl()->Restart(); }

void MenuDisplay::Continue() {
  pMenuController->GetGl()->ExitMenuResume();
}

void MenuDisplay::MusicToggle() {
  pMenuController->GetGl()->ToggleMusicPlayback();
  pMenuController->GetSettings()->sbMusicOn.Set(
      !pMenuController->GetGl()->IsMusicPlaybackOff());
  UpdateMenuEntries();
}

void MenuDisplay::SoundToggle() {
  pMenuController->GetGl()->ToggleSoundOutput();
  pMenuController->GetSettings()->sbSoundOn.Set(
      pMenuController->GetGl()->IsSoundOutputOn());
  UpdateMenuEntries();
}

void MenuDisplay::TutorialToggle() {
  BoolToggle(pMenuController->GetSettings()->sbTutorialOn);
  UpdateMenuEntries();
}

void MenuDisplay::FullScreenToggle() {
  pMenuController->GetGl()->SetFullScreenSetting(
      !pMenuController->GetGl()->IsFullScreenSetting());
  UpdateMenuEntries();
}

void MenuDisplay::CheatsToggle() {
  BoolToggle(pMenuController->GetSettings()->sbCheatsOn);
  UpdateMenuEntries();
}

void MenuDisplay::Exit() { pMenuController->GetGl()->ExitProgram(); }

void MenuDisplay::Escape() {
  if (pCurr != &memMain)
    pCurr = &memMain;
  else
    Continue();
}

void MenuDisplay::LoadChapterSubmenu() { pCurr = &memLoadChapter; }

void MenuDisplay::OptionsSubmenu() { pCurr = &memOptions; }

void MenuDisplay::UpdateMenuEntries() {
  DragonGameSettings *settings = pMenuController->GetSettings();
  memOptions.At(nMusic).sText =
      MusicString() + OnOffString(settings->sbMusicOn.Get());
  memOptions.At(nSound).sText =
      SoundString() + OnOffString(settings->sbSoundOn.Get());
  memOptions.At(nTutorial).sText =
      TutorialString() + OnOffString(settings->sbTutorialOn.Get());

  bool bFullScreenNow = GetProgramInfo().bFullScreen;
  bool bFullScreenSetting = pMenuController->GetGl()->IsFullScreenSetting();
  std::string sExtra = "";
  if (bFullScreenNow != bFullScreenSetting)
    sExtra = "changes will take effect next launch";

  memOptions.At(nFullScreen).sText =
      FullTextString() + OnOffString(bFullScreenSetting);

  vOptionText.at(nFullScreen) = sExtra;

  bool bCheatsOn = settings->sbCheatsOn.Get();

  if (bCheatsUnlocked)
    memOptions.At(nCheats).sText = "cheats: " + OnOffString(bCheatsOn);

  memLoadChapter.At(1).bDisabled = (settings->snProgress.Get() < 1);
  memLoadChapter.At(2).bDisabled = (settings->snProgress.Get() < 2);

  pMenuController->SetOptionTextContent(vOptionText.at(memOptions.GetPosition()));
}

void MenuDisplay::Chapter1() {
  pMenuController->GetGl()->RestartFromChapter(0);
}

void MenuDisplay::Chapter2() {
  pMenuController->GetGl()->RestartFromChapter(1);
}

void MenuDisplay::Chapter3() {
  pMenuController->GetGl()->RestartFromChapter(2);
}
