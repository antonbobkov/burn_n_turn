#include "game.h"

SimpleController::SimpleController(SP<TwrGlobalController> pGraph,
                                   std::string strFileName)
    : GameController(pGraph) {
  nImage = pGl->pGraph->LoadImage(strFileName);
}

SimpleController::~SimpleController() { pGl->pGraph->DeleteImage(nImage); }

void SimpleController::Update() {
  pGl->pGraph->DrawImage(Gui::Point(0, 0), nImage);
}

void SimpleController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;

  pGl->Next();
}

FlashingController::FlashingController(SP<TwrGlobalController> pGraph,
                                       std::string strFileName,
                                       std::string strTextName)
    : GameController(pGraph), nTimer(0), bShow(true) {
  nImage = pGl->pGraph->LoadImage(strFileName);
  nText = pGl->pGraph->LoadImage(strTextName);

  pGl->pGraph->GetImage(nText)->ChangeColor(Color(255, 255, 255),
                                            Color(0, 0, 0, 0));
}

FlashingController::~FlashingController() {
  pGl->pGraph->DeleteImage(nImage);
  pGl->pGraph->DeleteImage(nText);
}

void FlashingController::Update() {
  ++nTimer;
  if (nTimer % nFramesInSecond == 0)
    bShow = !bShow;

  pGl->pGraph->DrawImage(Point(0, 0), nImage, false);
  if (bShow)
    pGl->pGraph->DrawImage(Point(0, 0), nText, false);
  pGl->pGraph->RefreshAll();
}

void FlashingController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;

  pGl->Next();
}

std::string OnOffString(bool b) {
  if (b)
    return "on";
  else
    return "off";
}

void MenuDisplay::Draw(SP<ScalingDrawer> pDr) {
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

void Countdown::Update() {
  ++nCount;
  if (nCount % nFramesInSecond == 0)
    --nTime;
  if (nTime == 0)
    bExist = false;
}

void Countdown::Draw(SP<ScalingDrawer> pDr) {
  pNum->DrawNumber(nTime, Point(22, 2));
}

void BasicController::AddV(SP<VisualEntity> pVs) {
  lsDraw.push_back(ASSP<VisualEntity>(this, pVs));
}

void BasicController::AddE(SP<EventEntity> pEv) {
  lsUpdate.push_back(ASSP<EventEntity>(this, pEv));
}

void BasicController::AddBackground(Color c) {
  Rectangle r = rBound.sz;
  r.sz.x *= pGl->pDr->nFactor;
  r.sz.y *= pGl->pDr->nFactor;

  SP<StaticRectangle> pBkg = new StaticRectangle(r, c, -1.F);

  AddV(pBkg);
}

BasicController::BasicController(const BasicController &b)
    : GameController(b), bNoRefresh(b.bNoRefresh) {
  CopyArrayASSP(this, b.lsDraw, lsDraw);
  CopyArrayASSP(this, b.lsUpdate, lsUpdate);
  CopyArrayASSP(this, b.lsPpl, lsPpl);
}

BasicController::BasicController(SP<TwrGlobalController> pGl_, Rectangle rBound,
                                 Color c)
    : GameController(pGl_, rBound), bNoRefresh(false) {
  AddBackground(c);
}

void BasicController::Update() {
  CleanUp(lsUpdate);
  CleanUp(lsDraw);
  CleanUp(lsPpl);

  std::list<ASSP<EventEntity>>::iterator itr;
  for (itr = lsUpdate.begin(); itr != lsUpdate.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;
    (*itr)->Move();
  }

  for (itr = lsUpdate.begin(); itr != lsUpdate.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    (*itr)->Update();
  }

  {
    std::list<ASSP<VisualEntity>>::iterator itr;

    std::multimap<ScreenPos, SP<VisualEntity>> mmp;

    for (itr = lsDraw.begin(); itr != lsDraw.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      mmp.insert(std::pair<ScreenPos, SP<VisualEntity>>(
          ScreenPos((*itr)->GetPriority(), ((*itr)->GetPosition())), *itr));
    }

    for (std::multimap<ScreenPos, SP<VisualEntity>>::iterator
             mitr = mmp.begin(),
             metr = mmp.end();
         mitr != metr; ++mitr)
      mitr->second->Draw(pGl->pDr);
  }

  if (!bNoRefresh)
    pGl->pGraph->RefreshAll();
}

void BasicController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;

  pGl->Next();
}

void BasicController::OnMouseDown(Point pPos) { pGl->Next(); }

void MouseCursor::DrawCursor() {
  Index img;
  if (bPressed)
    img = imgCursor.vImage.at(1);
  else
    img = imgCursor.vImage.at(0);

  Size sz = pGl->pGraph->GetImage(img)->GetSize();
  Point p = pCursorPos;

  p.x -= sz.x / 2;
  p.y -= sz.y / 2;

  pGl->pGraph->DrawImage(p, img, false);
}

void MouseCursor::SetCursorPos(Point pPos) { pCursorPos = pPos; }

MenuController::MenuController(SP<TwrGlobalController> pGl_, Rectangle rBound,
                               Color c, int nResumePosition_)
    : BasicController(pGl_, rBound, c), nResumePosition(nResumePosition_),
      pMenuDisplay(this, 0), mc(pGl->pr("claw"), Point(), pGl.GetRawPointer()),
      pHintText(this, 0), pOptionText(this, 0) {
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
  BasicController::Update();

  if (pMenuDisplay->pCurr == &(pMenuDisplay->memOptions)) {
    if (pOptionText != 0)
      pOptionText->Draw(pGl->pDr);
  } else {
    if (pHintText != 0)
      pHintText->Draw(pGl->pDr);
  }

  pGl->pGraph->RefreshAll();
}

MenuDisplay::MenuDisplay(Point pLeftTop_, SP<NumberDrawer> pNum_,
                         SP<Animation> pMenuCaret_,
                         SP<MenuController> pMenuController_,
                         bool bCheatsUnlocked_)
    : pLeftTop(pLeftTop_), pNum(this, pNum_), pMenuCaret(this, pMenuCaret_),
      pMenuController(this, pMenuController_), pCurr(&memMain),
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

  if (pMenuController->pOptionText != 0)
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

  if (pMenuController->pOptionText != 0)
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

void StartScreenController::Next() {
  pGl->Next();
  pGl->pSnd->PlaySound(pGl->pr.GetSnd("start_game"));
}

void StartScreenController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;

  if (c == GUI_ESCAPE)
    pGl->Menu();
  else
    Next();
}

BuyNowController::BuyNowController(SP<TwrGlobalController> pGl_,
                                   Rectangle rBound, Color c)
    : BasicController(pGl_, rBound, c), t(120), nSlimeCount(50),
      tVel(nFramesInSecond / 2) {
  for (int i = 0; i < nSlimeCount; i++) {
    mSlimes.push_back(
        new Animation(0, pGl->pr("slime"), nFramesInSecond / 10,
                      Point(rBound.sz.x / 2, rBound.sz.y / 2 + 25), true));
    mSlimeVel.push_back(fPoint());
    mSlimePos.push_back(mSlimes.back()->pos);
  }

  bNoRefresh = true;
}

void BuyNowController::RandomizeVelocity(fPoint &fVel, fPoint pPos) {
  fVel = RandomAngle();

  if (rand() % 7 == 0)
    fVel = fPoint(rBound.sz.x / 2, rBound.sz.y / 2) - fPoint(pPos);

  fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fSlimeSpeed);
}

void BuyNowController::DrawSlimes() {
  for (unsigned i = 0; i < mSlimes.size(); i++)
    mSlimes[i]->Draw(pGl->pDr);
}

void BuyNowController::Update() {
  BasicController::Update();

  if (tVel.Tick()) {
    for (unsigned i = 0; i < mSlimes.size(); i++)
      if (float(rand()) / RAND_MAX < .25)
        RandomizeVelocity(mSlimeVel[i], mSlimePos[i]);
  }

  for (unsigned i = 0; i < mSlimes.size(); i++) {
    mSlimes[i]->Update();
    mSlimePos[i] += mSlimeVel[i];
    mSlimes[i]->pos = mSlimePos[i].ToPnt();
  }

  if (t >= 0)
    t--;

  pGl->pGraph->RefreshAll();
}

void BuyNowController::OnKey(GuiKeyType c, bool bUp) {
  if (t < 0)
    BasicController::OnKey(c, bUp);
}

void BuyNowController::OnMouseDown(Point pPos) {
  if (t < 0)
    BasicController::OnMouseDown(pPos);
}

void SlimeUpdater::Draw(SP<ScalingDrawer> pDr) { pBuy->DrawSlimes(); }

Cutscene::Cutscene(SP<TwrGlobalController> pGl_, Rectangle rBound_,
                   std::string sRun, std::string sChase, bool bFlip)
    : BasicController(pGl_, rBound_, Color(0, 0, 0)), pCrRun(this, 0),
      pCrFollow(this, 0), bRelease(false), tm(nFramesInSecond / 5),
      Beepy(true) {
  ImageSequence seq1 = pGl_->pr(sRun);

  int xPos = 5;
  if (bFlip)
    xPos = rBound_.sz.x - 5;
  float m = 1;
  if (bFlip)
    m = -1;

  SP<FancyCritter> pCr1 =
      new FancyCritter(7, fPoint(xPos, rBound_.sz.y / 2), fPoint(m * 10, 0),
                       rBound, 3, seq1, nFramesInSecond / 5);
  AddBoth(pCr1);

  pCrRun = pCr1;

  ImageSequence seq2 = pGl_->pr(sChase);

  SP<FancyCritter> pCr2 =
      new FancyCritter(7, fPoint(xPos, rBound_.sz.y / 2), fPoint(m * 12, 0),
                       rBound, 3, seq2, nFramesInSecond / 5);

  pCrFollow = pCr2;
}

void Cutscene::Update() {
  if (!bRelease && pCrRun->GetPosition().x >= rBound.sz.x / 3 &&
      pCrRun->GetPosition().x <= rBound.sz.x * 2 / 3) {
    bRelease = true;

    SP<FancyCritter> pCr2 = pCrFollow;
    AddBoth(pCr2);
  }

  if (!pCrFollow->bExist) {
    pGl->Next();
  }

  if (tm.Tick()) {
    if (Beepy)
      pGl->pSnd->PlaySound(pGl->pr.GetSnd("beep"));
    else
      pGl->pSnd->PlaySound(pGl->pr.GetSnd("boop"));

    Beepy = !Beepy;
  }

  BasicController::Update();
}

void Cutscene::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp && c == '\\' && pGl->sbCheatsOn.Get())
    pGl->Next();
}

void DragonScoreController::OnMouseDown(Point pPos) {
  if (bClickToExit)
    pGl->Next();
}

DragonScoreController::DragonScoreController(SP<TwrGlobalController> pGl_,
                                             Rectangle rBound, Color c,
                                             bool bScoreShow)
    : BasicController(pGl_, rBound, c), t(5 * nFramesInSecond),
      bClickToExit(false) {
  if (bScoreShow) {
    AddV(new HighScoreShower(pGl, rBound));
  } else
    AddV(new IntroTextShower(pGl, rBound));
}

void DragonScoreController::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp)
    pGl->Next();
}

void DragonScoreController::DoubleClick() { pGl->Next(); }

void DragonScoreController::Update() {
  if (pGl->nHighScore < pGl->nScore) {
    pGl->nHighScore = pGl->nScore;

    FilePath *fp = pGl->pWrp->GetFilePath();
    if (fp) {
      std::unique_ptr<Gui::OutStreamHandler> oh = fp->WriteFile("high.txt");
      if (oh)
        oh->GetStream() << pGl->nScore;
    }
  }

  BasicController::Update();
}

void AlmostBasicController::Update() {
  CleanUp(lsUpdate);
  CleanUp(lsDraw);

  if (lsDraw.size() == 1) {
    pGl->Next();
    return;
  }

  BasicController::Update();
}

void AlmostBasicController::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp)
    pGl->Next();
}

void AlmostBasicController::OnMouseDown(Point pPos) { pGl->Next(); }
