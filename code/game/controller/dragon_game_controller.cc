#include "game/controller/buy_now_controller.h"
#include "game/controller/dragon_game_controller.h"
#include "game/controller/level_controller.h"
#include "game/controller/menu_controller.h"
#include "game/dragon_constants.h"
#include "game/dragon_macros.h"
#include "game/dragon_game_runner.h"
#include "game/level.h"
#include "game_utils/Preloader.h"
#include "game_utils/image_sequence.h"
#include "game_utils/game_runner_interface.h"
#include "game_utils/sound_utils.h"
#include "game_utils/sound_sequence.h"
#include "utils/file_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/color.h"
#include "wrappers/geometry.h"
#include "wrappers/font_writer.h"

static void DrawStuff(Rectangle rBound,
                      GraphicalInterface<Index> *pGraph,
                      SoundInterface<Index> *pSnd, Preloader &pr,
                      int n) {
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

DragonGameController::DragonGameController(
    smart_pointer<ScalingDrawer> pDr_, smart_pointer<NumberDrawer> pNum_,
    smart_pointer<NumberDrawer> pBigNum_, FontWriter *pFancyNum_,
    SoundInterface<Index> *pSndRaw_,
    const std::vector<LevelLayout> &vLvl_, Rectangle rBound_,
    TowerDataWrap *pWrp_, FilePath *fp)
    : nActive(1), pDr(pDr_), pGraph(pDr_->pGr), pNum(pNum_), pBigNum(pBigNum_),
      pr(std::make_unique<Preloader>(pDr_->pGr, pSndRaw_, fp)),
      pSndRaw(pSndRaw_), pSnd(make_smart(new SoundInterfaceProxy(pSndRaw_))),
      nScore(0), vLvl(vLvl_), rBound(rBound_), bAngry(false), nHighScore(0),
      pFancyNum(pFancyNum_), pWrp(pWrp_), pMenu(), vLevelPointers(3),
      sbTutorialOn(fp, "tutorial_on.txt", true, true),
      snProgress(fp, "stuff.txt", 0, true),
      sbFullScreen(fp, sFullScreenPath, false, true),
      sbSoundOn(fp, "soundon.txt", true, true),
      sbMusicOn(fp, "musicon.txt", true, true),
      sbCheatsOn(fp, "cheat.txt", false, true),
      sbCheatsUnlocked(fp, "more_stuff.txt", false, true) {
  {
    if (fp->FileExists("high.txt")) {
      std::unique_ptr<InStreamHandler> ih = fp->ReadFile("high.txt");
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

  pr->AddTransparent(Color(0, 0, 0));
  pr->SetScale(nScale);

  pr->LoadTS("icons.bmp", "loading");
  pr->LoadTS("robotbear.bmp", "splash");
  pr->LoadSnd("beep.wav", "beep");
  pr->LoadSnd("boop.wav", "boop");
  DrawStuff(rBound, pGraph, pSndRaw_, *pr, 0);

  pr->LoadTS("road.bmp", "road");
  pr->LoadTS("turnandvorn.bmp", "logo");
#ifdef TRIAL_VERSION
  pr->LoadTS("trial.bmp", "trial");
  pr->LoadTS("buy_now.bmp", "buy");
#endif
  pr->LoadSeqTS("burn\\burn.txt", "burn");
  pr->LoadTS("empty.bmp", "empty");

  pr->LoadSeqTS("arrow\\sword.txt", "arrow");
  pr->LoadSeqTS("arrow\\claw.txt", "claw");

  DrawStuff(rBound, pGraph, pSndRaw_, *pr, 1);

  pr->LoadSeqTS("corona\\crosshair.txt", "corona"); // (not used icon)
  pr->LoadSeqTS("bonus\\void.txt", "void_bonus");
  pr->LoadSeqTS("bonus\\pershot.txt", "pershot_bonus"); // 1 - pershot
  pr->LoadSeqTS("bonus\\laser.txt", "laser_bonus");     // 2 - laser
  pr->LoadSeqTS("bonus\\big.txt", "big_bonus");         // 3 - big
  pr->LoadSeqTS("bonus\\totnum.txt", "totnum_bonus");   // 4 - totnum
  pr->LoadSeqTS("bonus\\explode.txt", "explode_bonus"); // 5 - explode
  pr->LoadSeqTS("bonus\\split.txt", "split_bonus");     // 6 - split
  pr->LoadSeqTS("bonus\\burning.txt", "burning_bonus"); // 7 - burning
  pr->LoadSeqTS("bonus\\ring.txt", "ring_bonus");       // 8 - ring
  pr->LoadSeqTS("bonus\\nuke.txt", "nuke_bonus");       // 9 - nuke
  pr->LoadSeqTS("bonus\\speed.txt", "speed_bonus");     // 10 - speed
  // pr.LoadSeqTS("bonus\\frequency.txt", "frequency");		// 11 -
  // frequency (not used)

  DrawStuff(rBound, pGraph, pSndRaw_, *pr, 2);
  pr->LoadSeqTS("start.txt", "start");
  pr->LoadSeqTS("win\\win.txt", "win");
  pr->LoadSeqTS("win\\over.txt", "over");
  pr->LoadSeqTS("logo\\pluanbo.txt", "pluanbo", Color(0, 0, 0), nScale * 2);
  pr->LoadSeqTS("logo\\gengui.txt", "gengui", Color(0, 0, 0), nScale * 2);
  pr->LoadSeqTS("castle\\castle.txt", "castle", Color(0, 0, 0));
  pr->LoadSeqTS("castle\\destroy_castle_dust.txt", "destroy_castle",
                Color(0, 0, 0));

  pr->LoadSeqTS("dragon_fly\\fly.txt", "dragon_fly");
  pr->LoadSeqTS("dragon\\stable.txt", "dragon_stable");
  pr->LoadSeqTS("dragon\\walk.txt", "dragon_walk");

  DrawStuff(rBound, pGraph, pSndRaw_, *pr, 3);

  pr->AddSequence((*pr)("dragon_fly"), "bdragon_fly");
  pr->AddSequence((*pr)("dragon_stable"), "bdragon_stable");
  pr->AddSequence((*pr)("dragon_walk"), "dragon_walk_f");

  ForEachImage((*pr)("bdragon_fly"), ImagePainter(pGraph, vColors));
  ForEachImage((*pr)("bdragon_stable"), ImagePainter(pGraph, vColors));
  ForEachImage((*pr)("dragon_walk_f"), ImageFlipper(pGraph));

  pr->LoadSeqTS("explosion\\explosion2.txt", "explosion");
  pr->LoadSeqTS("explosion\\laser_expl.txt", "laser_expl");

  pr->LoadSeqTS("explosion\\explosion_15.txt", "explosion_15");
  pr->LoadSeqTS("explosion\\laser_expl_15.txt", "laser_expl_15");

  DrawStuff(rBound, pGraph, pSndRaw_, *pr, 4);

  pr->LoadSeqTS("explosion\\explosion2.txt", "explosion_2", Color(0, 0, 0, 0),
                nScale * 2);
  pr->LoadSeqTS("explosion\\laser_expl.txt", "laser_expl_2", Color(0, 0, 0, 0),
                nScale * 2);

  pr->LoadSeqTS("explosion\\explosion_15.txt", "explosion_3", Color(0, 0, 0, 0),
                nScale * 2);
  pr->LoadSeqTS("explosion\\laser_expl_15.txt", "laser_expl_3",
                Color(0, 0, 0, 0), nScale * 2);

  pr->LoadSeqTS("fireball\\fireball.txt", "fireball");
  pr->LoadSeqTS("fireball\\fireball_15.txt", "fireball_15");
  pr->LoadSeqTS("fireball\\fireball.txt", "fireball_2", Color(0, 0, 0, 0),
                nScale * 2);
  pr->LoadSeqTS("fireball\\fireball_15.txt", "fireball_3", Color(0, 0, 0, 0),
                nScale * 2);

  DrawStuff(rBound, pGraph, pSndRaw_, *pr, 5);

  pr->LoadSeqTS("fireball\\laser.txt", "laser");
  pr->LoadSeqTS("fireball\\laser_15.txt", "laser_15");
  pr->LoadSeqTS("fireball\\laser.txt", "laser_2", Color(0, 0, 0, 0),
                nScale * 2);
  pr->LoadSeqTS("fireball\\laser_15.txt", "laser_3", Color(0, 0, 0, 0),
                nScale * 2);
  pr->LoadSeqTS("fireball\\fireball_icon.txt", "fireball_icon",
                Color(255, 255, 255));

  DrawStuff(rBound, pGraph, pSndRaw_, *pr, 6);

  pr->LoadSeqTS("knight\\knight.txt", "knight");
  pr->LoadSeqTS("knight\\die.txt", "knight_die");
  pr->LoadSeqTS("knight\\fire.txt", "knight_fire");
  pr->LoadSeqTS("ghostnight\\ghost_knight_burn.txt", "ghost_knight_burn");
  pr->LoadSeqTS("ghostnight\\ghost_knight.txt", "ghost_knight");
  pr->LoadSeqTS("ghostnight\\ghost_burn.txt", "ghost_burn");
  pr->LoadSeqTS("ghostnight\\ghost.txt", "ghost");

  pr->LoadSeqTS("golem\\golem.txt", "golem");
  pr->LoadSeqTS("golem\\golem_death.txt", "golem_die");

  pr->AddSequence((*pr)("golem"), "golem_f");
  ForEachImage((*pr)("golem_f"), ImageFlipper(pGraph));

  pr->AddSequence((*pr)("golem_die"), "golem_die_f");
  ForEachImage((*pr)("golem_die_f"), ImageFlipper(pGraph));

  pr->LoadSeqTS("skelly\\skelly.txt", "skelly");
  pr->LoadSeqTS("skelly\\die.txt", "skelly_die");
  pr->LoadSeqTS("skelly\\summon.txt", "skelly_summon");

  pr->LoadSeqTS("trader\\trader.txt", "trader");
  pr->LoadSeqTS("trader\\die.txt", "trader_die");

  DrawStuff(rBound, pGraph, pSndRaw_, *pr, 7);

  pr->AddSequence((*pr)("trader"), "trader_f");
  pr->AddSequence((*pr)("trader_die"), "trader_die_f");
  ForEachImage((*pr)("trader_f"), ImageFlipper(pGraph));
  ForEachImage((*pr)("trader_die_f"), ImageFlipper(pGraph));

  pr->LoadSeqTS("princess\\princess.txt", "princess");
  pr->LoadSeqTS("princess\\die.txt", "princess_die");

  pr->AddSequence((*pr)("princess"), "princess_f");
  pr->AddSequence((*pr)("princess_die"), "princess_die_f");
  ForEachImage((*pr)("princess_f"), ImageFlipper(pGraph));
  ForEachImage((*pr)("princess_die_f"), ImageFlipper(pGraph));

  pr->LoadSeqTS("mage\\mage.txt", "mage");
  pr->LoadSeqTS("mage\\spell.txt", "mage_spell");
  pr->LoadSeqTS("mage\\die.txt", "mage_die");

  DrawStuff(rBound, pGraph, pSndRaw_, *pr, 8);

  pr->AddSequence((*pr)("mage"), "mage_f");
  pr->AddSequence((*pr)("mage_spell"), "mage_spell_f");
  pr->AddSequence((*pr)("mage_die"), "mage_die_f");
  ForEachImage((*pr)("mage_f"), ImageFlipper(pGraph));
  ForEachImage((*pr)("mage_spell_f"), ImageFlipper(pGraph));
  ForEachImage((*pr)("mage_die_f"), ImageFlipper(pGraph));

  pr->LoadSeqTS("slime\\slime_walk.txt", "slime");
  pr->LoadSeqTS("slime\\slime_die.txt", "slime_die");
  pr->LoadSeqTS("slime\\slime_poke.txt", "slime_poke");
  pr->LoadSeqTS("slime\\slime_reproduce.txt", "slime_reproduce");
  pr->LoadSeqTS("slime\\slime_reproduce_fast.txt", "slime_reproduce_fast");
  pr->LoadSeqTS("slime\\slime_cloud.txt", "slime_cloud");

  pr->LoadSeqTS("slime\\mega_slime_walk.txt", "megaslime");
  pr->LoadSeqTS("slime\\mega_slime_die.txt", "megaslime_die");
  pr->LoadSeqTS("slime\\mega_slime_reproduce.txt", "megaslime_reproduce");

  pr->LoadSndSeq("sound\\over.txt", "over");
  pr->LoadSndSeq("sound\\pluanbo.txt", "pluanbo");
  pr->LoadSndSeq("sound\\click.txt", "click");

  DrawStuff(rBound, pGraph, pSndRaw_, *pr, 9);

  pr->LoadSnd("start_game.wav", "start_game");
  pr->LoadSnd("death01.wav", "death");
  pr->LoadSnd("golem_death2.wav", "golem_death");
  pr->LoadSnd("megaslime_spawn1.wav", "slime_spawn");
  pr->LoadSnd("megaslime_spawn2.wav", "megaslime_die");
  pr->LoadSnd("megaslime_hit.wav", "megaslime_hit");
  pr->LoadSnd("megaslime_bonus2.wav", "megaslime_bonus");
  pr->LoadSnd("megaslime_move4.wav", "megaslime_jump");
  pr->LoadSnd("megaslime_move3.wav", "megaslime_land");
  pr->LoadSnd("skeleton_bonus1.wav", "skeleton_bonus");

  pr->LoadSnd("explosion01.wav", "explosion");
  pr->LoadSnd("powerup03.wav", "powerup");
  pr->LoadSnd("laser04.wav", "laser");
  pr->LoadSnd("shoot2.wav", "shoot");
  pr->LoadSnd("dropping.wav", "knight_fall");
  pr->LoadSnd("knight_from_sky.wav", "dropping");
  pr->LoadSnd("pickup_sound_pxtone.wav", "pickup");
  pr->LoadSnd("princess_capture.wav", "princess_capture");
  pr->LoadSnd("win_level_2.wav", "win_level");
  pr->LoadSnd("all_princess_escape.wav", "all_princess_escape");
  pr->LoadSnd("destroy_castle_sound.wav", "destroy_castle_sound");
  pr->LoadSnd("one_princess.wav", "one_princess");
  pr->LoadSnd("leave_tower.wav", "leave_tower");
  pr->LoadSnd("return_tower.wav", "return_tower");
  pr->LoadSnd("step_left.wav", "step_left");
  pr->LoadSnd("step_right.wav", "step_right");
  pr->LoadSnd("hit_golem.wav", "hit_golem");
  pr->LoadSnd("slime_poke.wav", "slime_poke");
  pr->LoadSnd("slime_summon.wav", "slime_summon");
  pr->LoadSnd("princess_arrive.wav", "princess_arrive");

  pr->LoadSnd("sound/A.wav", "A");
  pr->LoadSnd("sound/B.wav", "B");
  pr->LoadSnd("sound/C.wav", "C");
  pr->LoadSnd("sound/D.wav", "D");
  pr->LoadSnd("sound/E.wav", "E");

  pr->LoadSnd("dddragon.wav", "background_music");
  pr->LoadSnd("_dddragon.wav", "background_music_slow");

#ifdef FULL_VERSION
  pr->LoadSnd("dddragon1.wav", "background_music2");
  pr->LoadSnd("_dddragon1.wav", "background_music_slow2");

  pr->LoadSnd("dddragon2.wav", "background_music3");
  pr->LoadSnd("_dddragon2.wav", "background_music_slow3");
#else // save memory on trial, though, *something* needs to be loaded otherwise
      // crashes.
  pr->LoadSnd("dddragon.wav", "background_music2");
  pr->LoadSnd("_dddragon.wav", "background_music_slow2");

  pr->LoadSnd("dddragon.wav", "background_music3");
  pr->LoadSnd("_dddragon.wav", "background_music_slow3");
#endif

  plr.pSnd = pSndRaw_;

  plr.vThemes.resize(6);
  plr.vThemes[BG_BACKGROUND] = pr->GetSnd("background_music");
  plr.vThemes[BG_SLOW_BACKGROUND] = pr->GetSnd("background_music_slow");
  plr.vThemes[BG_BACKGROUND2] = pr->GetSnd("background_music2");
  plr.vThemes[BG_SLOW_BACKGROUND2] = pr->GetSnd("background_music_slow2");
  plr.vThemes[BG_BACKGROUND3] = pr->GetSnd("background_music3");
  plr.vThemes[BG_SLOW_BACKGROUND3] = pr->GetSnd("background_music_slow3");

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

void DragonGameController::StartUp(DragonGameController *pSelf_) {
  pSelf = pSelf_;
  nScore = 0;
  bAngry = false;

  smart_pointer<Animation> pStr = make_smart(
      new Animation(0, (*pr)("start"), nFramesInSecond / 5,
                    Point(rBound.sz.x / 2, rBound.sz.y * 3 / 4), true));

  // menu
  smart_pointer<MenuController> pMenuHolder = make_smart(new MenuController(
      pSelf, rBound, Color(0, 0, 0), 3)); // resume position shouldn't matter
  pMenu = pMenuHolder;

  // logo 1
  smart_pointer<EntityListController> pCnt0_1 =
      make_smart(new AutoAdvanceController(pSelf, rBound, Color(0, 0, 0)));
  // logo 2
  smart_pointer<EntityListController> pCnt0_2 =
      make_smart(new AutoAdvanceController(pSelf, rBound, Color(0, 0, 0)));
  // press start screen
  smart_pointer<EntityListController> pCnt1 =
      make_smart(new StartScreenController(pSelf, rBound, Color(0, 0, 0)));
  // game over
  smart_pointer<EntityListController> pCnt2 =
      make_smart(new AutoAdvanceController(pSelf, rBound, Color(0, 0, 0)));
  // you win!
  smart_pointer<EntityListController> pCnt3 =
      make_smart(new EntityListController(pSelf, rBound, Color(0, 0, 0)));
  // score
  smart_pointer<DragonScoreController> pScore = make_smart(
      new DragonScoreController(pSelf, rBound, Color(0, 0, 0), true));
  // intro tutorial screen (non PC version)
  smart_pointer<DragonScoreController> pIntro = make_smart(
      new DragonScoreController(pSelf, rBound, Color(0, 0, 0), false));

  // cutscenes
  smart_pointer<EntityListController> pCut1 =
      make_smart(new Cutscene(pSelf, rBound, "princess", "knight"));
  smart_pointer<EntityListController> pCut2 =
      make_smart(new Cutscene(pSelf, rBound, "knight", "dragon_walk_f", true));
  smart_pointer<EntityListController> pCut3 =
      make_smart(new Cutscene(pSelf, rBound, "dragon_walk", "mage"));

  smart_pointer<SoundControls> pBckgMusic =
      make_smart(new SoundControls(plr, BG_BACKGROUND));
  smart_pointer<SoundControls> pNoMusic =
      make_smart(new SoundControls(plr, -1));

  smart_pointer<Animation> pWin = make_smart(new Animation(
      0, (*pr)("win"), 3, Point(rBound.sz.x / 2, rBound.sz.y / 2 - 20), true));
  smart_pointer<StaticImage> pL = make_smart(new StaticImage(
      (*pr)["logo"], Point(rBound.sz.x / 2, rBound.sz.y / 3), true));
  smart_pointer<Animation> pBurnL = make_smart(
      new Animation(0, (*pr)("burn"), 3,
                    Point(rBound.sz.x / 2 - 45, rBound.sz.y / 2 - 64), true));
  smart_pointer<Animation> pBurnR = make_smart(
      new Animation(0, (*pr)("burn"), 4,
                    Point(rBound.sz.x / 2 - 54, rBound.sz.y / 2 - 64), true));
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

  smart_pointer<TextDrawEntity> pHintText = make_smart(new TextDrawEntity(
      0, Point(rBound.sz.x / 2, rBound.sz.y * 7 / 8), true, sHint, pNum));
  smart_pointer<TextDrawEntity> pOptionText = make_smart(new TextDrawEntity(
      0, Point(rBound.sz.x / 2, rBound.sz.y * 7 / 8), true, "sup", pNum));

  smart_pointer<AnimationOnce> pO = make_smart(
      new AnimationOnce(0, (*pr)("over"), nFramesInSecond / 2,
                        Point(rBound.sz.x / 2, Crd(rBound.sz.y / 2.5f)), true));
  smart_pointer<AnimationOnce> pPlu = make_smart(
      new AnimationOnce(0, (*pr)("pluanbo"), nFramesInSecond / 10,
                        Point(rBound.sz.x / 2, rBound.sz.y / 2), true));
  smart_pointer<AnimationOnce> pGen = make_smart(
      new AnimationOnce(0, (*pr)("gengui"), nFramesInSecond / 5,
                        Point(rBound.sz.x / 2, rBound.sz.y / 2), true));

  smart_pointer<SimpleSoundEntity> pOver = make_smart(
      new SimpleSoundEntity(pr->GetSndSeq("over"), nFramesInSecond / 2, pSnd));
  smart_pointer<SimpleSoundEntity> pPluSnd = make_smart(new SimpleSoundEntity(
      pr->GetSndSeq("pluanbo"), nFramesInSecond / 10, pSnd));
  smart_pointer<SimpleSoundEntity> pClkSnd = make_smart(
      new SimpleSoundEntity(pr->GetSndSeq("click"), nFramesInSecond / 5, pSnd));

#ifdef TRIAL_VERSION
  smart_pointer<StaticImage> pTrial = make_smart(new StaticImage(
      (*pr)["trial"], Point(rBound.sz.x / 2 - 73, rBound.sz.y / 3 + 28), true));
  pCnt1->AddV(pTrial);
#endif

  smart_pointer<Animation> pMenuCaret =
      make_smart(new Animation(2, (*pr)("arrow"), 3, Point(0, 0), true));
  // menu entity
  smart_pointer<MenuDisplay> pMenuDisplay = make_smart(
      new MenuDisplay(Point(rBound.sz.x / 2 - 8, rBound.sz.y / 2), pNum,
                      pMenuCaret, pMenu, sbCheatsUnlocked.Get()));

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

  vCnt.push_back(pMenuHolder); // menu
  vCnt.push_back(pCnt0_1);     // logo 1
  vCnt.push_back(pCnt0_2);     // logo 2
  vCnt.push_back(pCnt1);       // press start screen
#ifndef PC_VERSION
  vCnt.push_back(pIntro); // tutorial screen
#endif

  for (unsigned i = 0; i < vLvl.size(); ++i) {
    smart_pointer<LevelController> pAd =
        make_smart(new LevelController(pSelf, rBound, Color(0, 0, 0), vLvl[i]));
    pAd->Init(pAd.get(), vLvl[i]);

    pAd->AddE(pBckgMusic);
    pAd->pSc = pBckgMusic;

    // game level
    vCnt.push_back(pAd);

    // chapters
    if (i == 0)
      vLevelPointers.at(0) = vCnt.size() - 1;
    else if (i == 3)
      vLevelPointers.at(1) = vCnt.size() - 1;
    else if (i == 6)
      vLevelPointers.at(2) = vCnt.size() - 1;

    // cutscene
    if (i == 2)
      vCnt.push_back(pCut1);
    if (i == 5)
      vCnt.push_back(pCut2);
    if (i == 8)
      vCnt.push_back(pCut3);
  }

#ifdef TRIAL_VERSION
  smart_pointer<BuyNowController> pBuy =
      make_smart(new BuyNowController(pSelf, rBound, Color(0, 0, 0)));

  smart_pointer<Animation> pGolem = make_smart(
      new Animation(0, (*pr)("golem_f"), nFramesInSecond / 10,
                    Point(rBound.sz.x / 4, rBound.sz.y * 3 / 4 - 10), true));
  smart_pointer<Animation> pSkeleton1 = make_smart(
      new Animation(0, (*pr)("skelly"), nFramesInSecond / 4,
                    Point(rBound.sz.x * 3 / 4, rBound.sz.y * 3 / 4 - 5), true));
  smart_pointer<Animation> pSkeleton2 = make_smart(new Animation(
      0, (*pr)("skelly"), nFramesInSecond / 4 + 1,
      Point(rBound.sz.x * 3 / 4 - 10, rBound.sz.y * 3 / 4 - 15), true));
  smart_pointer<Animation> pSkeleton3 = make_smart(new Animation(
      0, (*pr)("skelly"), nFramesInSecond / 4 - 1,
      Point(rBound.sz.x * 3 / 4 + 10, rBound.sz.y * 3 / 4 - 15), true));
  smart_pointer<Animation> pMage = make_smart(
      new Animation(0, (*pr)("mage_spell"), nFramesInSecond / 2,
                    Point(rBound.sz.x / 2, rBound.sz.y * 3 / 4), true));
  smart_pointer<Animation> pGhost = make_smart(new Animation(
      0, (*pr)("ghost"), nFramesInSecond / 6,
      Point(rBound.sz.x * 5 / 8, rBound.sz.y * 3 / 4 - 30), true));
  smart_pointer<Animation> pWhiteKnight = make_smart(new Animation(
      0, (*pr)("ghost_knight"), nFramesInSecond / 6,
      Point(rBound.sz.x * 3 / 8, rBound.sz.y * 3 / 4 - 30), true));

  // smart_pointer<StaticImage> pBuyNow = make_smart(new StaticImage(...));
  smart_pointer<StaticImage> pBuyNow = make_smart(new StaticImage(
      (*pr)["buy"], Point(rBound.sz.x / 2, rBound.sz.y / 3 + 33), true));
  smart_pointer<VisualEntity> pSlimeUpd =
      make_smart(new SlimeUpdater(pBuy.get()));

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
  vCnt.push_back(pCnt3);  // you win
  vCnt.push_back(pCnt2);  // game over
  vCnt.push_back(pScore); // score
#else
  vCnt.push_back(pCnt2);
  vCnt.push_back(pBuy);
#endif
}

void DragonGameController::Next() {
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

void DragonGameController::Restart(int nActive_ /* = -1*/) {
  if (nActive_ == -1)
    nActive = 3;
  else
    nActive = nActive_;

  vCnt.clear();

  StartUp(pSelf);
}

void DragonGameController::Menu() {
  pMenu->nResumePosition = nActive;
  nActive = 0;
}

Index &DragonGameController::GetImg(std::string key) { return (*pr)[key]; }

ImageSequence &DragonGameController::GetImgSeq(std::string key) {
  return (*pr)(key);
}

Index &DragonGameController::GetSnd(std::string key) { return pr->GetSnd(key); }

SoundSequence &DragonGameController::GetSndSeq(std::string key) {
  return pr->GetSndSeq(key);
}
