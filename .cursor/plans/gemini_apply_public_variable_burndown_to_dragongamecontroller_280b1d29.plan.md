---
name: Apply Public Variable Burndown to DragonGameController
overview: Encapsulate all public members of DragonGameController, making them private and adding accessors, and update all call sites.
todos:
  - id: "1"
    content: Modify DragonGameController Header
    status: pending
  - id: "2"
    content: Modify DragonGameController Implementation
    status: pending
  - id: "3"
    content: Update DragonGameRunner
    status: pending
  - id: "4"
    content: Update LevelController
    status: pending
  - id: "5"
    content: Update MenuController
    status: pending
  - id: "6"
    content: Update BasicControllers
    status: pending
  - id: "7"
    content: Update BuyNowController
    status: pending
  - id: "8"
    content: Update Critters
    status: pending
  - id: "9"
    content: Update Fireball
    status: pending
  - id: "10"
    content: Update Entities
    status: pending
  - id: "11"
    content: Update Simulation
    status: pending
  - id: "12"
    content: Update CritterGenerators
    status: pending
  - id: "13"
    content: Update Dragon
    status: pending
  - id: "14"
    content: Update Level
    status: pending
isProject: false
---

1. **Modify DragonGameController Header (`code/game/controller/dragon_game_controller.h`)**:
  - Change `struct` to `class`.
    - Move all member variables to `private` section.
    - Add public accessor methods:
      - `GameController* GetActiveController()`
      - `size_t GetControllerCount()`
      - `unsigned GetActiveIndex()`
      - `void SetActiveIndex(unsigned n)`
      - `int GetLevelPointer(unsigned i)`
      - `GraphicalInterface<Index>* GetGraph()`
      - `ScalingDrawer* GetDrawer()`
      - `NumberDrawer* GetNumberDrawer()`
      - `NumberDrawer* GetBigNumberDrawer()`
      - `FontWriter* GetFancyWriter()`
      - `SoundInterfaceProxy* GetSound()`
      - `int GetScore()`
      - `void SetScore(int n)`
      - `void AddScore(int n)`
      - `int GetHighScore()`
      - `void UpdateHighScore(int n)`
      - `bool IsAngry()`
      - `void SetAngry(bool b)`
      - `int GetProgress()`
      - `void SetProgress(int n)`
      - `bool IsSoundOn()`, `void ToggleSound()`
      - `bool IsMusicOn()`, `void ToggleMusic()`
      - `bool IsTutorialOn()`, `void ToggleTutorial()`
      - `bool IsFullScreen()`, `void ToggleFullScreen()`
      - `bool IsCheatsOn()`, `void ToggleCheats()`
      - `bool IsCheatsUnlocked()`
      - `Rectangle GetBounds()`
      - `BackgroundMusicPlayer& GetMusicPlayer()`
      - `std::list<smart_pointer<TimedFireballBonus>>& GetBonusesToCarryOver()`
      - `TowerDataWrap* GetTowerData()`
    - Remove `pSelf` member variable.
2. **Modify DragonGameController Implementation (`code/game/controller/dragon_game_controller.cc`)**:
  - Implement the new accessor methods.
    - Update `StartUp` and `Restart` to use `this` instead of `pSelf` member.
    - Ensure constructors initialize private members correctly.
3. **Update Consumers**:
  - `**code/game/dragon_game_runner.cc**`: Use `GetActiveController`, `GetActiveIndex`, `GetControllerCount`.
    - `**code/game/controller/level_controller.cc**`: Update access to `nActive` (via Set), `pGraph`, `pNum`, `pBigNum`, `pSnd`, `nScore`, `sbMusicOn`, `sbTutorialOn`, `sbCheatsOn`, `rBound`, `lsBonusesToCarryOver`, `pWrp`.
    - `**code/game/controller/menu_controller.cc**`: Update access to `nActive`, `vLevelPointers`, `pGraph`, `pDr`, `pNum`, `pSnd`, `snProgress`, `sbSoundOn`, `sbMusicOn`, `sbTutorialOn`, `sbFullScreen`, `sbCheatsOn`, `plr`, `pWrp`.
    - `**code/game/controller/basic_controllers.cc**`: Update `pGraph`, `pDr`, `pSnd`, `nScore`, `nHighScore`, `sbCheatsOn`, `pWrp`.
    - `**code/game/controller/buy_now_controller.cc**`: Update `pGraph`, `pDr`.
    - `**code/game/critters.cc**`: Update `pNum`, `pSnd`, `bAngry`.
    - `**code/game/fireball.cc**`: Update `pNum`, `pSnd`.
    - `**code/game/entities.cc**`: Update `pBigNum`, `pFancyNum`, `pNum`, `nScore`, `nHighScore`, `plr`.
    - `**code/simulation/simulation.cc**`: Update `nScore`, `nHighScore`.
    - `**code/game/critter_generators.cc**`: Update `nHighScore`, `bAngry`.
    - `**code/game/dragon.cc**`: Update `pSnd`, `lsBonusesToCarryOver`.
    - `**code/game/level.cc**`: Update `GetImg`.
4. **Verification**:
  - Verify that all accesses are covered and no direct member access remains.
    - Compile and check for errors (if possible/applicable).

