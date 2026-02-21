#include "entities.h"
#include "dragon_constants.h"
#include "dragon_macros.h"
#include "game/controller/dragon_game_controller.h"
#include "game/controller/level_controller.h"
#include "game_utils/draw_utils.h"
#include "utils/string_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/font_writer.h"
#include "wrappers/geometry.h"

#include <sstream>

void TextDrawEntity::SetText(std::string sText) {
  vText = BreakUpString(sText);
}

void TextDrawEntity::Draw(ScalingDrawer *pDr) {
  Point p = pos;
  for (unsigned i = 0; i < vText.size(); ++i) {
    pNum->DrawWord(vText[i], p, bCenter);
    p.y += 7;
  }
}

void SimpleVisualEntity::Update() {
  if (bTimer) {
    if (t.Tick()) {
      seq.Toggle();
      t = Timer(nPeriod * seq.GetTime());
    }
  } else if (bStep) {
    Point p = GetPosition();
    if (p != pPrev) {
      bImageToggle = !bImageToggle;

      if (bImageToggle)
        seq.Toggle();
    }
    pPrev = p;
  }
}

void SimpleVisualEntity::Draw(ScalingDrawer *pDr) {
  pDr->Draw(seq.GetImage(), GetPosition(), bCenter);
}

void StaticImage::Draw(ScalingDrawer *pDr) {
  pDr->Draw(img, GetPosition(), bCentered);
}

void StaticRectangle::Draw(ScalingDrawer *pDr) {
  pDr->pGr->DrawRectangle(r, c, false);
}

void SimpleSoundEntity::Update() {
  if (t.Tick()) {
    pSnd->PlaySound(seq.GetSound());

    if (seq.nActive == seq.vSounds.size() - 1) {
      bExist = false;
      return;
    }

    seq.Toggle();
    t = Timer(nPeriod * seq.GetTime());
  }
}

void AnimationOnce::Update() {
  SimpleVisualEntity::Update();

  if (SimpleVisualEntity::seq.nActive == 0) {
    if (!bOnce)
      bExist = false;
  } else {
    bOnce = false;
  }
}

bool PhysicalEntity::HitDetection(smart_pointer<PhysicalEntity> pPh) {
  return HitDetection(pPh.get());
}

bool PhysicalEntity::HitDetection(PhysicalEntity *pPh) {
  if (pPh == nullptr)
    return false;
  Point d = GetPosition() - pPh->GetPosition();
  unsigned r1 = GetRadius(), r2 = pPh->GetRadius();
  return unsigned(d.x * d.x + d.y * d.y) < (r1 * r1 + r2 * r2);
}

void Critter::Move() {
  fPos += fVel;
  if (!InsideRectangle(rBound, fPos.ToPnt())) {
    if (bDieOnExit)
      bExist = false;
    else {
      if (InsideRectangle(rBound, (fPos - fPoint(0, fVel.y)).ToPnt()))
        fPos.y -= fVel.y;
      else if (InsideRectangle(rBound, (fPos - fPoint(fVel.x, 0)).ToPnt()))
        fPos.x -= fVel.x;
      else
        fPos -= fVel;
    }
  }
}

void FancyCritter::Move() {
  if (tm.Tick()) {
    fPos += fVel;
    seq.Toggle();
  }

  if (!InsideRectangle(rBound, fPos.ToPnt())) {
    if (bDieOnExit)
      bExist = false;
    else {
      if (InsideRectangle(rBound, (fPos - fPoint(0, fVel.y)).ToPnt()))
        fPos.y -= fVel.y;
      else if (InsideRectangle(rBound, (fPos - fPoint(fVel.x, 0)).ToPnt()))
        fPos.x -= fVel.x;
      else
        fPos -= fVel;
    }
  }
}

BonusScore::BonusScore(LevelController *pAc_, Point p_, unsigned nScore_)
    : p(p_ + Point(0, -5)), t(unsigned(.1F * nFramesInSecond)), nC(0),
      pAc(pAc_), c(255, 255, 0), nScore(nScore_), nScoreSoFar(0) {
  std::ostringstream ostr(sText);
  ostr << '+' << nScore;
  sText = ostr.str();
}

void SoundControls::Update() {
  if (nTheme != -1)
    plr.SwitchTheme(nTheme);
  else
    plr.StopMusic();
}

/*virtual*/ void HighScoreShower::Draw(ScalingDrawer *pDr) {
  int nScale = 2;
  int nCharWidth = 4;

  Point p1 = Point(rBound.sz.x / 2 / nScale, rBound.sz.y / 2 / nScale - 6);
  Point p2 = Point(rBound.sz.x / 2 / nScale, rBound.sz.y / 2 / nScale + 1);

  std::string s1 = "score: ";
  std::string s2 = "high:  ";

  p1.x -= (nCharWidth * (s1.size() + 7)) / 2;

  pGl->GetBigNumberDrawer()->DrawWord(s1, p1, false);

  p1.x += nCharWidth * s1.size();

  pGl->GetBigNumberDrawer()->DrawNumber(pGl->GetScore(), p1, 7);

  p2.x -= (nCharWidth * (s2.size() + 7)) / 2;

  pGl->GetBigNumberDrawer()->DrawWord(s2, p2, false);

  p2.x += nCharWidth * s2.size();

  pGl->GetBigNumberDrawer()->DrawNumber(pGl->GetHighScore(), p2, 7);
}

/*virtual*/ void IntroTextShower::Draw(ScalingDrawer *pDr) {
  int nScale = 2;

  Point pCnt = Point(rBound.sz.x / 2 * nScale, rBound.sz.y / 2 * nScale);

  std::vector<std::string> vText;

  vText.push_back("I must bring the royal princesses back to my tower");
  vText.push_back("and defend it from the knights!");
  vText.push_back("---");
#ifdef PC_VERSION
  vText.push_back("CLICK screen to shoot, CLICK tower to fly");
  vText.push_back("CLICK + HOLD to steer");
  vText.push_back("---");
  vText.push_back("press SPACE to PLAY!");
#else

#ifndef SMALL_SCREEN_VERSION
  vText.push_back("TAP screen to shoot, TAP tower to fly");
  vText.push_back("TAP + HOLD to steer");
  vText.push_back("---");
  vText.push_back("double TAP to PLAY!");
#else
  vText.push_back("TAP screen to shoot, TAP tower to fly");
  vText.push_back("TAP + HOLD to steer");
  vText.push_back("OR use the GAMEPAD!");
  vText.push_back("---");
  vText.push_back("double TAP or press X to PLAY!");
#endif

#endif

  int nHeight = pGl->GetFancyFont()->GetSize(" ").y + 2;

  pCnt.y -= (vText.size() * nHeight) / 2;

  for (unsigned i = 0; i < vText.size(); ++i) {
    pGl->GetFancyFont()->DrawWord(vText[i], pCnt, true);
    pCnt.y += nHeight;
  }
}

/*virtual*/ void BonusScore::Draw(ScalingDrawer *pDr) {
  if (nC < 11)
    pAc->pGl->GetNumberDrawer()->DrawWord(sText, p, true);
  else
    pAc->pGl->GetNumberDrawer()->DrawColorWord(sText, p, c, true);
}

void BonusScore::Update() {
  if (pAc->bGhostTime)
    bExist = false;

  if (t.Tick()) {
    ++nC;
    if (nC < 11) {
      nScoreSoFar += nScore / 11;
      pAc->pGl->AddScore(nScore / 11);
      --p.y;
    } else {
      c.R -= 50;
      c.G -= 50;
    }

    if (nC == 11) {
      pAc->pGl->AddScore(nScore - nScoreSoFar);
    }

    if (nC >= 15) {
      bExist = false;
    }
  }
}
