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

TutorialLevelOne::TutorialLevelOne(TutorialTextEntity *pTexter,
                                   std::string steer_message,
                                   std::string shooting_message,
                                   std::string take_off_message,
                                   bool show_flying_shoot_hint)
    : killed_knight_(false), flying_(false), princess_generated_(false),
      princess_captured_(false), pTexter_(pTexter),
      steer_message_(std::move(steer_message)),
      shooting_message_(std::move(shooting_message)),
      take_off_message_(std::move(take_off_message)),
      show_flying_shoot_hint_(show_flying_shoot_hint) {
  Update();
}

std::vector<std::string> TutorialLevelOne::GetText() {
  std::vector<std::string> text;

  if (flying_) {
    if (!killed_knight_ || !princess_generated_) {
      text.push_back(steer_message_);
      text.push_back("fly back to your tower");
      return text;
    } else {
      text.push_back(steer_message_);
      text.push_back("fly over the princess to pick her up");
      text.push_back("bring captured princess to the tower");
      if (show_flying_shoot_hint_)
        text.push_back("(you can shoot while flying!)");
      return text;
    }
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
      return text;
    }

    text.push_back("capture four princesses to beat the level");
    text.push_back("don't let knights get to the tower!");
    return text;
  }

  return text;
}

void TutorialLevelOne::Update() {
  if (pTexter_ != nullptr)
    pTexter_->SetText(GetText());
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
    : trader_generated_(false), trader_killed_(false),
      bonus_picked_up_(false), pTexter_(pTexter) {
  Update();
}

std::vector<std::string> TutorialLevelTwo::GetText() {
  std::vector<std::string> text;

  if (!trader_generated_)
    return text;

  if (!trader_killed_) {
    text.push_back("trader in sight!");
    text.push_back("kill a trader to get a power up");
    return text;
  }

  if (!bonus_picked_up_) {
    text.push_back("traders drop power ups");
    text.push_back("fly over to pick them up");
    text.push_back("collect as many as you can!");
    return text;
  }

  return text;
}

void TutorialLevelTwo::Update() {
  if (pTexter_ != nullptr)
    pTexter_->SetText(GetText());
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
