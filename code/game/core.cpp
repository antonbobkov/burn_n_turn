#include "core.h"
#include "game.h"

Polar::Polar(fPoint p) : r(p.Length()) {
  if (p.y == 0 && p.x == 0)
    a = 0;
  else
    a = atan2(p.y, p.x);
}

fPoint ComposeDirection(int dir1, int dir2) {
  fPoint r(0, 0);
  switch (dir1) {
  case 1:
    r += fPoint(-1, 0);
    break;
  case 2:
    r += fPoint(1, 0);
    break;
  case 3:
    r += fPoint(0, 1);
    break;
  case 4:
    r += fPoint(0, -1);
    break;
  default:
    break;
  }
  switch (dir2) {
  case 1:
    r += fPoint(-1, 0);
    break;
  case 2:
    r += fPoint(1, 0);
    break;
  case 3:
    r += fPoint(0, 1);
    break;
  case 4:
    r += fPoint(0, -1);
    break;
  default:
    break;
  }
  r.Normalize();
  return r;
}

fPoint GetWedgeAngle(fPoint fDir, float dWidth, unsigned nWhich,
                     unsigned nHowMany) {
  if (nHowMany == 1)
    return fDir;

  float d = 3.1415F * 2 * dWidth / (nHowMany - 1) * nWhich;

  return (Polar(fDir) * Polar(d - 3.1415F * dWidth, 1)).TofPoint();
}

fPoint RandomAngle(fPoint fDir, float fRange) {
  return (Polar(fDir) *
          Polar((float(rand()) / RAND_MAX - .5F) * fRange * 2 * 3.1415F, 1))
      .TofPoint();
}

void ScalingDrawer::Scale(Index &pImg, int nFactor_) {
  if (nFactor_ < 0)
    nFactor_ = nFactor;

  Image *pOrig = pGr->GetImage(pImg);
  Index pRet = pGr->GetBlankImage(
      Size(pOrig->GetSize().x * nFactor_, pOrig->GetSize().y * nFactor_));
  Image *pFin = pGr->GetImage(pRet);

  Point p, s;
  for (p.y = 0; p.y < pOrig->GetSize().y; ++p.y)
    for (p.x = 0; p.x < pOrig->GetSize().x; ++p.x)
      for (s.y = 0; s.y < nFactor_; ++s.y)
        for (s.x = 0; s.x < nFactor_; ++s.x)
          pFin->SetPixel(Point(p.x * nFactor_ + s.x, p.y * nFactor_ + s.y),
                         pOrig->GetPixel(p));

  pImg = pRet;
}

void ScalingDrawer::Draw(Index nImg, Point p, bool bCentered) {
  p.x *= nFactor;
  p.y *= nFactor;
  if (bCentered) {
    Size sz = pGr->GetImage(nImg)->GetSize();
    p.x -= sz.x / 2;
    p.y -= sz.y / 2;
  }

  pGr->DrawImage(p, nImg, false);
}

Index ScalingDrawer::LoadImage(std::string strFile) {
  Index n = pGr->LoadImage(strFile);
  pGr->GetImage(n)->ChangeColor(Color(0, 0, 0), Color(0, 0, 0, 0));
  return n;
}

void NumberDrawer::CacheColor(Color c) {
  std::vector<Index> vNewColors;

  for (size_t i = 0, sz = vImg.size(); i < sz; ++i) {
    Index vColImg = pDr->pGr->CopyImage(vImg[i]);
    pDr->pGr->GetImage(vColImg)->ChangeColor(Color(255, 255, 0), c);
    vNewColors.push_back(vColImg);
  }

  mpCachedRecolorings[c] = vNewColors;
}

NumberDrawer::NumberDrawer(SP<ScalingDrawer> pDr_, Gui::FilePath *fp,
                           std::string sFontPath, std::string sFontName)
    : pDr(this, pDr_), vImgIndx(256, -1) {
  std::string txtPath = fp->GetRelativePath(sFontPath + sFontName + ".txt");
  std::unique_ptr<Gui::InStreamHandler> ih = fp->ReadFile(txtPath);
  std::istream &ifs = ih->GetStream();

  unsigned n;
  unsigned char c;
  for (n = 0; c = ifs.get(), !ifs.fail(); ++n)
    vImgIndx[c] = n;

  std::string bmpPath = fp->GetRelativePath(sFontPath + sFontName + ".bmp");
  Index nImg = pDr->LoadImage(bmpPath);
  Image *pImg = pDr->pGr->GetImage(nImg);
  for (unsigned i = 0; i < n; ++i) {
    Index nCurr = pDr->pGr->GetBlankImage(Size(3, 5));
    Image *pCurr = pDr->pGr->GetImage(nCurr);

    Point p;
    for (p.y = 0; p.y < 5; ++p.y)
      for (p.x = 0; p.x < 3; ++p.x)
        pCurr->SetPixel(p, pImg->GetPixel(Point(i * 4 + p.x, p.y)));
    pDr->Scale(nCurr);
    vImg.push_back(nCurr);
  }

  pDr->pGr->DeleteImage(nImg);
}

std::string NumberDrawer::GetNumber(unsigned n, unsigned nDigits) {
  std::string s;
  if (n == 0)
    s += '0';
  while (n != 0) {
    s += ('0' + n % 10);
    n /= 10;
  }

  unsigned i, sz = unsigned(s.size());
  for (i = 0; int(i) < int(nDigits) - int(sz); ++i)
    s += '0';

  std::reverse(s.begin(), s.end());

  return s;
}

void NumberDrawer::DrawWord(std::string s, Point p, bool bCenter) {
  if (bCenter) {
    p.x -= 2 * s.length();
    p.y -= 2;
  }

  for (unsigned i = 0; i < s.length(); ++i) {
    int n = int(s[i]);

    if (vImgIndx[n] == -1)
      continue;

    pDr->Draw(vImg[vImgIndx[n]], Point(p.x + 4 * i, p.y), false);
  }
}

void NumberDrawer::DrawColorWord(std::string s, Point p, Color c,
                                 bool bCenter) {
  if (bCenter) {
    p.x -= 2 * s.length();
    p.y -= 2;
  }

  std::vector<Index> *pImageVector;

  bool bManualRecolor = true;

  std::map<Color, std::vector<Index>>::iterator itr =
      mpCachedRecolorings.find(c);

  if (itr != mpCachedRecolorings.end()) {
    bManualRecolor = false;
    pImageVector = &(itr->second);
  }

  for (unsigned i = 0; i < s.length(); ++i) {
    int n = int(s[i]);

    if (vImgIndx[n] == -1)
      continue;

    if (bManualRecolor) {
      Index vColImg = pDr->pGr->CopyImage(vImg[vImgIndx[n]]);
      pDr->pGr->GetImage(vColImg)->ChangeColor(Color(255, 255, 0), c);
      pDr->Draw(vColImg, Point(p.x + 4 * i, p.y), false);
    } else {
      pDr->Draw((*pImageVector)[vImgIndx[n]], Point(p.x + 4 * i, p.y), false);
    }
  }
}

void SoundInterfaceProxy::PlaySound(Index i, int nChannel, bool bLoop) {
  if (bSoundOn)
    pSndRaw->PlaySound(i, nChannel, bLoop);
}

std::vector<std::string> BreakUpString(std::string s) {
  s += '\n';

  std::vector<std::string> vRet;
  std::string sCurr;

  for (unsigned i = 0; i < s.size(); ++i) {
    if (s[i] == '\n') {
      vRet.push_back(sCurr);
      sCurr = "";
    } else
      sCurr += s[i];
  }

  return vRet;
}

unsigned GetRandNum(unsigned nRange) {
  return unsigned(float(rand()) / (float(RAND_MAX) + 1) * nRange);
}

unsigned GetRandFromDistribution(std::vector<float> vProb) {
  float fSum = 0;

  for (unsigned i = 0; i < vProb.size(); ++i)
    fSum += vProb[i];

  float fRand = float(rand()) / (float(RAND_MAX) + 1) * fSum;

  fSum = 0;

  for (unsigned i = 0; i < vProb.size(); ++i) {
    fSum += vProb[i];
    if (fSum > fRand)
      return i;
  }

  throw std::string("bad range");
}

MessageWriter *pWr = 0;

int nSlimeMax = 100;

void DrawStuff(Rectangle rBound, SP<Graphic> pGraph, SP<Soundic> pSnd,
               Preloader &pr, int n) {
#ifdef LOADING_SCREEN
  rBound.sz.x *= 2;
  rBound.sz.y *= 2;

  Size sz1 = pGraph->GetImage(pr["splash"])->GetSize();
  Size sz2 = pGraph->GetImage(pr["loading"])->GetSize();

  Point p1(Crd(rBound.sz.x / 2), Crd(rBound.sz.y / 2));
#ifdef SMALL_SCREEN_VERSION
        Point p2(Crd(rBound.sz.x/2, Crd(rBound.sz.y*7.0f/10);
#else
  Point p2(Crd(rBound.sz.x / 2), Crd(rBound.sz.y * 6.5f / 10));
#endif

	p1.x -= sz1.x/2;
	p1.y -= sz1.y/2;
	p2.x -= sz2.x/2;
	p2.y -= sz2.y/2;

	pGraph->DrawImage(p1, pr["splash"], false);

	pGraph->DrawImage(p2, pr["loading"], Rectangle(0,0,144*n/9,32), false);

	pGraph->DrawImage(p2, pr["loading"], Rectangle(0,0,144*n/9,32), false);

	pGraph->RefreshAll();

#ifndef PC_VERSION
	if(n%2)
		pSnd->PlaySound(pr.GetSnd("beep"));
	else pSnd->PlaySound(pr.GetSnd("boop"));
#endif

#endif // LOADING_SCREEN
}

const std::string sFullScreenPath = "fullscreen.txt";

TwrGlobalController::TwrGlobalController(
    SP<ScalingDrawer> pDr_, SP<NumberDrawer> pNum_, SP<NumberDrawer> pBigNum_,
    SP<FontWriter> pFancyNum_, SP<Soundic> pSndRaw_, const LevelStorage &vLvl_,
    Rectangle rBound_, TowerDataWrap *pWrp_, FilePath *fp)
    : nActive(1), pDr(this, pDr_), pGraph(this, pDr_->pGr), pNum(this, pNum_),
      pBigNum(this, pBigNum_), pr(pDr_->pGr, pSndRaw_, fp),
      pSndRaw(this, pSndRaw_), pSnd(this, new SoundInterfaceProxy(pSndRaw)),
      nScore(0), vLvl(vLvl_), rBound(rBound_), bAngry(false), nHighScore(0),
      pFancyNum(this, pFancyNum_), pWrp(pWrp_), pMenu(this, 0),
      vLevelPointers(3), sbTutorialOn(fp, "tutorial_on.txt", true, true),
      snProgress(fp, "stuff.txt", 0, true),
      sbFullScreen(fp, sFullScreenPath, false, true),
      sbSoundOn(fp, "soundon.txt", true, true),
      sbMusicOn(fp, "musicon.txt", true, true),
      sbCheatsOn(fp, "cheat.txt", false, true),
      sbCheatsUnlocked(fp, "more_stuff.txt", false, true) {
  {
    if (fp->FileExists("high.txt")) {
      std::unique_ptr<Gui::InStreamHandler> ih = fp->ReadFile("high.txt");
      ih->GetStream() >> nHighScore;
    }
  }

  typedef ImagePainter::ColorMap ColorMap;

  std::vector<ColorMap> vColors;
  vColors.push_back(ColorMap(Color(0, 255, 0), Color(0, 0, 255)));
  vColors.push_back(ColorMap(Color(0, 128, 0), Color(0, 0, 128)));
  vColors.push_back(ColorMap(Color(0, 127, 0), Color(0, 0, 127)));
  vColors.push_back(ColorMap(Color(0, 191, 0), Color(0, 0, 191)));

  unsigned nScale = 2;

  pr.AddTransparent(Color(0, 0, 0));
  pr.SetScale(nScale);

  pr.LoadTS("icons.bmp", "loading");
  pr.LoadTS("robotbear.bmp", "splash");
  pr.LoadSnd("beep.wav", "beep");
  pr.LoadSnd("boop.wav", "boop");
  DrawStuff(rBound, pGraph, pSndRaw_, pr, 0);

  pr.LoadTS("road.bmp", "road");
  pr.LoadTS("turnandvorn.bmp", "logo");
#ifdef TRIAL_VERSION
  pr.LoadTS("trial.bmp", "trial");
  pr.LoadTS("buy_now.bmp", "buy");
#endif
  pr.LoadSeqTS("burn\\burn.txt", "burn");
  pr.LoadTS("empty.bmp", "empty");

  pr.LoadSeqTS("arrow\\sword.txt", "arrow");
  pr.LoadSeqTS("arrow\\claw.txt", "claw");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 1);

  pr.LoadSeqTS("corona\\crosshair.txt", "corona"); // (not used icon)
  pr.LoadSeqTS("bonus\\void.txt", "void_bonus");
  pr.LoadSeqTS("bonus\\pershot.txt", "pershot_bonus"); // 1 - pershot
  pr.LoadSeqTS("bonus\\laser.txt", "laser_bonus");     // 2 - laser
  pr.LoadSeqTS("bonus\\big.txt", "big_bonus");         // 3 - big
  pr.LoadSeqTS("bonus\\totnum.txt", "totnum_bonus");   // 4 - totnum
  pr.LoadSeqTS("bonus\\explode.txt", "explode_bonus"); // 5 - explode
  pr.LoadSeqTS("bonus\\split.txt", "split_bonus");     // 6 - split
  pr.LoadSeqTS("bonus\\burning.txt", "burning_bonus"); // 7 - burning
  pr.LoadSeqTS("bonus\\ring.txt", "ring_bonus");       // 8 - ring
  pr.LoadSeqTS("bonus\\nuke.txt", "nuke_bonus");       // 9 - nuke
  pr.LoadSeqTS("bonus\\speed.txt", "speed_bonus");     // 10 - speed
  // pr.LoadSeqTS("bonus\\frequency.txt", "frequency");		// 11 -
  // frequency (not used)

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 2);
  pr.LoadSeqTS("start.txt", "start");
  pr.LoadSeqTS("win\\win.txt", "win");
  pr.LoadSeqTS("win\\over.txt", "over");
  pr.LoadSeqTS("logo\\pluanbo.txt", "pluanbo", Color(0, 0, 0), nScale * 2);
  pr.LoadSeqTS("logo\\gengui.txt", "gengui", Color(0, 0, 0), nScale * 2);
  pr.LoadSeqTS("castle\\castle.txt", "castle", Color(0, 0, 0));
  pr.LoadSeqTS("castle\\destroy_castle_dust.txt", "destroy_castle",
               Color(0, 0, 0));

  pr.LoadSeqTS("dragon_fly\\fly.txt", "dragon_fly");
  pr.LoadSeqTS("dragon\\stable.txt", "dragon_stable");
  pr.LoadSeqTS("dragon\\walk.txt", "dragon_walk");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 3);

  pr.AddSequence(pr("dragon_fly"), "bdragon_fly");
  pr.AddSequence(pr("dragon_stable"), "bdragon_stable");
  pr.AddSequence(pr("dragon_walk"), "dragon_walk_f");

  ForEachImage(pr("bdragon_fly"), ImagePainter(pGraph, vColors));
  ForEachImage(pr("bdragon_stable"), ImagePainter(pGraph, vColors));
  ForEachImage(pr("dragon_walk_f"), ImageFlipper(pGraph));

  pr.LoadSeqTS("explosion\\explosion2.txt", "explosion");
  pr.LoadSeqTS("explosion\\laser_expl.txt", "laser_expl");

  pr.LoadSeqTS("explosion\\explosion_15.txt", "explosion_15");
  pr.LoadSeqTS("explosion\\laser_expl_15.txt", "laser_expl_15");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 4);

  pr.LoadSeqTS("explosion\\explosion2.txt", "explosion_2", Color(0, 0, 0, 0),
               nScale * 2);
  pr.LoadSeqTS("explosion\\laser_expl.txt", "laser_expl_2", Color(0, 0, 0, 0),
               nScale * 2);

  pr.LoadSeqTS("explosion\\explosion_15.txt", "explosion_3", Color(0, 0, 0, 0),
               nScale * 2);
  pr.LoadSeqTS("explosion\\laser_expl_15.txt", "laser_expl_3",
               Color(0, 0, 0, 0), nScale * 2);

  pr.LoadSeqTS("fireball\\fireball.txt", "fireball");
  pr.LoadSeqTS("fireball\\fireball_15.txt", "fireball_15");
  pr.LoadSeqTS("fireball\\fireball.txt", "fireball_2", Color(0, 0, 0, 0),
               nScale * 2);
  pr.LoadSeqTS("fireball\\fireball_15.txt", "fireball_3", Color(0, 0, 0, 0),
               nScale * 2);

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 5);

  pr.LoadSeqTS("fireball\\laser.txt", "laser");
  pr.LoadSeqTS("fireball\\laser_15.txt", "laser_15");
  pr.LoadSeqTS("fireball\\laser.txt", "laser_2", Color(0, 0, 0, 0), nScale * 2);
  pr.LoadSeqTS("fireball\\laser_15.txt", "laser_3", Color(0, 0, 0, 0),
               nScale * 2);
  pr.LoadSeqTS("fireball\\fireball_icon.txt", "fireball_icon",
               Color(255, 255, 255));

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 6);

  pr.LoadSeqTS("knight\\knight.txt", "knight");
  pr.LoadSeqTS("knight\\die.txt", "knight_die");
  pr.LoadSeqTS("knight\\fire.txt", "knight_fire");
  pr.LoadSeqTS("ghostnight\\ghost_knight_burn.txt", "ghost_knight_burn");
  pr.LoadSeqTS("ghostnight\\ghost_knight.txt", "ghost_knight");
  pr.LoadSeqTS("ghostnight\\ghost_burn.txt", "ghost_burn");
  pr.LoadSeqTS("ghostnight\\ghost.txt", "ghost");

  pr.LoadSeqTS("golem\\golem.txt", "golem");
  pr.LoadSeqTS("golem\\golem_death.txt", "golem_die");

  pr.AddSequence(pr("golem"), "golem_f");
  ForEachImage(pr("golem_f"), ImageFlipper(pGraph));

  pr.AddSequence(pr("golem_die"), "golem_die_f");
  ForEachImage(pr("golem_die_f"), ImageFlipper(pGraph));

  pr.LoadSeqTS("skelly\\skelly.txt", "skelly");
  pr.LoadSeqTS("skelly\\die.txt", "skelly_die");
  pr.LoadSeqTS("skelly\\summon.txt", "skelly_summon");

  pr.LoadSeqTS("trader\\trader.txt", "trader");
  pr.LoadSeqTS("trader\\die.txt", "trader_die");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 7);

  pr.AddSequence(pr("trader"), "trader_f");
  pr.AddSequence(pr("trader_die"), "trader_die_f");
  ForEachImage(pr("trader_f"), ImageFlipper(pGraph));
  ForEachImage(pr("trader_die_f"), ImageFlipper(pGraph));

  pr.LoadSeqTS("princess\\princess.txt", "princess");
  pr.LoadSeqTS("princess\\die.txt", "princess_die");

  pr.AddSequence(pr("princess"), "princess_f");
  pr.AddSequence(pr("princess_die"), "princess_die_f");
  ForEachImage(pr("princess_f"), ImageFlipper(pGraph));
  ForEachImage(pr("princess_die_f"), ImageFlipper(pGraph));

  pr.LoadSeqTS("mage\\mage.txt", "mage");
  pr.LoadSeqTS("mage\\spell.txt", "mage_spell");
  pr.LoadSeqTS("mage\\die.txt", "mage_die");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 8);

  pr.AddSequence(pr("mage"), "mage_f");
  pr.AddSequence(pr("mage_spell"), "mage_spell_f");
  pr.AddSequence(pr("mage_die"), "mage_die_f");
  ForEachImage(pr("mage_f"), ImageFlipper(pGraph));
  ForEachImage(pr("mage_spell_f"), ImageFlipper(pGraph));
  ForEachImage(pr("mage_die_f"), ImageFlipper(pGraph));

  pr.LoadSeqTS("slime\\slime_walk.txt", "slime");
  pr.LoadSeqTS("slime\\slime_die.txt", "slime_die");
  pr.LoadSeqTS("slime\\slime_poke.txt", "slime_poke");
  pr.LoadSeqTS("slime\\slime_reproduce.txt", "slime_reproduce");
  pr.LoadSeqTS("slime\\slime_reproduce_fast.txt", "slime_reproduce_fast");
  pr.LoadSeqTS("slime\\slime_cloud.txt", "slime_cloud");

  pr.LoadSeqTS("slime\\mega_slime_walk.txt", "megaslime");
  pr.LoadSeqTS("slime\\mega_slime_die.txt", "megaslime_die");
  pr.LoadSeqTS("slime\\mega_slime_reproduce.txt", "megaslime_reproduce");

  pr.LoadSndSeq("sound\\over.txt", "over");
  pr.LoadSndSeq("sound\\pluanbo.txt", "pluanbo");
  pr.LoadSndSeq("sound\\click.txt", "click");

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 9);

  pr.LoadSnd("start_game.wav", "start_game");
  pr.LoadSnd("death01.wav", "death");
  pr.LoadSnd("golem_death2.wav", "golem_death");
  pr.LoadSnd("megaslime_spawn1.wav", "slime_spawn");
  pr.LoadSnd("megaslime_spawn2.wav", "megaslime_die");
  pr.LoadSnd("megaslime_hit.wav", "megaslime_hit");
  pr.LoadSnd("megaslime_bonus2.wav", "megaslime_bonus");
  pr.LoadSnd("megaslime_move4.wav", "megaslime_jump");
  pr.LoadSnd("megaslime_move3.wav", "megaslime_land");
  pr.LoadSnd("skeleton_bonus1.wav", "skeleton_bonus");

  pr.LoadSnd("explosion01.wav", "explosion");
  pr.LoadSnd("powerup03.wav", "powerup");
  pr.LoadSnd("laser04.wav", "laser");
  pr.LoadSnd("shoot2.wav", "shoot");
  pr.LoadSnd("dropping.wav", "knight_fall");
  pr.LoadSnd("knight_from_sky.wav", "dropping");
  pr.LoadSnd("pickup_sound_pxtone.wav", "pickup");
  pr.LoadSnd("princess_capture.wav", "princess_capture");
  pr.LoadSnd("win_level_2.wav", "win_level");
  pr.LoadSnd("all_princess_escape.wav", "all_princess_escape");
  pr.LoadSnd("destroy_castle_sound.wav", "destroy_castle_sound");
  pr.LoadSnd("one_princess.wav", "one_princess");
  pr.LoadSnd("leave_tower.wav", "leave_tower");
  pr.LoadSnd("return_tower.wav", "return_tower");
  pr.LoadSnd("step_left.wav", "step_left");
  pr.LoadSnd("step_right.wav", "step_right");
  pr.LoadSnd("hit_golem.wav", "hit_golem");
  pr.LoadSnd("slime_poke.wav", "slime_poke");
  pr.LoadSnd("slime_summon.wav", "slime_summon");
  pr.LoadSnd("princess_arrive.wav", "princess_arrive");

  pr.LoadSnd("sound/A.wav", "A");
  pr.LoadSnd("sound/B.wav", "B");
  pr.LoadSnd("sound/C.wav", "C");
  pr.LoadSnd("sound/D.wav", "D");
  pr.LoadSnd("sound/E.wav", "E");

  pr.LoadSnd("dddragon.wav", "background_music");
  pr.LoadSnd("_dddragon.wav", "background_music_slow");

#ifdef FULL_VERSION
  pr.LoadSnd("dddragon1.wav", "background_music2");
  pr.LoadSnd("_dddragon1.wav", "background_music_slow2");

  pr.LoadSnd("dddragon2.wav", "background_music3");
  pr.LoadSnd("_dddragon2.wav", "background_music_slow3");
#else // save memory on trial, though, *something* needs to be loaded otherwise
      // crashes.
  pr.LoadSnd("dddragon.wav", "background_music2");
  pr.LoadSnd("_dddragon.wav", "background_music_slow2");

  pr.LoadSnd("dddragon.wav", "background_music3");
  pr.LoadSnd("_dddragon.wav", "background_music_slow3");
#endif

  plr.pSnd = pSndRaw_;

  plr.vThemes.resize(6);
  plr.vThemes[BG_BACKGROUND] = pr.GetSnd("background_music");
  plr.vThemes[BG_SLOW_BACKGROUND] = pr.GetSnd("background_music_slow");
  plr.vThemes[BG_BACKGROUND2] = pr.GetSnd("background_music2");
  plr.vThemes[BG_SLOW_BACKGROUND2] = pr.GetSnd("background_music_slow2");
  plr.vThemes[BG_BACKGROUND3] = pr.GetSnd("background_music3");
  plr.vThemes[BG_SLOW_BACKGROUND3] = pr.GetSnd("background_music_slow3");

  plr.pSnd->SetVolume(.5);

  if (!sbMusicOn.Get())
    plr.ToggleOff();

  if (!sbSoundOn.Get())
    pSnd->Toggle();

  pNum->CacheColor(Color(205, 205, 0));
  pNum->CacheColor(Color(155, 155, 0));
  pNum->CacheColor(Color(105, 105, 0));
  pNum->CacheColor(Color(55, 55, 0));
  pNum->CacheColor(Color(5, 5, 0));
}

void TwrGlobalController::StartUp() {
  nScore = 0;
  bAngry = false;

  SP<Animation> pStr =
      new Animation(0, pr("start"), nFramesInSecond / 5,
                    Point(rBound.sz.x / 2, rBound.sz.y * 3 / 4), true);

  // menu
  SP<MenuController> pMenuHolder = new MenuController(
      this, rBound, Color(0, 0, 0), 3); // resume position shouldn't matter
  pMenu = pMenuHolder;

  // logo 1
  SP<BasicController> pCnt0_1 =
      new AlmostBasicController(BasicController(this, rBound, Color(0, 0, 0)));
  // logo 2
  SP<BasicController> pCnt0_2 =
      new AlmostBasicController(BasicController(this, rBound, Color(0, 0, 0)));
  // press start screen
  SP<BasicController> pCnt1 =
      new StartScreenController(this, rBound, Color(0, 0, 0));
  // game over
  SP<BasicController> pCnt2 =
      new AlmostBasicController(BasicController(this, rBound, Color(0, 0, 0)));
  // you win!
  SP<BasicController> pCnt3 = new BasicController(this, rBound, Color(0, 0, 0));
  // score
  SP<DragonScoreController> pScore =
      new DragonScoreController(this, rBound, Color(0, 0, 0), true);
  // intro tutorial screen (non PC version)
  SP<DragonScoreController> pIntro =
      new DragonScoreController(this, rBound, Color(0, 0, 0), false);

  // cutscenes
  SP<BasicController> pCut1 = new Cutscene(this, rBound, "princess", "knight");
  SP<BasicController> pCut2 =
      new Cutscene(this, rBound, "knight", "dragon_walk_f", true);
  SP<BasicController> pCut3 = new Cutscene(this, rBound, "dragon_walk", "mage");

  SP<SoundControls> pBckgMusic = new SoundControls(plr, BG_BACKGROUND);
  SP<SoundControls> pNoMusic = new SoundControls(plr, -1);

  SP<Animation> pWin = new Animation(
      0, pr("win"), 3, Point(rBound.sz.x / 2, rBound.sz.y / 2 - 20), true);
  SP<StaticImage> pL = new StaticImage(
      pr["logo"], Point(rBound.sz.x / 2, rBound.sz.y / 3), true);
  SP<Animation> pBurnL =
      new Animation(0, pr("burn"), 3,
                    Point(rBound.sz.x / 2 - 45, rBound.sz.y / 2 - 64), true);
  SP<Animation> pBurnR =
      new Animation(0, pr("burn"), 4,
                    Point(rBound.sz.x / 2 - 54, rBound.sz.y / 2 - 64), true);
  pBurnR->seq.nActive += 4;

  std::vector<std::string> vHintPref;
  vHintPref.push_back("hint: ");
  vHintPref.push_back("tip: ");
  vHintPref.push_back("secret: ");
  vHintPref.push_back("beware: ");
  vHintPref.push_back("fun fact: ");

  std::vector<std::string> vHints;
  vHints.push_back("you can shoot while flying");
  vHints.push_back("spooky things happen when time runs out");
  vHints.push_back("more princesses you capture - more knights show up");
  vHints.push_back("you can capture multiple princesses in one flight");
#ifndef KEYBOARD_CONTROLS
  vHints.push_back("pick up traders by clicking when flying over them\nbring "
                   "them to the tower to get longer bonuses");
#endif
  vHints.push_back("this game was originally called tower defense");

  std::string sHint = vHintPref.at(rand() % vHintPref.size()) +
                      vHints.at(rand() % vHints.size());

  SP<TextDrawEntity> pHintText = new TextDrawEntity(
      0, Point(rBound.sz.x / 2, rBound.sz.y * 7 / 8), true, sHint, pNum);
  SP<TextDrawEntity> pOptionText = new TextDrawEntity(
      0, Point(rBound.sz.x / 2, rBound.sz.y * 7 / 8), true, "sup", pNum);

  SP<AnimationOnce> pO =
      new AnimationOnce(0, pr("over"), nFramesInSecond / 2,
                        Point(rBound.sz.x / 2, Crd(rBound.sz.y / 2.5f)), true);
  SP<AnimationOnce> pPlu =
      new AnimationOnce(0, pr("pluanbo"), nFramesInSecond / 10,
                        Point(rBound.sz.x / 2, rBound.sz.y / 2), true);
  SP<AnimationOnce> pGen =
      new AnimationOnce(0, pr("gengui"), nFramesInSecond / 5,
                        Point(rBound.sz.x / 2, rBound.sz.y / 2), true);

  SP<SimpleSoundEntity> pOver =
      new SimpleSoundEntity(pr.GetSndSeq("over"), nFramesInSecond / 2, pSnd);
  SP<SimpleSoundEntity> pPluSnd = new SimpleSoundEntity(
      pr.GetSndSeq("pluanbo"), nFramesInSecond / 10, pSnd);
  SP<SimpleSoundEntity> pClkSnd =
      new SimpleSoundEntity(pr.GetSndSeq("click"), nFramesInSecond / 5, pSnd);

#ifdef TRIAL_VERSION
  SP<StaticImage> pTrial = new StaticImage(
      pr["trial"], Point(rBound.sz.x / 2 - 73, rBound.sz.y / 3 + 28), true);
  pCnt1->AddV(pTrial);
#endif

  SP<Animation> pMenuCaret =
      new Animation(2, pr("arrow"), 3, Point(0, 0), true);
  // menu entity
  SP<MenuDisplay> pMenuDisplay =
      new MenuDisplay(Point(rBound.sz.x / 2 - 8, rBound.sz.y / 2), pNum,
                      pMenuCaret, pMenu, sbCheatsUnlocked.Get());

  pMenu->AddBoth(pMenuDisplay);
  pMenu->pMenuDisplay = pMenuDisplay;

  pCnt1->AddBoth(pStr);

  pCnt1->AddV(pL);
  pCnt1->AddBoth(pBurnL);
  pCnt1->AddBoth(pBurnR);

  pMenu->AddV(pL);
  pMenu->AddBoth(pBurnL);
  pMenu->AddBoth(pBurnR);
  pMenu->AddBoth(pMenuCaret);

#ifdef PC_VERSION
  pMenu->pHintText = pHintText;
  pMenu->pOptionText = pOptionText;
#else
  pCnt1->AddV(pHintText);
#endif

  pCnt3->AddBoth(pWin);
  pCnt2->AddBoth(pO);
  pCnt2->AddE(pOver);

  pCnt0_1->AddBoth(pPlu);
  pCnt0_1->AddE(pPluSnd);

  pCnt0_2->AddBoth(pGen);
  pCnt0_2->AddE(pClkSnd);

  pCnt1->AddE(pBckgMusic);

  pMenu->AddE(pNoMusic);

  pCut1->AddE(pNoMusic);
  pCut2->AddE(pNoMusic);
  pCut3->AddE(pNoMusic);

  pCnt2->AddE(pNoMusic);
  pCnt3->AddE(pNoMusic);

  PushBackASSP(this, vCnt, pMenuHolder); // menu
  PushBackASSP(this, vCnt, pCnt0_1);     // logo 1
  PushBackASSP(this, vCnt, pCnt0_2);     // logo 2
  PushBackASSP(this, vCnt, pCnt1);       // press start screen
#ifndef PC_VERSION
  PushBackASSP(this, vCnt, pIntro); // tutorial screen
#endif

  for (unsigned i = 0; i < vLvl.size(); ++i) {
    SP<AdvancedController> pAd =
        new AdvancedController(this, rBound, Color(0, 0, 0), vLvl[i]);

    pAd->AddE(pBckgMusic);
    pAd->pSc = pBckgMusic;

    // game level
    PushBackASSP(this, vCnt, pAd);

    // chapters
    if (i == 0)
      vLevelPointers.at(0) = vCnt.size() - 1;
    else if (i == 3)
      vLevelPointers.at(1) = vCnt.size() - 1;
    else if (i == 6)
      vLevelPointers.at(2) = vCnt.size() - 1;

    // cutscene
    if (i == 2)
      PushBackASSP(this, vCnt, pCut1);
    if (i == 5)
      PushBackASSP(this, vCnt, pCut2);
    if (i == 8)
      PushBackASSP(this, vCnt, pCut3);
  }

#ifdef TRIAL_VERSION
  SP<BuyNowController> pBuy =
      new BuyNowController(this, rBound, Color(0, 0, 0));

  SP<Animation> pGolem =
      new Animation(0, pr("golem_f"), nFramesInSecond / 10,
                    Point(rBound.sz.x / 4, rBound.sz.y * 3 / 4 - 10), true);
  SP<Animation> pSkeleton1 =
      new Animation(0, pr("skelly"), nFramesInSecond / 4,
                    Point(rBound.sz.x * 3 / 4, rBound.sz.y * 3 / 4 - 5), true);
  SP<Animation> pSkeleton2 = new Animation(
      0, pr("skelly"), nFramesInSecond / 4 + 1,
      Point(rBound.sz.x * 3 / 4 - 10, rBound.sz.y * 3 / 4 - 15), true);
  SP<Animation> pSkeleton3 = new Animation(
      0, pr("skelly"), nFramesInSecond / 4 - 1,
      Point(rBound.sz.x * 3 / 4 + 10, rBound.sz.y * 3 / 4 - 15), true);
  SP<Animation> pMage =
      new Animation(0, pr("mage_spell"), nFramesInSecond / 2,
                    Point(rBound.sz.x / 2, rBound.sz.y * 3 / 4), true);
  SP<Animation> pGhost =
      new Animation(0, pr("ghost"), nFramesInSecond / 6,
                    Point(rBound.sz.x * 5 / 8, rBound.sz.y * 3 / 4 - 30), true);
  SP<Animation> pWhiteKnight =
      new Animation(0, pr("ghost_knight"), nFramesInSecond / 6,
                    Point(rBound.sz.x * 3 / 8, rBound.sz.y * 3 / 4 - 30), true);

  // SP<StaticImage> pBuyNow = new StaticImage(pr["buy"],Point(rBound.sz.x/2 -
  // 73,rBound.sz.y/3 + 33), true);
  SP<StaticImage> pBuyNow = new StaticImage(
      pr["buy"], Point(rBound.sz.x / 2, rBound.sz.y / 3 + 33), true);
  SP<VisualEntity> pSlimeUpd = new SlimeUpdater(pBuy.GetRawPointer());

  pBuy->AddV(pL);
  pBuy->AddV(pSlimeUpd);
  pBuy->AddBoth(pBurnL);
  pBuy->AddBoth(pBurnR);
  pBuy->AddV(pBuyNow);

  pBuy->AddBoth(pGolem);
  pBuy->AddBoth(pSkeleton1);
  pBuy->AddBoth(pSkeleton2);
  pBuy->AddBoth(pSkeleton3);
  pBuy->AddBoth(pMage);
  pBuy->AddBoth(pGhost);
  pBuy->AddBoth(pWhiteKnight);
#endif

#ifdef FULL_VERSION
  PushBackASSP(this, vCnt, pCnt3);  // you win
  PushBackASSP(this, vCnt, pCnt2);  // game over
  PushBackASSP(this, vCnt, pScore); // score
#else
  PushBackASSP(this, vCnt, pCnt2);
  PushBackASSP(this, vCnt, pBuy);
#endif
}

void TwrGlobalController::Next() {
  if (nActive == vCnt.size() - 1)
    Restart();
  else {
    ++nActive;

    for (unsigned i = 0; i < vLevelPointers.size(); ++i) {
      if (nActive == vLevelPointers.at(i) && snProgress.Get() < int(i)) {
        snProgress.Set(i);
        pMenu->pMenuDisplay->UpdateMenuEntries();
      }
    }
  }
}

void TwrGlobalController::Restart(int nActive_ /* = -1*/) {
  if (nActive_ == -1)
    nActive = 3;
  else
    nActive = nActive_;

  vCnt.clear();

  CleanIslandSeeded(this);

  StartUp();
}

void TwrGlobalController::Menu() {
  pMenu->nResumePosition = nActive;
  nActive = 0;
}

TowerGameGlobalController::TowerGameGlobalController(ProgramEngine pe) {
  pData = new TowerDataWrap(pe);
}

TowerGameGlobalController::~TowerGameGlobalController() {
  SP_Info *pCleanUp = pData->pCnt.GetRawPointer();
  delete pData;
  CleanIslandSeeded(pCleanUp);
}
TowerDataWrap::TowerDataWrap(ProgramEngine pe) {
  szActualRez = pe.szActualRez;

  pExitProgram = pe.pExitProgram;

  pWr = pe.pMsg.GetRawPointer();

  p_fm_ = pe.GetFileManager();
  {
    std::unique_ptr<Gui::InStreamHandler> ih = p_fm_->ReadFile("config.txt");
    fp_ = Gui::FilePath::CreateFromStream(ih->GetStream(), p_fm_);
  }

  Rectangle sBound = Rectangle(pe.szScreenRez);
  unsigned nScale = 2;
  Rectangle rBound =
      Rectangle(0, 0, sBound.sz.x / nScale, sBound.sz.y / nScale);

  pGr = pe.pGr;
  pSm = pe.pSndMng;

  pDr = new ScalingDrawer(pGr, nScale);

  SP<ScalingDrawer> pBigDr = new ScalingDrawer(pGr, nScale * 2);

  std::string sFontPath = "dragonfont\\";

  pNum = new NumberDrawer(pDr, fp_.get(), sFontPath, "dragonfont");
  pBigNum = new NumberDrawer(pBigDr, fp_.get(), sFontPath, "dragonfont");
  pFancyNum = new FontWriter(fp_.get(), "dragonfont\\dragonfont2.txt", pGr, 2);

  std::string levelsFile;
#ifdef FULL_VERSION
#ifdef SMALL_SCREEN_VERSION
  levelsFile = "levels_small.txt";
#else
  levelsFile = "levels.txt";
#endif
#else
  levelsFile = "levels_trial.txt";
#endif
  ReadLevels(fp_.get(), levelsFile, rBound, vLvl);

  pCnt = new TwrGlobalController(pDr, pNum, pBigNum, pFancyNum, pSm, vLvl,
                                 rBound, this, fp_.get());
  pCnt->StartUp();
}

void TowerGameGlobalController::Update() {
  pData->pCnt->vCnt[pData->pCnt->nActive]->Update();
}

void TowerGameGlobalController::KeyDown(GuiKeyType nCode) {
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnKey(nCode, false);

#ifndef PC_VERSION
  if (nCode == GUI_ESCAPE)
    Trigger(pData->pExitProgram);
#endif
}

void TowerGameGlobalController::KeyUp(GuiKeyType nCode) {
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnKey(nCode, true);
}

void TowerGameGlobalController::MouseMove(Point pPos) {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnMouse(pPos);
#endif
#endif
}

void TowerGameGlobalController::MouseDown(Point pPos) {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnMouseDown(pPos);
#endif
#endif
}

void TowerGameGlobalController::MouseUp() {
#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  pData->pCnt->vCnt[pData->pCnt->nActive]->OnMouseUp();
#endif
#endif
}

void TowerGameGlobalController::DoubleClick() {
  pData->pCnt->vCnt[pData->pCnt->nActive]->DoubleClick();
}

void TowerGameGlobalController::Fire() {
  pData->pCnt->vCnt[pData->pCnt->nActive]->Fire();
}
