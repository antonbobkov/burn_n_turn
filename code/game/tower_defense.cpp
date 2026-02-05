#include "tower_defense.h"

MessageWriter *pWr = 0;

int nSlimeMax = 100;

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

void DrawStuff(Rectangle rBound, SP<Graphic> pGraph, SP<Soundic> pSnd,
               MyPreloader &pr, int n) {
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

	//p1.x *= 2;
	//p1.y *= 2;
	//p2.x *= 2;
	//p2.y *= 2;
	
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
    Rectangle rBound_, TowerDataWrap *pWrp_, FilePath fp)
    : nActive(1), pDr(this, pDr_), pGraph(this, pDr_->pGr), pNum(this, pNum_),
      pBigNum(this, pBigNum_), pr(pDr_->pGr, pSndRaw_, fp),
      pSndRaw(this, pSndRaw_), pSnd(this, new SoundInterfaceProxy(pSndRaw)),
      nScore(0), vLvl(vLvl_), rBound(rBound_), bAngry(false), nHighScore(0),
      pFancyNum(this, pFancyNum_), pWrp(pWrp_), pMenu(this, 0),
      vLevelPointers(3), sbTutorialOn("tutorial_on.txt", true, true),
      snProgress("stuff.txt", 0, true),
      sbFullScreen(sFullScreenPath, false, true),
      sbSoundOn("soundon.txt", true, true),
      sbMusicOn("musicon.txt", true, true),
      sbCheatsOn("cheat.txt", false, true),
      sbCheatsUnlocked("more_stuff.txt", false, true) {
  {
    std::ifstream ifs("high.txt");

    if (!ifs.fail())
      ifs >> nHighScore;

    ifs.close();
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

  // pr.LoadSeqTS("arrow\\arrow.txt", "arrow");
  // pr.LoadSeqTS("arrow\\newpointer.txt", "arrow");
  pr.LoadSeqTS("arrow\\sword.txt", "arrow");
  pr.LoadSeqTS("arrow\\claw.txt", "claw");

  // ForEachImage(pr("arrow"), ImageFlipper(pGraph));

  DrawStuff(rBound, pGraph, pSndRaw_, pr, 1);

  pr.LoadSeqTS("corona\\crosshair.txt", "corona"); // (not used icon)
  // pr.LoadSeqTS("bonus\\peter_bonus.txt", "bonus");
  pr.LoadSeqTS("bonus\\void.txt", "void_bonus"); // (not used icon)
  // pr.LoadSeqTS("bonus\\burning.txt", "burning_bonus");	// 0 - time (not
  // used)
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
  // pr.LoadSnd("megaslime_death.wav", "megaslime_die");
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
  // pr.LoadSnd("reload.wav","reload");
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

  /*
      pNum->CacheColor(Color(255, 255, 0, 205));
      pNum->CacheColor(Color(255, 255, 0, 155));
      pNum->CacheColor(Color(255, 255, 0, 105));
      pNum->CacheColor(Color(255, 255, 0, 55));
      pNum->CacheColor(Color(255, 255, 0, 5));
      */

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

  // SP<Countdown> pCnn = new Countdown(pNum, 15);

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

  /*
for(int i = 6; i <= 12; ++i)
  {
          SP<Animation> pSlm = new Animation(0, pr("slime"), nFramesInSecond/10,
Point(rBound.sz.x*i/18, rBound.sz.y/2), true); pBuy->AddBoth(pSlm);
  }*/

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

KnightOnFire::KnightOnFire(const Critter &cr, SP<BasicController> pBc_,
                           unsigned nTimer_, Chain c_)
    : Critter(cr), pBc(this, pBc_), nTimer(nTimer_), nTimer_i(nTimer_), c(c_),
      t(nFramesInSecond / 5) {
  Critter::seq = pBc->pGl->pr("knight_fire");
  RandomizeVelocity();
  // pBc->lsPpl.push_back(this);
}

void KnightOnFire::Update() {
  Critter::Update();

  for (std::list<ASSP<ConsumableEntity>>::iterator itr = pBc->lsPpl.begin();
       itr != pBc->lsPpl.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    if (this->HitDetection(*itr)) {
      char cType = (*itr)->GetType();

      if (cType == 'W' || cType == 'E')
        continue;

      if (c.IsLast() || cType != 'K')
        (*itr)->OnHit('F');
      else {
        (*itr)->bExist = false;
        SP<KnightOnFire> pKn = new KnightOnFire(
            Critter(GetRadius(), (*itr)->GetPosition(), fPoint(), rBound,
                    GetPriority(), ImageSequence(), true),
            pBc, nTimer_i, c.Evolve());
        pBc->AddBoth(pKn);
      }
    }
  }

  if (nTimer != 0 && --nTimer == 0) {
    bExist = false;

    SP<AnimationOnce> pAn = new AnimationOnce(
        dPriority, pBc->pGl->pr("knight_die"),
        unsigned(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true);
    pBc->AddBoth(pAn);
  }

  if (t.Tick() && float(rand()) / RAND_MAX < .25)
    RandomizeVelocity();
}

struct AdNumberDrawer : public VisualEntity {
  SSP<AdvancedController> pAd;

  AdNumberDrawer() : pAd(this, 0) {}

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    unsigned n = pDr->nFactor;

#ifdef FULL_VERSION
    pAd->pGl->pNum->DrawNumber(pAd->pGl->nScore,
                               Point(pAd->rBound.sz.x - 27 * 4, 4), 7);

    if (pAd->bBlink) {
      Color c(255, 255, 0);

      if (pAd->bTimerFlash)
        c = Color(255, 0, 0);

      pAd->pGl->pNum->DrawColorNumber(
          (pAd->t.nPeriod - pAd->t.nTimer) / nFramesInSecond,
          Point(pAd->rBound.sz.x - 14 * 4, 4), c, 4);
      pAd->pGl->pNum->DrawColorWord(
          "time:", Point(pAd->rBound.sz.x - 19 * 4, 4), c);
    }

    pAd->pGl->pNum->DrawNumber(pAd->nLvl, Point(pAd->rBound.sz.x - 3 * 4, 4),
                               2);

    pAd->pGl->pNum->DrawWord("score:", Point(pAd->rBound.sz.x - 33 * 4, 4));
    pAd->pGl->pNum->DrawWord("level:", Point(pAd->rBound.sz.x - 9 * 4, 4));
    if (pAd->bCh) {
      pAd->pGl->pNum->DrawColorWord(
          "invincible", Point(pAd->rBound.sz.x - 44 * 4, 4), Color(0, 255, 0));
    }
#else
    pAd->pGl->pNum->DrawNumber(pAd->pGl->nScore,
                               Point(pAd->rBound.sz.x - 17 * 4, 4), 7);
    pAd->pGl->pNum->DrawNumber(pAd->nLvl, Point(pAd->rBound.sz.x - 3 * 4, 4),
                               2);

    pAd->pGl->pNum->DrawWord("score:", Point(pAd->rBound.sz.x - 23 * 4, 4));
    pAd->pGl->pNum->DrawWord("level:", Point(pAd->rBound.sz.x - 9 * 4, 4));
#endif
  }
  /*virtual*/ Point GetPosition() { return Point(); }
  /*virtual*/ float GetPriority() { return 10; }
};

struct BonusDrawer : public VisualEntity {
  typedef std::list<ASSP<TimedFireballBonus>> BonusList;

  SSP<AdvancedController> pAd;

  Timer t;
  int nAnimationCounter;

  BonusDrawer()
      : pAd(this, 0), t(unsigned(nFramesInSecond * .1F)), nAnimationCounter(0) {
  }

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr) {
    if (t.Tick())
      ++nAnimationCounter;

    Point p(1, 3);

    for (unsigned nDr = 0; nDr < pAd->vDr.size(); ++nDr) {
      BonusList &lst = pAd->vDr[nDr]->lsBonuses;

      for (BonusList::iterator itr = lst.begin(), etr = lst.end(); itr != etr;
           ++itr) {

        SP<TimedFireballBonus> pBns = *itr;

        if (pBns->t.nPeriod &&
            (pBns->t.nPeriod - pBns->t.nTimer) < 4 * nFramesInSecond)
          if (nAnimationCounter % 2 == 1) {
            p.x += 10;
            continue;
          }

        ImageSequence img = GetBonusImage(pBns->nNum, pAd->pGl->pr);

        pDr->Draw(img.vImage[nAnimationCounter % img.vImage.size()], p, false);

        p.x += 10;
      }

      p.y += 10;

      p.x = 3;

      for (int i = 0; i < pAd->vDr[nDr]->nFireballCount; ++i) {
        pDr->Draw(pAd->pGl->pr("fireball_icon").GetImage(), p, false);

        p.x += 7;
      }

      p.y += 7;
      p.x = 0;
    }
  }
  /*virtual*/ Point GetPosition() { return Point(); }
  /*virtual*/ float GetPriority() { return 10; }
};

AdvancedController::AdvancedController(SP<TwrGlobalController> pGl_,
                                       Rectangle rBound, Color c,
                                       const LevelLayout &lvl)
    : BasicController(pGl_, rBound, c), bCh(false), nLvl(lvl.nLvl),
      nSlimeNum(0), bPaused(false), bFirstUpdate(true), bLeftDown(false),
      bRightDown(false), nLastDir(0), bWasDirectionalInput(0),
      bGhostTime(false), bBlink(true), pGr(0), bLeft(false), pSc(0),
      bTakeOffToggle(false), pTutorialText(this, 0),
      mc(pGl->pr("claw"), Point(), pGl.GetRawPointer()), bTimerFlash(false) {
  // imgCursor = pGl->pr("arrow");
  bNoRefresh = true;

  tLoseTimer.nPeriod = 0;

  SP<AdNumberDrawer> pNm = new AdNumberDrawer();
  pNm->pAd = this;
  AddV(pNm);

  SP<BonusDrawer> pBd = new BonusDrawer();
  pBd->pAd = this;
  AddV(pBd);

  SP<KnightGenerator> pGen =
      new KnightGenerator(lvl.vFreq.at(0), rBound, this, lvl.blKnightGen);
  SP<PrincessGenerator> pPGen =
      new PrincessGenerator(lvl.vFreq.at(1), rBound, this);
  SP<TraderGenerator> pTGen =
      new TraderGenerator(lvl.vFreq.at(2), rBound, this);
  SP<MageGenerator> pMGen =
      new MageGenerator(lvl.vFreq.at(3), lvl.vFreq.at(4), rBound, this);

  pGr = pGen.GetRawPointer();
  pMgGen = pMGen.GetRawPointer();

  unsigned i;
  for (i = 0; i < lvl.vRoadGen.size(); ++i)
    PushBackASSP(this, vRd, new FancyRoad(lvl.vRoadGen[i], this));

  for (i = 0; i < lvl.vCastleLoc.size(); ++i)
    PushBackASSP(this, vCs, new Castle(lvl.vCastleLoc[i], rBound, this));

  t = Timer(lvl.nTimer);

  PushBackASSP(
      this, vDr,
      new Dragon(vCs[0], this, pGl->pr("dragon_stable"), pGl->pr("dragon_fly"),
                 ButtonSet('q', 'w', 'e', 'd', 'c', 'x', 'z', 'a', ' ')));

  Point pos(pGl->rBound.sz.x / 2, pGl->rBound.sz.y);
  SP<TutorialTextEntity> pTT = new TutorialTextEntity(
      1, pos, pGl->pNum, pGl->sbTutorialOn.GetConstPointer());
  pTutorialText = pTT;

#ifdef PC_VERSION

  if (nLvl == 1) {
    tutOne.pTexter = pTT;
    tutOne.Update();
    AddBoth(pTT);
  }

  if (nLvl == 2) {
    tutTwo.pTexter = pTT;
    tutTwo.Update();
    AddBoth(pTT);
  }

#endif

  AddE(pGen);
  AddE(pPGen);
  AddE(pTGen);
  AddE(pMGen);

  for (i = 0; i < vCs.size(); ++i)
    AddBoth(vCs[i]);
  for (i = 0; i < vRd.size(); ++i)
    AddV(vRd[i]);
  for (i = 0; i < vDr.size(); ++i)
    AddBoth(vDr[i]);
}

const float fSpreadFactor = 2.0f;

/*virtual*/ void AdvancedController::OnKey(GuiKeyType c, bool bUp) {

#ifdef KEYBOARD_CONTROLS
  if (!bUp) {
    if (c == GUI_LEFT)
      bLeftDown = true;
    else if (c == GUI_RIGHT)
      bRightDown = true;
  } else {
    if (c == GUI_LEFT)
      bLeftDown = false;
    else if (c == GUI_RIGHT)
      bRightDown = false;
  }
#endif

  if (bUp)
    return;

  if (pGl->sbCheatsOn.Get()) {
    if (c == '\\') {
      pGl->Next();
      return;
    }

    if (c == 'i')
      bCh = !bCh;

    if (c == 'g')
      pGr->Generate(true);

    if (c == 'l')
      MegaGeneration();

    if (c == 'm')
      pMgGen->MageGenerate();

    if (c == '6')
      std::cout << "Slimes: " << nSlimeNum << "\n";

    if (c == 't')
      t.nTimer = t.nPeriod - 1;
    // t.nTimer = t.nPeriod-30*nFramesInSecond;

    if (c >= GUI_F1 && c <= GUI_F10)
      for (unsigned i = 0; i < vDr.size(); ++i)
        vDr[i]->AddBonus(vDr[i]->GetBonus(c - GUI_F1 + 1, nBonusCheatTime));
  }

  /*
  if(c == 'm')
          pGl->plr.ToggleOff();

  if(c == 'n')
          pGl->pSnd->Toggle();

  if(c == 'b')
          BoolToggle(pGl->sbTutorialOn);

  if(c == 'p')
  {
          bPaused = !bPaused;
          pGl->plr.ToggleOff();
  */

#ifdef PC_VERSION
  if (c == GUI_ESCAPE)
    pGl->Menu();
#endif

  for (unsigned i = 0; i < vDr.size(); ++i)
    if (vDr[i]->bt.IsSpace(c)) {
      // toggle if at base, otherwise shoot!
      if (!vDr[i]->bFly)
        vDr[i]->Toggle();
      else {
        fPoint fFb = vDr[0]->fVel;
        fFb.Normalize(100);
        vDr[0]->Fire(fFb);
      }
    }

    // this does directional shooting using the qwerty keys
    /*
        for(unsigned i = 0; i < vDr.size(); ++i)
    {
        Point p = vDr[i]->bt.GetPoint(c);

        if(p == Point())
            continue;

        fPoint fp(p);
        fp.Normalize();

        if(vDr[i]->bFly)
        {
            fp.x *= 3;
            fp.y *= 3;

            vDr[i]->fVel = fp;
        }
        else
        {
            fp.x += (float(rand())/RAND_MAX - .5F) / fSpreadFactor;
            fp.y += (float(rand())/RAND_MAX - .5F) / fSpreadFactor;

            vDr[i]->Fire(fp);
        }
    }
        */

#ifdef KEYBOARD_CONTROLS
  // directional shooting using arrow keys
  if (nLastDir == 0) {
    bool flag = true;
    if (c == GUI_LEFT)
      nLastDir = 1;
    else if (c == GUI_RIGHT)
      nLastDir = 2;
    else if (c == GUI_DOWN)
      nLastDir = 3;
    else if (c == GUI_UP)
      nLastDir = 4;
    else
      flag = false;
    if (flag)
      bWasDirectionalInput = true;
  } else {
    int dir = 0;
    if (c == GUI_LEFT)
      dir = 1;
    else if (c == GUI_RIGHT)
      dir = 2;
    else if (c == GUI_DOWN)
      dir = 3;
    else if (c == GUI_UP)
      dir = 4;
    fPoint fp = ComposeDirection(nLastDir, dir);
    fp.x += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;
    fp.y += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;

    if (!vDr[0]->bFly) {
      vDr[0]->Fire(fp);
      pt.UpdateLastDownPosition(Point(fp.x * 10000, fp.y * 10000));
    }
    bWasDirectionalInput = false;
  }
#endif
}

/*
void AdvancedController::SetDir(Point pPos, bool bInTower)
{
        pPos.x /= 2;
        pPos.y /= 2;

        fDir = pPos - vDr[0]->GetPosition();
        if(bInTower)
                fDir -= Point(-10, -25);
        fDir.Normalize();
}
*/

void AdvancedController::OnMouse(Point pPos) {
  Size sz1 = GetProgramInfo().szScreenRez;
  Size sz2 = pGl->pWrp->szActualRez;

  float fX = float(sz1.x) / sz2.x;
  float fY = float(sz1.y) / sz2.y;

  pPos.x *= Crd(fX);
  pPos.y *= Crd(fY);

  // tr.mtr.OnMouse(pPos);
  pt.UpdateMouse(pPos);
  mc.SetCursorPos(pPos);
}

void AdvancedController::OnMouseDown(Point pPos) {
  Size sz1 = GetProgramInfo().szScreenRez;
  Size sz2 = pGl->pWrp->szActualRez;

  float fX = float(sz1.x) / sz2.x;
  float fY = float(sz1.y) / sz2.y;

  pPos.x *= Crd(fX);
  pPos.y *= Crd(fY);

  pt.UpdateMouse(pPos);
  pt.UpdateLastDownPosition(pPos);
  pt.On();

  // if(pt.GetDirection(vDr[0]->GetPosition()).Length() < vDr[0]->GetRadius())

  if (bPaused)
    return;

  bool bHit = false;

  if (!vDr[0]->bFly)
    bHit = (pt.GetDirection(vDr[0]->pCs->GetPosition()).Length() <
            fTowerClickRadius);
  else
    bHit =
        (pt.GetDirection(vDr[0]->GetPosition()).Length() < fDragonClickRadius);

  if (bHit) {
    if (!vDr[0]->bFly)
      bTakeOffToggle = true;

    vDr[0]->Toggle();
  } else {
    if (!vDr[0]->bFly) {
      fPoint fFb = pt.GetDirection(vDr[0]->GetPosition() + Point(-10, -25));

      fFb.Normalize(100);
      vDr[0]->Fire(fFb);
    }
  }
}

void AdvancedController::OnMouseUp() {
  float fTime = float(pt.Off());
  fTime = fTime / nFramesInSecond;

  if (vDr[0]->bFly && fTime <= .2 && !bTakeOffToggle &&
      pt.GetDirection(vDr[0]->GetPosition()).Length() > vDr[0]->nRadius) {
    fPoint fFb = vDr[0]->fVel;

    fFb.Normalize(100);
    vDr[0]->Fire(fFb);
  }

  if (bTakeOffToggle)
    bTakeOffToggle = false;
}

void AdvancedController::Fire() {
  if (vDr[0]->bFly) {
    fPoint fFb = vDr[0]->fVel;

    fFb.Normalize(100);
    vDr[0]->Fire(fFb);
  } else {
    /* turning off multishoot on tower
    fPoint fFb = pt.GetDirection(vDr[0]->GetPosition() + Point(-10, -25));
    fFb.Normalize(100);
    vDr[0]->Fire(fFb);
    */
  }
}

float AdvancedController::GetCompletionRate() {
  float fCap = 0;
  for (unsigned i = 0; i < vCs.size(); ++i)
    fCap += vCs[i]->nPrincesses;

  fCap /= (4 * vCs.size());

  return fCap;
}

void AdvancedController::MegaGeneration() {
  Point p;
  p.x = GetRandNum(rBound.sz.x);
  p.y = GetRandNum(rBound.sz.y);
  MegaGeneration(p);
}

void AdvancedController::MegaGeneration(Point p) {
  SP<MegaSliminess> pSlm = new MegaSliminess(p, this);
  AddE(pSlm);
}

void MouseCursor::DrawCursor() {
  Index img;
  // if(vDr[0]->bFly)
  //{
  if (bPressed)
    img = imgCursor.vImage.at(1);
  else
    img = imgCursor.vImage.at(0);
  //}
  // else
  //	img = imgCursor.GetImage();

  Size sz = pGl->pGraph->GetImage(img)->GetSize();
  Point p = pCursorPos;

  p.x -= sz.x / 2;
  p.y -= sz.y / 2;

  pGl->pGraph->DrawImage(p, img, false);
}

void MouseCursor::SetCursorPos(Point pPos) { pCursorPos = pPos; }

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

/*virtual*/ void DragonScoreController::OnKey(GuiKeyType c, bool bUp) {
  if (!bUp)
    pGl->Next();
}

void DragonScoreController::DoubleClick() { pGl->Next(); }

/*virtual*/ void DragonScoreController::Update() {
  if (pGl->nHighScore < pGl->nScore) {
    pGl->nHighScore = pGl->nScore;

    std::ofstream ofs("high.txt");
    ofs << pGl->nScore;
    ofs.close();
  }

  BasicController::Update();

  // if(t.Tick())
  //	pGl->Next();
}

/*virtual*/ void HighScoreShower::Draw(SP<ScalingDrawer> pDr) {
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

/*virtual*/ void IntroTextShower::Draw(SP<ScalingDrawer> pDr) {
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

Castle::Castle(Point p, Rectangle rBound_, SP<AdvancedController> pAv_)
    : Critter(15, p, Point(), rBound_, 3, pAv_->pGl->pr("castle")),
      nPrincesses(0), pAv(this, pAv_), pDrag(this, 0), bBroken(false) {}

void Castle::OnKnight(char cWhat) {
  if (pAv->bCh)
    return;

  if (!nPrincesses || cWhat == 'W') {
    if (!bBroken) {
      pAv->pGl->pSnd->PlaySound(pAv->pGl->pr.GetSnd("destroy_castle_sound"));
      pAv->pSc->nTheme = -1;
      Critter::seq = pAv->pGl->pr("destroy_castle");
    }

    if (pAv->tLoseTimer.nPeriod == 0)
      pAv->tLoseTimer = Timer(nFramesInSecond * 3);

    bBroken = true;
    nPrincesses = 0;

    if (pDrag != 0) {

      pDrag->bFly = true;
      pDrag->bTookOff = true;

      pDrag->SimpleVisualEntity::seq = pDrag->imgFly;
      pDrag->SimpleVisualEntity::dPriority = 5;

      pDrag->pCs = 0;

      pDrag->fVel = pAv->pt.GetDirection(GetPosition());
      if (pDrag->fVel.Length() == 0)
        pDrag->fVel = fPoint(0, -1);
      pDrag->fVel.Normalize(pDrag->leash.speed);

      // pDrag->bExist = false;

      pDrag = 0;
    }

    return;
  }

  if (pDrag != 0) {
    pAv->pGl->pSnd->PlaySound(pAv->pGl->pr.GetSnd("one_princess"));

    --nPrincesses;

    if (cWhat == 'K') {
      fPoint v = RandomAngle();
      v.Normalize(fPrincessSpeed * 3.F);

      SP<Princess> pCr =
          new Princess(Critter(7, GetPosition(), v, rBound, 0,
                               v.x < 0 ? pAv->pGl->pr("princess_f")
                                       : pAv->pGl->pr("princess"),
                               true),
                       pAv);
      pAv->AddBoth(pCr);
      PushBackASSP(pAv.GetRawPointer(), pAv->lsPpl, pCr);
    }
  } else {
    pAv->pGl->pSnd->PlaySound(pAv->pGl->pr.GetSnd("all_princess_escape"));

    if (cWhat == 'K') {
      float r = float(rand()) / RAND_MAX * 2 * 3.1415F;

      for (unsigned i = 0; i < nPrincesses; ++i) {
        fPoint v(sin(r + i * 2 * 3.1415F / nPrincesses),
                 cos(r + i * 2 * 3.1415F / nPrincesses));
        v.Normalize(fPrincessSpeed * 3.F);

        SP<Princess> pCr =
            new Princess(Critter(7, GetPosition(), v, rBound, 0,
                                 v.x < 0 ? pAv->pGl->pr("princess_f")
                                         : pAv->pGl->pr("princess"),
                                 true),
                         pAv);
        pAv->AddBoth(pCr);
        PushBackASSP(pAv.GetRawPointer(), pAv->lsPpl, pCr);
      }
    }

    nPrincesses = 0;
  }
}

/*unsigned*/ void Castle::Draw(SP<ScalingDrawer> pDr) {
  Critter::seq.nActive = nPrincesses;

  if (nPrincesses > 4)
    Critter::seq.nActive = 4;

  if (bBroken) {
    Critter::seq.nActive = pAv->tLoseTimer.nTimer / 2;
    if (seq.nActive > seq.vImage.size() - 1)
      seq.nActive = seq.vImage.size() - 1;
  }

  Critter::Draw(pDr);

  // pAv->pGl->pNum->DrawNumber(nPrincesses, GetPosition() + Point(0, 18));
}

/*virtual*/ void AdvancedController::Update() {
  CleanUp(lsSlimes);
  CleanUp(lsSliminess);

  pt.Update();

  if (bFirstUpdate) {
    bFirstUpdate = false;

    if (nLvl != 1 && nLvl != 4 && nLvl != 7 && nLvl != 10) {
      vDr[0]->RecoverBonuses();
    }
    pGl->lsBonusesToCarryOver.clear();
  }

  if (bPaused) {
    int nScale = 2;
    int nCharWidth = 4;

    Point p1 = Point(rBound.sz.x / 2 / nScale, rBound.sz.y / 2 / nScale);

    std::string s1 = "paused";

    nCharWidth *= nScale;

    Rectangle r(Point(rBound.sz.x / 2 - (s1.size() * nCharWidth) / 2 - 10,
                      rBound.sz.y / 2 - 10),
                Size(s1.size() * nCharWidth + 18, 21));
    // Rectangle r(Point(rBound.sz.x/2 - 10, rBound.sz.y/2 - 10), Size(20, 20));

    r.p.x *= nScale;
    r.p.y *= nScale;
    r.sz.x *= nScale;
    r.sz.y *= nScale;

    pGl->pGraph->DrawRectangle(r, Color(0, 0, 0), false);
    pGl->pBigNum->DrawWord(s1, p1, true);
    pGl->pGraph->RefreshAll();

    return;
  }

  if (tLoseTimer.nPeriod == 0) {
    if (!bGhostTime) {
      if (nLvl <= 3)
        pSc->nTheme = BG_BACKGROUND;
      else if (nLvl <= 6)
        pSc->nTheme = BG_BACKGROUND2;
      else
        pSc->nTheme = BG_BACKGROUND3;
    } else {
      if (nLvl <= 3)
        pSc->nTheme = BG_SLOW_BACKGROUND;
      else if (nLvl <= 6)
        pSc->nTheme = BG_SLOW_BACKGROUND2;
      else
        pSc->nTheme = BG_SLOW_BACKGROUND3;
    }
  }

  BasicController::Update();

#ifdef PC_VERSION
#ifndef KEYBOARD_CONTROLS
  mc.bPressed = pt.bPressed;
  mc.DrawCursor();
#endif
#endif
  pGl->pGraph->RefreshAll();

  if (tLoseTimer.nPeriod != 0 && tLoseTimer.Tick()) {
    pGl->nActive = pGl->vCnt.size() - 2;
    return;
  }

  tr.Update();
  if (!vDr[0]->bFly) {
    if (tr.IsTrigger()) {
      fPoint p = tr.GetMovement();

      // std::cout << p.x << " " << p.y << " " << p.Length() << "\n";

      if (p.Length() > 50) {
        if (p.Length() > 250)
          p.Normalize(250);

        vDr[0]->Fire(p);
      }

      // p.Normalize(5);

      // SP<Fireball> pFr = new Fireball(pDr->GetPosition(), p, rBound, this);
      // AddBoth(pFr);
    }

    /*
    if(pt.bPressed)
    {
            if(tShootTimer.Tick())
            {
                    fPoint fFb = pt.GetDirection(vDr[0]->GetPosition() +
    Point(-10, -25)); fFb.Normalize(100); vDr[0]->Fire(fFb);
            }
    }
    else
    {
            if(tShootTimer.Tick())
                    tShootTimer.nTimer = tShootTimer.nPeriod - 1;
    }
    */
  } else {
    // fPoint p = tr.GetAvMovement();
    // if(p.Length() <= 25)
    //    return;

    // p.Normalize(3);

    // if(p != fPoint())
    //    vDr[0]->fVel = p;

    // vDr[0]->fVel = vDr[0]->leash.GetNewVelocity(tr.GetMovement());

    if (pt.bPressed) {
      fPoint v = vDr[0]->fVel;
      fPoint d = pt.GetDirection(vDr[0]->GetPosition());

      if (d.Length() == 0)
        d = v;

      // d.Normalize();
      d.Normalize(v.Length());

      // fPoint fDelta = d*Dot(v, d) - v;
      // fDelta /= 3;

      // if(Dot(v, d) < 0)
      //	fDelta *= -1;

      // vDr[0]->fVel += fDelta;

      vDr[0]->fVel = v * fFlightCoefficient + d;
      vDr[0]->fVel.Normalize(vDr[0]->leash.speed);
    } else if (bLeftDown || bRightDown) {
      fPoint v = vDr[0]->fVel;
      fPoint d(v.y, v.x);
      // left precedents right
      if (bLeftDown)
        d.y *= -1;
      else
        d.x *= -1;
      vDr[0]->fVel = v * fFlightCoefficient * 1.2f + d;
      vDr[0]->fVel.Normalize(vDr[0]->leash.speed);
    }
  }

#ifdef FULL_VERSION
  if (!bGhostTime) {
    if (t.Tick()) {
      // pSc->plr.StopMusic();
      bGhostTime = true;

      if (!pGl->sbMusicOn.Get())
        pGl->pSnd->PlaySound(pGl->pr.GetSnd("E"));

      if (nLvl > 6)
        pGr->Generate(true);
    }
  }

  if (!bTimerFlash) {
    if (t.nPeriod - t.nTimer < 20 * nFramesInSecond) {
      bTimerFlash = true;
      tBlink = Timer(nFramesInSecond / 2);
    }
  } else {
    if (tBlink.Tick()) {
      if (!pGl->sbMusicOn.Get() && !bGhostTime && !bBlink)
        pGl->pSnd->PlaySound(pGl->pr.GetSnd("D"));

      bBlink = !bBlink;
    }
  }

#endif

#ifdef KEYBOARD_CONTROLS
  if (!bWasDirectionalInput)
    nLastDir = 0;
  else {
    fPoint fp = ComposeDirection(nLastDir, nLastDir);
    fp.x += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;
    fp.y += (float(rand()) / RAND_MAX - .5F) / fSpreadFactor;

    if (!vDr[0]->bFly) {
      vDr[0]->Fire(fp);
      pt.UpdateLastDownPosition(Point(fp.x * 10000, fp.y * 10000));
    }
  }

  bWasDirectionalInput = false;
#endif
}

/*virtual*/ void Slime::Update() {
  if (t.Tick() && float(rand()) / RAND_MAX < .25)
    RandomizeVelocity();

  for (std::list<ASSP<ConsumableEntity>>::iterator itr = pAc->lsPpl.begin();
       itr != pAc->lsPpl.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    if (this->HitDetection(*itr)) {
      if ((*itr)->GetType() == 'K') {
        pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("slime_poke"));

        // if(bExist)
        //    --pAc->nSlimeNum;
        bExist = false;

        SP<AnimationOnce> pAn =
            new AnimationOnce(dPriority, pAc->pGl->pr("slime_poke"),
                              nFramesInSecond / 5, GetPosition(), true);
        pAc->AddBoth(pAn);

        break;
      }
    }
  }

  Critter::Update(); // can walk off the screen
}

/*virtual*/ void Slime::OnHit(char cWhat) {
  if (pAc->nSlimeNum >= nSlimeMax && cWhat != 'M') {
    std::vector<Point> vDeadSlimes;

    for (std::list<ASSP<Slime>>::iterator itr = pAc->lsSlimes.begin();
         itr != pAc->lsSlimes.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      vDeadSlimes.push_back((*itr)->GetPosition());
      (*itr)->OnHit('M');
    }

    for (std::list<ASSP<Sliminess>>::iterator itr = pAc->lsSliminess.begin();
         itr != pAc->lsSliminess.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      vDeadSlimes.push_back((*itr)->GetPosition());
      (*itr)->Kill();
    }

    if (vDeadSlimes.empty())
      throw SimpleException("No slimes found!");

    fPoint fAvg(0, 0);
    for (unsigned i = 0; i < vDeadSlimes.size(); ++i) {
      fAvg += vDeadSlimes[i];
    }

    fAvg /= float(vDeadSlimes.size());

    pAc->MegaGeneration(fAvg.ToPnt());

    for (unsigned i = 0; i < vDeadSlimes.size(); ++i) {
      SP<FloatingSlime> pSlm =
          new FloatingSlime(pAc->pGl->pr("slime_cloud"), vDeadSlimes[i],
                            fAvg.ToPnt(), nFramesInSecond * 1);
      pAc->AddBoth(pSlm);
    }

    return;
  }

  bExist = false;

  bool bRevive =
      (cWhat != 'M'); // pAc->nSlimeNum < nSlimeMax;// && nGeneration != 0;

  if (cWhat != 'M') {
    SP<BonusScore> pB = new BonusScore(pAc, GetPosition(), 1);
    pAc->AddBoth(pB);
  }

  SP<AnimationOnce> pAn = new AnimationOnce(
      dPriority, pAc->pGl->pr(bRevive ? "slime_die" : "slime_poke"),
      nFramesInSecond / 5, GetPosition(), true);
  pAc->AddBoth(pAn);

  if (!bRevive)
    return;

  for (int i = 0; i < 2; ++i) {
    fPoint f = RandomAngle();
    f.Normalize(4);

    SP<Sliminess> pSlm =
        new Sliminess(GetPosition() + f.ToPnt(), pAc, false, nGeneration + 1);
    pAc->AddE(pSlm);
    PushBackASSP(pAc.GetRawPointer(), pAc->lsSliminess, pSlm);
  }
}

/*virtual*/ void BonusScore::Draw(SP<ScalingDrawer> pDr) {
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

bool AreWeFullScreen() {
  std::ifstream ifs(sFullScreenPath);

  bool bRet;
  ifs >> bRet;

  if (ifs.fail())
    return false;

  return bRet;
}

// std::ofstream ofs_move ("moves.txt");
// std::ofstream ofs_angl ("angles.txt");

ProgramInfo GetProgramInfo() {
  ProgramInfo inf;

  // inf.szScreenRez = Size(640, 480);
  // inf.szScreenRez = Size(1024, 768);
  // inf.szScreenRez = Size(960, 640);

#ifdef SMALL_SCREEN_VERSION
  inf.szScreenRez = Size(854, 480);
#else
  inf.szScreenRez = Size(960, 600);
#endif

  inf.strTitle = "Tower Defense";
  inf.nFramerate = 1000 / nFramesInSecond;
  inf.bMouseCapture = false;

  static bool bFullScreen = AreWeFullScreen();

  if (bFullScreen) {
    inf.bFullScreen = true;
    inf.bFlexibleResolution = true;
    inf.bBlackBox = true;
  }

  return inf;
}

SP<GlobalController> GetGlobalController(ProgramEngine pe) {
  return new TowerGameGlobalController(pe);
}

TowerGameGlobalController::TowerGameGlobalController(ProgramEngine pe) {
  pData = new TowerDataWrap(pe);
}

TowerGameGlobalController::~TowerGameGlobalController() {
  SP_Info *pCleanUp = pData->pCnt.GetRawPointer();
  delete pData;
  CleanIslandSeeded(pCleanUp);
}

void ReadLevels(std::string sFile, Rectangle rBound, LevelStorage &vLvl) {
  std::istringstream istr;

  std::ifstream ifs(sFile.c_str());

  LevelLayout l(rBound);
  while (ifs >> l) {
    l.Convert();
    vLvl.push_back(l);
  }

  if (vLvl.size() == 0)
    throw SimpleException("<global>", "ReadLevels",
                          "Cannot read levels at " + sFile);
}

TowerDataWrap::TowerDataWrap(ProgramEngine pe) {
  szActualRez = pe.szActualRez;

  pExitProgram = pe.pExitProgram;

  pWr = pe.pMsg.GetRawPointer();

  {
    std::ifstream ifs("config.txt");
    if (ifs.fail())
      throw SimpleException("TowerDataWrap", "<constructor>",
                            "Need config.txt file!");
    ifs >> fp;
  }

  Rectangle sBound = Rectangle(pe.szScreenRez);
  unsigned nScale = 2;
  Rectangle rBound =
      Rectangle(0, 0, sBound.sz.x / nScale, sBound.sz.y / nScale);

  pGr = pe.pGr;
  pSm = pe.pSndMng;

  pDr = new ScalingDrawer(pGr, nScale);

  SP<ScalingDrawer> pBigDr = new ScalingDrawer(pGr, nScale * 2);

  std::string sPath = "dragonfont\\";
  fp.Parse(sPath);

  pNum = new NumberDrawer(pDr, sPath, "dragonfont");
  pBigNum = new NumberDrawer(pBigDr, sPath, "dragonfont");
  pFancyNum = new FontWriter(fp, "dragonfont\\dragonfont2.txt", pGr, 2);

#ifdef FULL_VERSION
#ifdef SMALL_SCREEN_VERSION
  ReadLevels(fp.sPath + "levels_small.txt", rBound, vLvl);
#else
  ReadLevels(fp.sPath + "levels.txt", rBound, vLvl);
#endif
#else
  ReadLevels(fp.sPath + "levels_trial.txt", rBound, vLvl);
#endif

  pCnt = new TwrGlobalController(pDr, pNum, pBigNum, pFancyNum, pSm, vLvl,
                                 rBound, this, fp);
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

/*virutal*/ void SkellyGenerator::Update() {
  if (t.Tick()) {
    bExist = false;

    // SP<Slime> pSlm = new Slime(p, pAdv->rBound, pAdv);
    // pAdv->AddBoth(pSlm);
    // PushBackASSP(pAdv.GetRawPointer(), pAdv->lsPpl, pSlm);

    unsigned n = unsigned(rand() % pAdv->vCs.size());

    fPoint v = pAdv->vCs[n]->GetPosition() - p;
    v.Normalize(fSkeletonSpeed);

    SP<Knight> pCr = new Knight(
        Critter(7, p, v, pAdv->rBound, 3, pAdv->pGl->pr("skelly"), true), pAdv,
        'S');
    pAdv->AddBoth(pCr);
    PushBackASSP(pAdv.GetRawPointer(), pAdv->lsPpl, pCr);
  }
}

/*virtual*/ void Mage::Update() {
  if (bAngry) {
    if (!bCasting) {
      unsigned i = 0;
      for (; i < pAc->vCs.size(); ++i) {
        fPoint p = pAc->vCs[i]->GetPosition() - fPos;
        if (p.Length() < nSummonRadius)
          break;
      }
      if (i == pAc->vCs.size())
        if (rand() % nSummonChance == 0) {
          bCasting = true;
          Critter::seq = pAc->pGl->pr("mage_spell");
          Critter::fVel = fPoint(0, 0);

          // tUntilSpell = Timer(GetTimeUntillSpell());
        }
    } else {
      if (tSpellAnimate.Tick()) {
        Critter::seq.Toggle();
      }

      if (tSpell.UntilTick() == int(1.F * nFramesInSecond)) {
        // SummonSlimes();
        SummonSkeletons(pAc, GetPosition());
      }

      if (tSpell.Tick()) {
        bCasting = false;
        Critter::fVel = fMvVel;
        Critter::seq =
            fMvVel.x < 0 ? pAc->pGl->pr("mage_f") : pAc->pGl->pr("mage");
      }
    }
  }

  Critter::Update();
}

void Mage::SummonSlimes() {
  for (int i = 0; i < 2; ++i) {
    fPoint f = RandomAngle();
    f.Normalize(10);

    // pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("slime_summon"));

    SP<Sliminess> pSlm = new Sliminess(GetPosition() + f.ToPnt(), pAc, true, 0);
    pAc->AddE(pSlm);
  }
}

/*virutal*/ void Ghostiness::Update() {
  if (t.Tick()) {
    bExist = false;

    if (nGhostHit == 0)
      return;

    SP<Knight> pCr = new Knight(knCp, pAdv, 'G');
    if (nGhostHit == 1)
      pCr->seq = pAdv->pGl->pr("ghost");
    else
      pCr->seq = pAdv->pGl->pr("ghost_knight");
    pCr->nGhostHit = nGhostHit - 1;

    pAdv->AddBoth(pCr);
    PushBackASSP(pAdv.GetRawPointer(), pAdv->lsPpl, pCr);
  }
}

/*virtual*/ void Knight::OnHit(char cWhat) {
  if (cType == 'W') {
    KnockBack();
    if (nGolemHealth > 0) {
      --nGolemHealth;
      pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("hit_golem"));
      return;
    }

    pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("golem_death"));

    // pAc->pGr->Generate(true);
    SP<BonusScore> pB = new BonusScore(pAc, GetPosition(), 5000);
  }

  bExist = false;

  pAc->tutOne.KnightKilled();

  if (cType != 'G') {
    SP<BonusScore> pB = new BonusScore(pAc, GetPosition(), 100);
    pAc->AddBoth(pB);

    ImageSequence seqDead = pAc->pGl->pr("knight_die");

    if (cType == 'S')
      seqDead = pAc->pGl->pr("skelly_die");
    else if (cType == 'W') {
      if (this->fVel.x < 0)
        seqDead = pAc->pGl->pr("golem_die");
      else
        seqDead = pAc->pGl->pr("golem_die_f");
    }

    SP<AnimationOnce> pAn = new AnimationOnce(
        dPriority, seqDead, unsigned(nFramesInSecond / 5 / fDeathMultiplier),
        GetPosition(), true);
    pAc->AddBoth(pAn);
  } else {
    SP<Ghostiness> pGhs = new Ghostiness(GetPosition(), pAc, *this, nGhostHit);
    pAc->AddE(pGhs);
  }
}

/*virtual*/ void MegaSlime::OnHit(char cWhat) {
  // KnockBack();
  if (nHealth > 0) {
    --nHealth;
    pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_hit"));
    return;
  }

  bExist = false;

  SP<BonusScore> pB = new BonusScore(pAc, GetPosition(), 500);
  pAc->AddBoth(pB);

  ImageSequence seqDead = pAc->pGl->pr("megaslime_die");
  pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_die"));

  SP<AnimationOnce> pAn = new AnimationOnce(
      dPriority, seqDead, unsigned(nFramesInSecond / 5 / fDeathMultiplier),
      GetPosition(), true);
  pAc->AddBoth(pAn);
}

void SlimeUpdater::Draw(SP<ScalingDrawer> pDr) { pBuy->DrawSlimes(); }

void MenuController::OnKey(GuiKeyType c, bool bUp) {
  if (bUp)
    return;
  if (c == GUI_ESCAPE) {
    pMenuDisplay->Escape();
    return;
  }
  /*
  else if(c == '1')
  {
          pGl->Restart();
  }
  else if(c == '2')
  {
          pGl->pSnd->Toggle();
          std::cout << "Sound toggled\n";
  }
  else if(c == '3')
  {
          pGl->plr.ToggleOff();
          std::cout << "Music toggled\n";
  }
  else if(c == '4')
  {
          BoolToggle(pGl->sbTutorialOn);
          std::cout << "Tutorial toggled\n";
  }
  */
  else if (c == GUI_UP) {
    pMenuDisplay->PositionIncrement(false);
  } else if (c == GUI_DOWN) {
    pMenuDisplay->PositionIncrement(true);
  } else if (c == GUI_RETURN || c == ' ') {
    pMenuDisplay->Boop();
  }
}

void MenuController::OnMouse(Point pPos) {
  // mc.SetCursorPos(pPos);
  // pMenuDisplay->OnMouseMove(pPos);
}

void MenuController::OnMouseDown(Point pPos) {
  // pMenuDisplay->Boop();
}

void MenuController::Update() {
  BasicController::Update();

  if (pMenuDisplay->pCurr == &(pMenuDisplay->memOptions)) {
    if (pOptionText != 0)
      pOptionText->Draw(pGl->pDr);
  } else {
    if (pHintText != 0)
      pHintText->Draw(pGl->pDr);
  }

  // mc.DrawCursor();
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

  // vOptionText.at(nMusic) = "background music";
  // vOptionText.at(nSound) = "sound effects";
  // vOptionText.at(nTutorial) = "tutorial hints in fist two levels";
  // vOptionText.at(nFullScreen) = "full screen mode\n(relaunch for changes to
  // take place)";
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
      FullTextString() + OnOffString(bFullScreenSetting); // + sExtra;

  vOptionText.at(nFullScreen) = sExtra;

  bool bCheatsOn = pMenuController->pGl->sbCheatsOn.Get();
  // std::string sCheatExtra = "off";
  // if(bCheatsOn)
  //	sCheatExtra = "bonus f1-f10 skip \\ invincible i";

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
