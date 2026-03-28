#ifndef TOWER_DEFENSE_CRITTERS_FIGHTER_H
#define TOWER_DEFENSE_CRITTERS_FIGHTER_H

#include "critters.h"
#include "../dragon_constants.h"

class LevelController;
class Castle;

/** The hero of the dark army: marches on castles, fells all in its path.
 * Fighter is the base for Knight, Skeleton, Golem, and Ghost — each a
 * different face of the enemy horde. */
class Fighter : public Critter {
public:
  std::string get_class_name() override = 0;

  Fighter(const Critter &cr, LevelController *pAc_)
      : Critter(cr), pAc(pAc_) {}

  void Draw(ScalingDrawer *pDr) override;

  Index GetImage() override { return seq.GetImageAt(0); }

  /** Each tick: march toward the castle gate; if reached, the siege begins
   * and the fighter vanishes. When moving, step the walk animation and play
   * footstep sounds. */
  virtual void Update() override;

  void OnHit(char cWhat) override = 0;
  std::string GetType() override = 0;

protected:
  LevelController *pAc;
};

/** The knight: a stalwart soldier who marches on the castle. */
class Knight : public Fighter {
public:
  std::string get_class_name() override { return "Knight"; }

  Knight(const Critter &cr, LevelController *pAc_) : Fighter(cr, pAc_) {}

  std::string GetType() override { return "knight"; }

  void OnHit(char cWhat) override;
};

/** The skeleton: summoned by mages, it slays princesses and traders on
 * contact and destroys bonus pickups it walks over. */
class Skeleton : public Fighter {
public:
  std::string get_class_name() override { return "Skeleton"; }

  Skeleton(const Critter &cr, LevelController *pAc_) : Fighter(cr, pAc_) {}

  std::string GetType() override { return "skeleton"; }

  /** Each tick: slay any princess or trader on contact and devour bonus
   * pickups, then do the shared fighter march and animation. */
  void Update() override;

  void OnHit(char cWhat) override;
};

/** The golem: a stone giant that shrugs off fireballs and smashes castles
 * regardless of their princess count. Takes 70 hits to bring down. */
class Golem : public Fighter {
public:
  std::string get_class_name() override { return "Golem"; }

  Golem(const Critter &cr, LevelController *pAc_)
      : Fighter(cr, pAc_), nGolemHealth(nGolemHealthMax) {}

  std::string GetType() override { return "golem"; }

  void OnHit(char cWhat) override;

private:
  /** Shove the golem back one step along its path — each fireball pushes it. */
  void KnockBack();

  int nGolemHealth;
};

/** The ghost: a specter that rises when a knight falls. It may respawn once
 * more as a pure ghost before it fades forever.
 *
 * Two ghost levels:
 *   nGhostHit = 1 (default): Ghost Knight — armored spirit, spawns a pure
 *                             ghost on death (ghost_knight sprite).
 *   nGhostHit = 0:           Pure Ghost — the final echo, vanishes on death
 *                             (ghost sprite). */
class Ghost : public Fighter {
public:
  std::string get_class_name() override { return "Ghost"; }

  /** nGhostHit controls how many more times this ghost can respawn.
   * The constructor picks the correct sprite automatically. */
  Ghost(const Critter &cr, LevelController *pAc_, int nGhostHit_ = 1);

  std::string GetType() override { return "ghost"; }

  void OnHit(char cWhat) override;

private:
  int nGhostHit;
};

/** Waits on a timer, then vanishes and summons a skeleton knight at a spot. */
class SpawningSkeleton : public Entity {
public:
  std::string get_class_name() override { return "SpawningSkeleton"; }

  SpawningSkeleton(Point p_, LevelController *pAdv_);

  void Update() override;

private:
  Timer t;
  Point p;
  LevelController *pAdv;
};

/** Waits on a timer where a knight fell, then vanishes and summons a ghost. */
class SpawningGhost : public Entity {
public:
  std::string get_class_name() override { return "SpawningGhost"; }

  SpawningGhost(Point p_, LevelController *pAdv_, Critter knCp_, int nGhostHit_);

  void Update() override;

private:
  Timer t;
  Point p;
  LevelController *pAdv;
  Critter knCp;
  int nGhostHit;
};

/** Summon a ring of skeletons at point p, scaled to the current level. */
void SummonSkeletons(LevelController *pAc, Point p);

#endif
