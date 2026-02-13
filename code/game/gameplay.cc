#include "game.h"
#include "controller/level_controller.h"
#include "smart_pointer.h"
#include "string_utils.h"

void BackgroundMusicPlayer::ToggleOff() {
  if (!bOff) {
    bOff = true;
    StopMusic();
  } else {
    bOff = false;
  }
}

void BackgroundMusicPlayer::SwitchTheme(int nTheme) {
  if (bOff)
    return;

  if (nCurrTheme == nTheme)
    return;

  if (nTheme < 0 || nTheme >= int(vThemes.size()))
    throw SimpleException("BackgroundMusicPlayer", "SwitchTheme",
                          "Bad theme index " + S(nTheme));

  StopMusic();
  pSnd->PlaySound(vThemes[nTheme], BG_MUSIC_CHANNEL, true);

  nCurrTheme = nTheme;
}

void BackgroundMusicPlayer::StopMusic() {
  if (nCurrTheme != -1)
    pSnd->StopSound(BG_MUSIC_CHANNEL);
  nCurrTheme = -1;
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

/*virtual*/ void HighScoreShower::Draw(smart_pointer<ScalingDrawer> pDr) {
  int nScale = 2;
  int nCharWidth = 4;

  Point p1 = Point(rBound.sz.x / 2 / nScale, rBound.sz.y / 2 / nScale - 6);
  Point p2 = Point(rBound.sz.x / 2 / nScale, rBound.sz.y / 2 / nScale + 1);

  std::string s1 = "score: ";
  std::string s2 = "high:  ";

  p1.x -= (nCharWidth * (s1.size() + 7)) / 2;

  pGl->pBigNum->DrawWord(s1, p1, false);

  p1.x += nCharWidth * s1.size();

  pGl->pBigNum->DrawNumber(pGl->nScore, p1, 7);

  p2.x -= (nCharWidth * (s2.size() + 7)) / 2;

  pGl->pBigNum->DrawWord(s2, p2, false);

  p2.x += nCharWidth * s2.size();

  pGl->pBigNum->DrawNumber(pGl->nHighScore, p2, 7);

  // pGl->pBigNum->DrawWord("score:", Point(0, 0), false);
  // pGl->pBigNum->DrawWord("high:", Point(0, 0), false);
}

/*virtual*/ void IntroTextShower::Draw(smart_pointer<ScalingDrawer> pDr) {
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

  int nHeight = pGl->pFancyNum->GetSize(" ").y + 2;

  pCnt.y -= (vText.size() * nHeight) / 2;

  for (unsigned i = 0; i < vText.size(); ++i) {
    pGl->pFancyNum->DrawWord(vText[i], pCnt, true);
    pCnt.y += nHeight;
  }
}

/*virtual*/ void BonusScore::Draw(smart_pointer<ScalingDrawer> pDr) {
  if (nC < 11)
    pAc->pGl->pNum->DrawWord(sText, p, true);
  else
    pAc->pGl->pNum->DrawColorWord(sText, p, c, true);
}

void BonusScore::Update() {
  if (pAc->bGhostTime)
    bExist = false;

  if (t.Tick()) {
    ++nC;
    if (nC < 11) {
      nScoreSoFar += nScore / 11;
      pAc->pGl->nScore += nScore / 11;
      --p.y;
    } else {
      c.R -= 50;
      c.G -= 50;
      // c.nTransparent -= 50;
    }

    if (nC == 11) {
      pAc->pGl->nScore += nScore - nScoreSoFar;
    }

    if (nC >= 15) {
      bExist = false;
    }
  }
}
