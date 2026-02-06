#include "tutorial.h"

namespace {

#ifdef KEYBOARD_CONTROLS
#ifdef JOYSTICK_TUTORIAL
const std::string sSteerMessage = "steer with joystick";
const std::string sShootingMessage = "move joystick to shoot a fireball";
const std::string sTakeOffMessage = "press button to take off";
#else
const std::string sSteerMessage = "steer with left and right keys";
const std::string sShootingMessage = "shoot with arrow keys";
const std::string sTakeOffMessage = "press space to take off";
#endif
#else
const std::string sSteerMessage = "click and hold to steer";
const std::string sShootingMessage = "click anywhere to shoot a fireball";
const std::string sTakeOffMessage =
    "press space or click the tower to take off";
#endif

} // namespace

void TutorialTextEntity::SetText(std::vector<std::string> v) {
  if (sText == v && nDelta == 1)
    return;

  if (sNewText == v && nDelta == -1)
    return;

  if (sText.empty()) {
    nOffset = 0;
    nDelta = 1;
    sText = v;
  } else {
    sNewText = v;
    nDelta = -1;
  }
}

void TutorialTextEntity::Draw(SP<ScalingDrawer> pDr) {
  if (pIsTutorialOn && (*pIsTutorialOn == false))
    return;

  for (unsigned i = 0; i < sText.size(); ++i) {
    Point p = pos;
    p.y -= nOffset;
    p.y += nTextVerticalSpacing * i + nTextVerticalSpacing / 3;
    pNum->DrawWord(sText[i], p, true);
  }
}

void TutorialTextEntity::Update() {
  if (t.Tick()) {
    nOffset += nDelta;
    int nMax = nTextVerticalSpacing * sText.size();

    if (nOffset >= nMax)
      nOffset = nMax;
    if (nOffset < 0) {
      nOffset = 0;
      nDelta = 1;
      sText = sNewText;
      sNewText.clear();
    }
  }
}

std::vector<std::string> TutorialLevelOne::GetText() {
  std::vector<std::string> sText;

  if (bFlying) {
    if (!bKilledKnight || !bPrincessGenerated) {
      sText.push_back(sSteerMessage);
      sText.push_back("fly back to your tower");

      return sText;
    } else {
      sText.push_back(sSteerMessage);
      sText.push_back("fly over the princess to pick her up");
      sText.push_back("bring captured princess to the tower");
#ifndef JOYSTICK_TUTORIAL
      sText.push_back("(you can shoot while flying!)");
#endif

      return sText;
    }
  }

  if (!bKilledKnight) {
    sText.push_back(sShootingMessage);
    sText.push_back("aim for the knights!");
    sText.push_back("don't let them get to the tower");

    return sText;
  }

  if (bPrincessGenerated) {
    if (!bPrincessCaptured) {
      sText.push_back("princess in sight!");
      sText.push_back(sTakeOffMessage);

      return sText;
    }

    sText.push_back("capture four princesses to beat the level");
    sText.push_back("don't let knights get to the tower!");

    return sText;
  }

  return sText;
}

void TutorialLevelOne::Update() {
  if (pTexter != 0)
    pTexter->SetText(GetText());
}

void TutorialLevelOne::KnightKilled() {
  if (bKilledKnight == false) {
    bKilledKnight = true;
    Update();
  }
}

void TutorialLevelOne::FlyOn() {
  bFlying = true;
  Update();
}

void TutorialLevelOne::FlyOff() {
  bFlying = false;
  Update();
}

void TutorialLevelOne::PrincessGenerate() {
  bPrincessGenerated = true;
  Update();
}

void TutorialLevelOne::PrincessCaptured() {
  bPrincessCaptured = true;
  Update();
}

std::vector<std::string> TutorialLevelTwo::GetText() {
  std::vector<std::string> sText;

  if (!bTraderGenerated)
    return sText;

  if (!bTraderKilled) {
    sText.push_back("trader in sight!");
    sText.push_back("kill a trader to get a power up");

    return sText;
  }

  if (!bBonusPickedUp) {
    sText.push_back("traders drop power ups");
    sText.push_back("fly over to pick them up");
    sText.push_back("collect as many as you can!");

    return sText;
  }

  return sText;
}

void TutorialLevelTwo::Update() {
  if (pTexter != 0)
    pTexter->SetText(GetText());
}

void TutorialLevelTwo::TraderKilled() {
  if (bTraderKilled == false) {
    bTraderKilled = true;
    Update();
  }
}

void TutorialLevelTwo::TraderGenerate() {
  bTraderGenerated = true;
  Update();
}

void TutorialLevelTwo::BonusPickUp() {
  bBonusPickedUp = true;
  Update();
}
