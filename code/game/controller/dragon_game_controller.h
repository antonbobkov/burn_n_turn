#ifndef TOWER_DEFENSE_DRAGON_GAME_CONTROLLER_H
#define TOWER_DEFENSE_DRAGON_GAME_CONTROLLER_H

struct DragonGameController;
struct TimedFireballBonus;

#include "game/controller/game_controller_interface.h"
#include "game/controller/menu_controller.h"
#include "game/level.h"
#include "game_utils/draw_utils.h"
#include "game_utils/sound_utils.h"
#include "utils/file_utils.h"
#include "wrappers/font_writer.h"
#include "wrappers/GuiGen.h"
#include "wrappers/geometry.h"
#include "utils/smart_pointer.h"
#include <memory>

struct TowerDataWrap;
class FilePath;

/** Global game state: level storage, active controller, graphics/sound, score,
 * savable options, music. */
struct DragonGameController {
  std::vector<smart_pointer<GameController>> vCnt;
  unsigned nActive;

  std::vector<int> vLevelPointers;

  smart_pointer<MenuController> pMenu;

  smart_pointer<GraphicalInterface<Index>> pGraph;
  smart_pointer<ScalingDrawer> pDr;
  smart_pointer<NumberDrawer> pNum;
  smart_pointer<NumberDrawer> pBigNum;
  smart_pointer<FontWriter> pFancyNum;

  smart_pointer<SoundInterface<Index>> pSndRaw;
  smart_pointer<SoundInterfaceProxy> pSnd;

  LevelStorage vLvl;

  Preloader pr;

  int nScore;
  int nHighScore;

  bool bAngry;

  SavableVariable<int> snProgress;
  SavableVariable<bool> sbSoundOn;
  SavableVariable<bool> sbMusicOn;
  SavableVariable<bool> sbTutorialOn;
  SavableVariable<bool> sbFullScreen;
  SavableVariable<bool> sbCheatsOn;
  SavableVariable<bool> sbCheatsUnlocked;

  Rectangle rBound;

  BackgroundMusicPlayer plr;

  std::list<smart_pointer<TimedFireballBonus>> lsBonusesToCarryOver;

  DragonGameController(smart_pointer<ScalingDrawer> pDr_,
                      smart_pointer<NumberDrawer> pNum_,
                      smart_pointer<NumberDrawer> pBigNum_,
                      smart_pointer<FontWriter> pFancyNum_,
                      smart_pointer<SoundInterface<Index>> pSndRaw_,
                      const LevelStorage &vLvl_, Rectangle rBound_,
                      TowerDataWrap *pWrp_, FilePath *fp);

  TowerDataWrap *pWrp;

  DragonGameController *pSelf;

  void StartUp(DragonGameController *pSelf);
  void Next();
  void Restart(int nActive_ = -1);
  void Menu();
};

#endif
