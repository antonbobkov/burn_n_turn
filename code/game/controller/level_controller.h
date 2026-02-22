#ifndef TOWER_DEFENSE_LEVEL_CONTROLLER_H
#define TOWER_DEFENSE_LEVEL_CONTROLLER_H

#include "game/controller/basic_controllers.h"
#include "game/level.h"
#include "game_utils/mouse_utils.h"
#include "utils/smart_pointer.h"
#include "utils/timer.h"
#include "wrappers/color.h"
#include "wrappers/geometry.h"
#include "wrappers/gui_key_type.h"
#include <memory>

struct Castle;
struct Dragon;
struct DragonGameController;
struct FireballBonusAnimation;
struct KnightGenerator;
struct MageGenerator;
struct Road;
struct Slime;
struct Sliminess;
struct SoundControls;
struct TutorialLevelOne;
struct TutorialLevelTwo;
struct TutorialTextEntity;

/** The keeper of the chapter: castles, roads, dragon, spawners, treasures,
 * and the hero's input. */
struct LevelController : public EntityListController {
  std::string get_class_name() override { return "LevelController"; }
  std::vector<std::unique_ptr<Castle>> vCs;
  std::vector<smart_pointer<Road>> vRd;
  std::vector<smart_pointer<Dragon>> vDr;

  std::list<smart_pointer<FireballBonusAnimation>> lsBonus;
  std::list<smart_pointer<Slime>> lsSlimes;
  std::list<smart_pointer<Sliminess>> lsSliminess;

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

  bool bPaused;

  Timer tLoseTimer;

  KnightGenerator *pGr;
  MageGenerator *pMgGen;
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

  /** Find the dragon in our list that matches p, or return nothing. */
  smart_pointer<Dragon> FindDragon(Dragon *p);

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
