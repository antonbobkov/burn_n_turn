#ifndef TOWER_DEFENSE_GAMEPLAY_H
#define TOWER_DEFENSE_GAMEPLAY_H

#include "smart_pointer.h"
#include "controller/menu_controller.h"

/** Included after critter_generators.h and dragon.h from tower_defense.h. */
struct AdvancedController;

/** Tracks mouse for trackball-style steering (angle and fire). */
struct TrackballTracker {
  MouseTracker mtr;

  std::list<Point> lsMouse;
  int nMaxLength;
  bool trigFlag;
  int threshold;
  TrackballTracker();

  void Update();

  bool IsTrigger();

  Point GetMovement() { return *(++lsMouse.begin()); }

  fPoint GetAvMovement();

  int GetLengthSq(Point p) { return p.x * p.x + p.y * p.y; }

  int GetDerivative();
};

/** Floating "+N" score text at a point; animates then removes. */
struct BonusScore : public EventEntity, public VisualEntity {
  SSP<AdvancedController> pAc;
  std::string sText;
  unsigned nScore;
  unsigned nScoreSoFar;
  Point p;
  Timer t;
  unsigned nC;
  Color c;

  BonusScore(smart_pointer<AdvancedController> pAc_, Point p_,
             unsigned nScore_);

  /*virtual*/ void Update();

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return 5; }

  /*virtual*/ Point GetPosition() { return p; }
};

struct SoundControls : public EventEntity {
  BackgroundMusicPlayer &plr;
  int nTheme;

  SoundControls(BackgroundMusicPlayer &plr_, int nTheme_)
      : plr(plr_), nTheme(nTheme_) {}

  /*virtual*/ void Update();
};

/** Tracks mouse position, last down, press state and counter for input. */
struct PositionTracker {
  Point pMouse;
  Point pLastDownPosition;
  bool bPressed;
  int nCounter;

  PositionTracker() : bPressed(false) {}

  void On();
  int Off();
  void Update();

  void UpdateMouse(Point pMouse_);
  void UpdateLastDownPosition(Point pMouse_);
  fPoint GetDirection(fPoint fDragonPos) { return pMouse - fDragonPos; }
  fPoint GetFlightDirection(fPoint fDragonPos);
};

/** Main game controller: castles, roads, dragon, generators, bonuses, input.
 */
struct AdvancedController : public EntityListController {
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

  smart_pointer<AdvancedController> pSelf;

  PositionTracker pt;
  bool bTakeOffToggle;

  TutorialLevelOne tutOne;
  TutorialLevelTwo tutTwo;
  SSP<TutorialTextEntity> pTutorialText;

  AdvancedController(const AdvancedController &a);

  AdvancedController(smart_pointer<TwrGlobalController> pGl_, Rectangle rBound,
                     Color c, const LevelLayout &lvl);

  void Init(smart_pointer<AdvancedController> pSelf,
            const LevelLayout &lvl);

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

/** Draws high score in a rectangle. */
struct HighScoreShower : public VisualEntity {
  SSP<TwrGlobalController> pGl;
  Rectangle rBound;

  HighScoreShower(smart_pointer<TwrGlobalController> pGl_, Rectangle rBound_)
      : pGl(this, pGl_), rBound(rBound_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

struct IntroTextShower : public VisualEntity {
  SSP<TwrGlobalController> pGl;
  Rectangle rBound;

  IntroTextShower(smart_pointer<TwrGlobalController> pGl_, Rectangle rBound_)
      : pGl(this, pGl_), rBound(rBound_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

#endif
