#ifndef TOWER_DEFENSE_LEVEL_CONTROLLER_H
#define TOWER_DEFENSE_LEVEL_CONTROLLER_H

#include "wrappers/color.h"
#include "game/controller/basic_controllers.h"
#include "wrappers/geometry.h"
#include "wrappers/gui_key_type.h"
#include "game/level.h"
#include "game_utils/mouse_utils.h"
#include "utils/smart_pointer.h"
#include "utils/timer.h"
#include <memory>


struct Castle;
struct Dragon;
struct DragonGameControllerList;
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

/** Main game controller: castles, roads, dragon, generators, bonuses, input.
 */
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
  smart_pointer<SoundControls> pSc;

  LevelController *pSelf;

  PositionTracker pt;
  bool bTakeOffToggle;

  std::unique_ptr<TutorialLevelOne> tutOne;
  std::unique_ptr<TutorialLevelTwo> tutTwo;
  smart_pointer<TutorialTextEntity> pTutorialText;

  LevelController(const LevelController &) = delete;
  LevelController(DragonGameControllerList *pGl_, Rectangle rBound, Color c,
                  const LevelLayout &lvl);

  void Init(LevelController *pSelf, const LevelLayout &lvl);

  /** Return the smart_pointer in vDr that wraps p, or null. */
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
