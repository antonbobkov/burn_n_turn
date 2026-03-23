#ifndef TOWER_DEFENSE_LEVEL_CONTROLLER_H
#define TOWER_DEFENSE_LEVEL_CONTROLLER_H

#include "../../game_utils/mouse_utils.h"
#include "../../utils/timer.h"
#include "../../wrappers/color.h"
#include "../../wrappers/geometry.h"
#include "../../wrappers/gui_key_type.h"
#include "../critter_generators.h"
#include "../level.h"
#include "basic_controllers.h"
#include <list>
#include <memory>
#include <vector>

class Castle;
class Dragon;
class DragonGameController;
class FireballBonusAnimation;
class Road;
class Slime;
class MegaSlime;
class Sliminess;
class MegaSliminess;
class SoundControls;
class Tutorial;
class TutorialTextEntity;
enum class TutorialEvent;

/** The keeper of the chapter: castles, roads, dragon, spawners, treasures,
 * and the hero's input. */
class LevelController : public EntityListController {
public:
  std::string get_class_name() { return "LevelController"; }

  /** Deliver a game event to the active tutorial handler. */
  void TutorialNotify(TutorialEvent event);

  /** Own a consumable entity (knight, princess, …). */
  template <class T> void AddOwnedConsumable(std::unique_ptr<T> p) {
    lsPpl_.push_back(std::unique_ptr<ConsumableEntity>(p.release()));
  }

  /** Pointers to people (non-slime consumables) for targeted iteration. */
  std::vector<ConsumableEntity *> GetPeoplePointers();

  std::vector<ConsumableEntity *> GetConsumablePointers();

  LevelController(const LevelController &) = delete;
  LevelController(DragonGameController *pGl_, Rectangle rBound, Color c,
                  const LevelLayout &lvl, std::unique_ptr<SoundControls> pSc);
  ~LevelController();

  void Init(const LevelLayout &lvl);

  void AddBonusAnimation(std::unique_ptr<FireballBonusAnimation> p);
  std::vector<FireballBonusAnimation *> GetBonusAnimations();
  void AddSlime(std::unique_ptr<Slime> p);
  void AddMegaSlime(std::unique_ptr<MegaSlime> p);
  void AddSliminess(std::unique_ptr<Sliminess> p);
  void AddMegaSliminess(std::unique_ptr<MegaSliminess> p);
  void AddSpawnedGenerator(std::unique_ptr<Entity> p);

  /** Find the dragon in our list that matches p, or nullptr. */
  Dragon *FindDragon(Dragon *p);

  void OnKey(GuiKeyType c, bool bUp) override;

  void Update() override;

  void OnMouse(Point pPos) override;

  void OnMouseDown(Point pPos) override;
  void OnMouseUp() override;
  void Fire() override;
  std::string GetControllerName() const override { return "level"; }

  float GetCompletionRate();

  std::vector<Entity *> GetNonOwnedEntities() override;

  void MegaGeneration();
  void MegaGeneration(Point p);

  // Narrow accessors replacing direct public-variable access
  int GetLevel() const { return nLvl_; }
  bool IsGhostTime() const { return bGhostTime_; }
  bool IsCheating() const { return bCh_; }
  /** Start the lose countdown if not already started. */
  void StartLoseTimer();
  /** Frame index for castle destruction animation. */
  int GetLoseTimerFrame() const;
  /** Stop background music (set theme to silent). */
  void StopMusic();
  /** Increment live slime count. */
  void IncrementSlimeCount() { ++nSlimeNum_; }
  /** Decrement live slime count. */
  void DecrementSlimeCount() { --nSlimeNum_; }
  /** Current live slime count. */
  int GetSlimeCount() const { return nSlimeNum_; }
  /** Kill all active slimes/sliminess and trigger mega-generation at centroid. */
  void DoSlimeMassKill();

  Castle *GetFirstCastle();
  std::vector<Castle *> GetCastlePointers();
  /** Pick a random road and return its spawn point and direction. */
  void GetRandomRoadLocation(Point &p, Point &v);

 private:
  friend class AdNumberDrawer;
  friend class BonusDrawer;

  std::list<std::unique_ptr<ConsumableEntity>> lsPpl_;

  std::vector<std::unique_ptr<Castle>> vCs_;
  std::vector<std::unique_ptr<Road>> vRd_;
  std::vector<std::unique_ptr<Dragon>> vDr_;

  std::list<std::unique_ptr<FireballBonusAnimation>> lsBonus_;
  std::list<std::unique_ptr<Slime>> lsSlimes_;
  std::list<std::unique_ptr<MegaSlime>> lsMegaSlimes_;
  std::list<std::unique_ptr<Sliminess>> lsSliminess_;
  std::list<std::unique_ptr<MegaSliminess>> lsMegaSliminess_;

  Timer t_;
  bool bFirstUpdate_;
  bool bGhostTime_;
  bool bTimerFlash_;
  Timer tBlink_;
  bool bBlink_;
  Timer tStep_;
  bool bLeft_;
  bool bCh_;
  bool bLeftDown_, bRightDown_;
  int nLastDir_;
  bool bWasDirectionalInput_;
  int nLvl_;
  int nSlimeNum_;
  Timer tLoseTimer_;
  PositionTracker pt_;
  bool bTakeOffToggle_;
  TrackballTracker tr_;

  std::unique_ptr<KnightGenerator> pKnightGen_;
  std::unique_ptr<PrincessGenerator> pPGen_;
  std::unique_ptr<TraderGenerator> pTGen_;
  std::unique_ptr<MageGenerator> pMGen_;
  KnightGenerator *pGr_;
  MageGenerator *pMgGen_;
  std::list<std::unique_ptr<Entity>> lsSpawnedGenerators_;

  std::unique_ptr<SoundControls> pSc_;
  std::unique_ptr<TutorialTextEntity> pTutorialText_;
  std::unique_ptr<Tutorial> pTutorial_;

  MouseCursor mc_;
};

#endif
