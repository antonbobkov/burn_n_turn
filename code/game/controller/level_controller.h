#ifndef TOWER_DEFENSE_LEVEL_CONTROLLER_H
#define TOWER_DEFENSE_LEVEL_CONTROLLER_H

#include "controller/basic_controllers.h"
#include "controller/menu_controller.h"
#include "gameplay.h"
#include "level.h"
#include "mouse_utils.h"
#include "smart_pointer.h"
#include "tutorial.h"

struct Castle;
struct Dragon;
struct FireballBonusAnimation;
struct KnightGenerator;
struct MageGenerator;
struct Road;
struct Slime;
struct Sliminess;

/** Main game controller: castles, roads, dragon, generators, bonuses, input.
 */
struct LevelController : public EntityListController {
  std::string get_class_name() override { return "LevelController"; }
  std::vector<ASSP<Castle>> vCs;
  std::vector<ASSP<Road>> vRd;
  std::vector<ASSP<Dragon>> vDr;

  std::list<ASSP<FireballBonusAnimation>> lsBonus;
  std::list<ASSP<Slime>> lsSlimes;
  std::list<ASSP<Sliminess>> lsSliminess;

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

  TutorialLevelOne tutOne;
  TutorialLevelTwo tutTwo;
  SSP<TutorialTextEntity> pTutorialText;

  LevelController(const LevelController &a);

  LevelController(smart_pointer<TwrGlobalController> pGl_, Rectangle rBound,
                  Color c, const LevelLayout &lvl);

  void Init(LevelController *pSelf, const LevelLayout &lvl);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void Update();

  /*virtual*/ void OnMouse(Point pPos);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ void OnMouseUp();
  /*virtual*/ void Fire();
  /*virtual*/ std::string GetControllerName() const { return "level"; }

  float GetCompletionRate();

  MouseCursor mc;

  void MegaGeneration();
  void MegaGeneration(Point p);
};

#endif
