#ifndef TOWER_DEFENSE_DRAGON_GAME_CONTROLLER_H
#define TOWER_DEFENSE_DRAGON_GAME_CONTROLLER_H

#include "game_utils/sound_utils.h"
#include "utils/file_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/geometry.h"
#include <memory>
#include <string>

struct DragonGameController;
struct GameController;
struct LevelLayout;
struct MenuController;
class Preloader;
struct ImageSequence;
struct SoundSequence;
struct TimedFireballBonus;
struct TowerDataWrap;
class FilePath;
class Index;
class ScalingDrawer;
class NumberDrawer;
class FontWriter;
template <typename T> struct GraphicalInterface;
template <typename T> struct SoundInterface;
struct SoundInterfaceProxy;

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

  std::vector<LevelLayout> vLvl;

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
                       const std::vector<LevelLayout> &vLvl_, Rectangle rBound_,
                       TowerDataWrap *pWrp_, FilePath *fp);

  TowerDataWrap *pWrp;

  DragonGameController *pSelf;

  void StartUp(DragonGameController *pSelf);
  void Next();
  void Restart(int nActive_ = -1);
  void Menu();

  Index &GetImg(std::string key);
  ImageSequence &GetImgSeq(std::string key);
  Index &GetSnd(std::string key);
  SoundSequence &GetSndSeq(std::string key);

private:
  std::unique_ptr<Preloader> pr;
};

#endif
