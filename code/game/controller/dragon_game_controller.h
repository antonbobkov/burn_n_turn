#ifndef TOWER_DEFENSE_DRAGON_GAME_CONTROLLER_H
#define TOWER_DEFENSE_DRAGON_GAME_CONTROLLER_H

#include "game/level.h"
#include "game_utils/sound_utils.h"
#include "utils/file_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/geometry.h"
#include <list>
#include <memory>
#include <string>
#include <vector>

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

/** Holds all savable options; MenuController gets a pointer to mutate/persist. */
struct DragonGameSettings {
  SavableVariable<int> snProgress;
  SavableVariable<bool> sbSoundOn;
  SavableVariable<bool> sbMusicOn;
  SavableVariable<bool> sbTutorialOn;
  SavableVariable<bool> sbFullScreen;
  SavableVariable<bool> sbCheatsOn;
  SavableVariable<bool> sbCheatsUnlocked;

  DragonGameSettings(FilePath *fp, const std::string &fullScreenPath);
};

/** Global game state: level storage, active controller, graphics/sound, score,
 * savable options, music. */
struct DragonGameController {
  DragonGameController(smart_pointer<ScalingDrawer> pDr_,
                       smart_pointer<NumberDrawer> pNum_,
                       smart_pointer<NumberDrawer> pBigNum_,
                       FontWriter *pFancyNum_,
                       SoundInterface<Index> *pSndRaw_,
                       const std::vector<LevelLayout> &vLvl_, Rectangle rBound_,
                       TowerDataWrap *pWrp_, FilePath *fp);

  void StartUp(DragonGameController *pSelf);
  void Next();
  void Restart(int nActive_ = -1);

  /** Controller currently in control (menu, level, start, etc.). */
  GameController *GetActiveController() const;
  /** Name of current screen for logging and tests (e.g. "menu", "level"). */
  std::string GetActiveControllerName() const;
  /** True when the game-over screen is showing. */
  bool IsOnGameOverScreen() const;
  /** Open main menu and remember where to return. */
  void EnterMenu();
  /** Leave menu and return to the screen we came from. */
  void ExitMenuResume();
  /** Restart from a given chapter (e.g. menu "chapter 1/2/3"). */
  void RestartFromChapter(int chapter);
  /** Switch to the game-over screen. */
  void ShowGameOverScreen();

  /** Main graphics interface (e.g. for drawing cursor). */
  GraphicalInterface<Index> *GetGraphics() const;
  /** Redraw the whole screen. */
  void RefreshAll();
  /** Drawer that scales sprites to screen size. */
  smart_pointer<ScalingDrawer> GetDrawer() const;
  /** Scale factor used for drawing (e.g. 1, 2). */
  unsigned GetDrawScaleFactor() const;

  /** Small number font for score and UI. */
  smart_pointer<NumberDrawer> GetNumberDrawer() const;
  /** Large number font for score display. */
  smart_pointer<NumberDrawer> GetBigNumberDrawer() const;
  /** Fancy font for intro and messages. */
  FontWriter *GetFancyFont() const;

  /** Play a sound by key (e.g. "death", "explosion"). */
  void PlaySound(std::string key);
  /** Turn sound on or off and persist the choice. */
  void ToggleSoundOutput();
  /** True if sound is currently on. */
  bool IsSoundOutputOn() const;
  /** Start or stop background music and persist the choice. */
  void ToggleMusicPlayback();
  /** True if music is currently off. */
  bool IsMusicPlaybackOff() const;

  /** Current score this run. */
  int GetScore() const;
  /** Best score so far (saved to file). */
  int GetHighScore() const;
  /** Add points to current score. */
  void AddScore(int delta);
  /** If score beats high score, update it and save to file. */
  void UpdateHighScoreIfNeeded();

  /** True when dragon is in angry mode. */
  bool IsAngry() const;
  /** Switch dragon to angry mode. */
  void SetAngry();

  /** Saved progress (which level/chapter we reached). */
  int GetProgress() const;
  /** Saved option: sound on or off. */
  bool IsSoundOnSetting() const;
  /** Saved option: music on or off. */
  bool IsMusicOnSetting() const;
  /** Saved option: show tutorial or not. */
  bool IsTutorialOnSetting() const;
  /** Saved option: fullscreen or windowed. */
  bool IsFullScreenSetting() const;
  /** Saved option: cheats enabled or not. */
  bool AreCheatsOnSetting() const;
  /** Saved option: cheats have been unlocked. */
  bool CheatsUnlocked() const;

  /** Play area rectangle in logical coordinates. */
  const Rectangle &GetBounds() const;
  /** Screen size in pixels. */
  Size GetActualResolution() const;
  /** Path helper for loading and saving files. */
  FilePath *GetFilePath() const;
  /** Quit the game (trigger exit event). */
  void ExitProgram();

  /** Remove all bonuses carried to the next level. */
  void ClearBonusesToCarryOver();
  /** Add a bonus to carry to the next level. */
  void AddBonusToCarryOver(smart_pointer<TimedFireballBonus> b);
  /** List of bonuses to carry to the next level (read-only). */
  const std::list<smart_pointer<TimedFireballBonus>> &GetBonusesToCarryOver()
      const;

  Index &GetImg(std::string key);
  ImageSequence &GetImgSeq(std::string key);
  Index &GetSnd(std::string key);
  SoundSequence &GetSndSeq(std::string key);

private:
  std::vector<smart_pointer<GameController>> vCnt;
  unsigned nActive;
  unsigned nResumePosition;

  std::vector<int> vLevelPointers;

  smart_pointer<MenuController> pMenu;

  GraphicalInterface<Index> *pGraph;
  smart_pointer<ScalingDrawer> pDr;
  smart_pointer<NumberDrawer> pNum;
  smart_pointer<NumberDrawer> pBigNum;
  FontWriter *pFancyNum;

  SoundInterface<Index> *pSndRaw;
  smart_pointer<SoundInterfaceProxy> pSnd;

  std::vector<LevelLayout> vLvl;

  int nScore;
  int nHighScore;

  bool bAngry;

  DragonGameSettings settings_;

  Rectangle rBound;

  BackgroundMusicPlayer plr;

  std::list<smart_pointer<TimedFireballBonus>> lsBonusesToCarryOver;

  TowerDataWrap *pWrp;

  DragonGameController *pSelf;

  std::unique_ptr<Preloader> pr;
};

#endif
