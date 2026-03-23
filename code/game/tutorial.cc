#include "tutorial.h"
#include "controller/dragon_game_controller.h"
#include "../game_utils/draw_utils.h"

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

void TutorialTextEntity::Draw(ScalingDrawer * /*pDr*/) {
  if (pGl && !pGl->IsTutorialOnSetting())
    return;

  for (int i = 0; i < (int)sText.size(); ++i) {
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

void Tutorial::Update() {
  if (pTexter_ != nullptr)
    pTexter_->SetText(GetText());
}

TutorialLevelOne::TutorialLevelOne(TutorialTextEntity *pTexter, bool is_joystick,
                                   bool is_keyboard)
    : Tutorial(pTexter), killed_knight_(false), flying_(false),
      princess_generated_(false), princess_captured_(false),
      show_flying_shoot_hint_(!is_joystick) {
  if (is_joystick) {
    steer_message_    = "steer with joystick";
    shooting_message_ = "move joystick to shoot a fireball";
    take_off_message_ = "press button to take off";
  } else if (is_keyboard) {
    steer_message_    = "steer with left and right keys";
    shooting_message_ = "shoot with arrow keys";
    take_off_message_ = "press space to take off";
  } else {
    steer_message_    = "click and hold to steer";
    shooting_message_ = "click anywhere to shoot a fireball";
    take_off_message_ = "press space or click the tower to take off";
  }
  Update();
}

std::vector<std::string> TutorialLevelOne::GetText() {
  std::vector<std::string> text;

  if (flying_) {
    text.push_back(steer_message_);
    if (!killed_knight_ || !princess_generated_) {
      text.push_back("fly back to your tower");
    } else {
      text.push_back("fly over the princess to pick her up");
      text.push_back("bring captured princess to the tower");
      if (show_flying_shoot_hint_)
        text.push_back("(you can shoot while flying!)");
    }
    return text;
  }

  if (!killed_knight_) {
    text.push_back(shooting_message_);
    text.push_back("aim for the knights!");
    text.push_back("don't let them get to the tower");
    return text;
  }

  if (princess_generated_) {
    if (!princess_captured_) {
      text.push_back("princess in sight!");
      text.push_back(take_off_message_);
    } else {
      text.push_back("capture four princesses to beat the level");
      text.push_back("don't let knights get to the tower!");
    }
  }

  return text;
}

void TutorialLevelOne::Notify(TutorialEvent event) {
  switch (event) {
    case TutorialEvent::KnightKilled:
      if (!killed_knight_) {
        killed_knight_ = true;
        Update();
      }
      break;
    case TutorialEvent::FlyOn:
      flying_ = true;
      Update();
      break;
    case TutorialEvent::FlyOff:
      flying_ = false;
      Update();
      break;
    case TutorialEvent::PrincessGenerate:
      princess_generated_ = true;
      Update();
      break;
    case TutorialEvent::PrincessCaptured:
      princess_captured_ = true;
      Update();
      break;
    default:
      break;
  }
}

TutorialLevelTwo::TutorialLevelTwo(TutorialTextEntity *pTexter)
    : Tutorial(pTexter), trader_generated_(false), trader_killed_(false),
      bonus_picked_up_(false) {
  Update();
}

std::vector<std::string> TutorialLevelTwo::GetText() {
  if (!trader_generated_)
    return {};

  if (!trader_killed_)
    return {"trader in sight!", "kill a trader to get a power up"};

  if (!bonus_picked_up_)
    return {"traders drop power ups", "fly over to pick them up",
            "collect as many as you can!"};

  return {};
}

void TutorialLevelTwo::Notify(TutorialEvent event) {
  switch (event) {
    case TutorialEvent::TraderGenerate:
      trader_generated_ = true;
      Update();
      break;
    case TutorialEvent::TraderKilled:
      if (!trader_killed_) {
        trader_killed_ = true;
        Update();
      }
      break;
    case TutorialEvent::BonusPickUp:
      bonus_picked_up_ = true;
      Update();
      break;
    default:
      break;
  }
}
