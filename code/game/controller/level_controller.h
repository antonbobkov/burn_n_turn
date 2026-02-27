#ifndef TOWER_DEFENSE_LEVEL_CONTROLLER_H
#define TOWER_DEFENSE_LEVEL_CONTROLLER_H

#include "game/controller/basic_controllers.h"
#include "game/critter_generators.h"
#include "game/level.h"
#include "game_utils/mouse_utils.h"
#include "utils/smart_pointer.h"
#include "utils/timer.h"
#include "wrappers/color.h"
#include "wrappers/geometry.h"
#include "wrappers/gui_key_type.h"
#include <list>
#include <memory>
#include <vector>

struct Castle;
struct Dragon;
struct DragonGameController;
struct FireballBonusAnimation;
struct Road;
struct Slime;
struct MegaSlime;
struct Sliminess;
struct MegaSliminess;
struct SoundControls;
struct TutorialLevelOne;
struct TutorialLevelTwo;
struct TutorialTextEntity;

/** The keeper of the chapter: castles, roads, dragon, spawners, treasures,
 * and the hero's input. */
struct LevelController : public EntityListController {
  std::string get_class_name() override { return "LevelController"; }
  std::vector<std::unique_ptr<Castle>> vCs;
  std::vector<std::unique_ptr<Road>> vRd;
  std::vector<std::unique_ptr<Dragon>> vDr;

  std::list<std::unique_ptr<FireballBonusAnimation>> lsBonus;
  std::list<std::unique_ptr<Slime>> lsSlimes;
  std::list<std::unique_ptr<MegaSlime>> lsMegaSlimes;
  std::list<std::unique_ptr<Sliminess>> lsSliminess;
  std::list<std::unique_ptr<MegaSliminess>> lsMegaSliminess;

  TrackballTracker tr;

  Timer t;

  bool bFirstUpdate;

  bool bGhostTime;

  bool bTimerFlash;
  Timer tBlink;
  bool bBlink;

  Timer tStep;
  bool bLeft;

  bool bCh;

  bool bLeftDown, bRightDown;
  unsigned nLastDir;
  bool bWasDirectionalInput;

  unsigned nLvl;

  int nSlimeNum;

  Timer tLoseTimer;

  std::unique_ptr<KnightGenerator> pKnightGen;
  std::unique_ptr<PrincessGenerator> pPGen;
  std::unique_ptr<TraderGenerator> pTGen;
  std::unique_ptr<MageGenerator> pMGen;
  KnightGenerator *pGr;
  MageGenerator *pMgGen;
  std::list<std::unique_ptr<EventEntity>> lsSpawnedGenerators;
  std::unique_ptr<SoundControls> pSc;

  LevelController *pSelf;

  PositionTracker pt;
  bool bTakeOffToggle;

  std::unique_ptr<TutorialLevelOne> tutOne;
  std::unique_ptr<TutorialLevelTwo> tutTwo;
  std::unique_ptr<TutorialTextEntity> pTutorialText;

  LevelController(const LevelController &) = delete;
  LevelController(DragonGameController *pGl_, Rectangle rBound, Color c,
                  const LevelLayout &lvl);

  void Init(LevelController *pSelf, const LevelLayout &lvl);

  void AddBonusAnimation(std::unique_ptr<FireballBonusAnimation> p);
  std::vector<FireballBonusAnimation *> GetBonusAnimations();
  void AddSlime(std::unique_ptr<Slime> p);
  void AddMegaSlime(std::unique_ptr<MegaSlime> p);
  void AddSliminess(std::unique_ptr<Sliminess> p);
  void AddMegaSliminess(std::unique_ptr<MegaSliminess> p);
  void AddSpawnedGenerator(std::unique_ptr<EventEntity> p);

  std::vector<ConsumableEntity *> GetConsumablePointers() override;

  /** Find the dragon in our list that matches p, or nullptr. */
  Dragon *FindDragon(Dragon *p);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void Update();

  /*virtual*/ void OnMouse(Point pPos);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ void OnMouseUp();
  /*virtual*/ void Fire();
  /*virtual*/ std::string GetControllerName() const { return "level"; }

  float GetCompletionRate();

  std::vector<EventEntity *> GetNonOwnedUpdateEntities() override;
  std::vector<VisualEntity *> GetNonOwnedDrawEntities() override;

  MouseCursor mc;

  void MegaGeneration();
  void MegaGeneration(Point p);
};

#endif
