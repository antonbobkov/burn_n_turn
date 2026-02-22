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

/** The realm's scroll of choices—sound, music, tutorial, fullscreen, cheats.
 * The menu keeper reads and writes these; they are saved between sessions.
 */
struct DragonGameSettings {
  SavableVariable<int> snProgress;
  SavableVariable<bool> sbSoundOn;
  SavableVariable<bool> sbMusicOn;
  SavableVariable<bool> sbTutorialOn;
  SavableVariable<bool> sbCheatsOn;
  SavableVariable<bool> sbCheatsUnlocked;
  SavableVariable<int> snHighScore;

  DragonGameSettings(ConfigurationFile *cfg);
};

/** The great hall of the realm: where levels dwell, who holds the throne,
 * sights and sounds, the hero's tally, scrolls of choice, and the bard's song.
 */
struct DragonGameController {
  DragonGameController(ScalingDrawer *pDr_, NumberDrawer *pNum_,
                       NumberDrawer *pBigNum_, FontWriter *pFancyNum_,
                       SoundInterface<Index> *pSndRaw_,
                       const std::vector<LevelLayout> &vLvl_, Rectangle rBound_,
                       TowerDataWrap *pWrp_, FilePath *fp,
                       ConfigurationFile *config,
                       ConfigurationFile *game_data);

  void StartUp(DragonGameController *pSelf);
  void Next();
  void Restart(int nActive_ = -1);

  /** Who holds the throne right now—the menu, a level, or the gates of dawn. */
  GameController *GetActiveController() const;
  /** Name of the screen before you, for scribes and trials (e.g. menu, level). */
  std::string GetActiveControllerName() const;
  /** True when the dragon has claimed the day—the game-over screen is upon you. */
  bool IsOnGameOverScreen() const;
  /** Step into the great menu hall; the path back is remembered. */
  void EnterMenu();
  /** Leave the menu and return to where the quest had paused. */
  void ExitMenuResume();
  /** Begin the tale anew from a chosen chapter (e.g. chapter 1, 2, or 3). */
  void RestartFromChapter(int chapter);
  /** Show the screen where the dragon wins—game over. */
  void ShowGameOverScreen();

  /** The realm's canvas—where the cursor and all sights are drawn. */
  GraphicalInterface<Index> *GetGraphics() const;
  /** Refresh the entire vista; paint the screen anew. */
  void RefreshAll();
  /** The drawer that fits sprites to the screen's measure. */
  ScalingDrawer *GetDrawer() const;
  /** How much to magnify the realm when drawing (e.g. 1, 2). */
  unsigned GetDrawScaleFactor() const;

  /** Small runes for the score and the hero's interface. */
  NumberDrawer *GetNumberDrawer() const;
  /** Great runes for the score when it is shown in glory. */
  NumberDrawer *GetBigNumberDrawer() const;
  /** Elegant script for the tale's opening and messages from the realm. */
  FontWriter *GetFancyFont() const;

  /** Let the bard play a sound by its name (e.g. death, explosion). */
  void PlaySound(std::string key);
  /** Mute or unmute the realm's sounds; the choice is remembered. */
  void ToggleSoundOutput();
  /** True if the realm's sounds are not muted. */
  bool IsSoundOutputOn() const;
  /** Start or still the bard's song; the choice is remembered. */
  void ToggleMusicPlayback();
  /** True if the bard's song is stilled. */
  bool IsMusicPlaybackOff() const;

  /** The hero's tally for this run. */
  int GetScore() const;
  /** The finest tally yet—inscribed in the scrolls. */
  int GetHighScore() const;
  /** Add to the hero's tally. */
  void AddScore(int delta);
  /** If the tally bests the record, inscribe the new high in the scrolls. */
  void UpdateHighScoreIfNeeded();

  /** Do the wizards burn with wrath? */
  bool IsAngry() const;
  /** Rouse the wizards' wrath. */
  void SetAngry();

  /** How far the hero has journeyed—which chapter is unlocked (saved). */
  int GetProgress() const;
  /** Scroll of choice: are the realm's sounds heard? */
  bool IsSoundOnSetting() const;
  /** Scroll of choice: does the bard play? */
  bool IsMusicOnSetting() const;
  /** Scroll of choice: show the wise one's guidance or not. */
  bool IsTutorialOnSetting() const;
  /** Scroll of choice: fill the window or show a frame (from config FULLSCREEN). */
  bool IsFullScreenSetting() const;
  /** Set fullscreen in config so next launch uses it. */
  void SetFullScreenSetting(bool value);
  /** Scroll of choice: are the hidden arts enabled? */
  bool AreCheatsOnSetting() const;
  /** Set cheats on/off (e.g. for tests). */
  void SetCheatsOnSetting(bool value);
  /** Scroll of choice: the hidden arts have been earned. */
  bool CheatsUnlocked() const;

  /** The battlefield's bounds in the realm's measure. */
  const Rectangle &GetBounds() const;
  /** The window's size in pixels—the realm's true measure. */
  Size GetActualResolution() const;
  /** The keeper of paths—where scrolls are read and written. */
  FilePath *GetFilePath() const;
  /** Leave the realm—close the gates and end the tale. */
  void ExitProgram();

  /** Leave behind all treasures meant for the next chapter. */
  void ClearBonusesToCarryOver();
  /** Add a treasure to carry into the next chapter. */
  void AddBonusToCarryOver(smart_pointer<TimedFireballBonus> b);
  /** The pack of treasures bound for the next chapter (gaze but do not alter). */
  const std::list<smart_pointer<TimedFireballBonus>> &
  GetBonusesToCarryOver() const;

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
  ScalingDrawer *pDr;
  NumberDrawer *pNum;
  NumberDrawer *pBigNum;
  FontWriter *pFancyNum;

  SoundInterface<Index> *pSndRaw;
  std::unique_ptr<SoundInterfaceProxy> pSnd;

  std::vector<LevelLayout> vLvl;

  int nScore;
  int nHighScore;

  bool bAngry;

  DragonGameSettings settings_;

  Rectangle rBound;

  BackgroundMusicPlayer plr;

  std::list<smart_pointer<TimedFireballBonus>> lsBonusesToCarryOver;

  TowerDataWrap *pWrp;

  ConfigurationFile *p_config_;

  DragonGameController *pSelf;

  std::unique_ptr<Preloader> pr;
};

#endif
