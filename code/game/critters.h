#ifndef TOWER_DEFENSE_CRITTERS_H
#define TOWER_DEFENSE_CRITTERS_H

#include "../game_utils/draw_utils.h"
#include "dragon_constants.h"
#include "../game_utils/image_sequence.h"
#include "entities.h"
#include <memory>

class LevelController;
class Dragon;

class Castle : public Critter {
public:
  std::string get_class_name() override { return "Castle"; }

  Castle(Point p, Rectangle rBound_, LevelController *pAv_);

  void OnKnight(char cWhat);
  void Draw(ScalingDrawer *pDr) override;

  int GetPrincessCount() const { return nPrincesses; }
  void AddPrincesses(int n) { nPrincesses += n; }
  bool HasDragon() const { return pDrag != nullptr; }
  void SetDragon(Dragon *d) { pDrag = d; }
  bool IsBroken() const { return bBroken; }

private:
  int nPrincesses;
  LevelController *pAv;
  Dragon *pDrag;
  bool bBroken;
};

/** The princess: a soul the dragon can rescue and carry to the castle. */
class Princess : public Critter {
public:
  std::string get_class_name() override { return "Princess"; }

  Princess(const Critter &cr, LevelController *pAc_) : Critter(cr), pAc(pAc_) {}

  Index GetImage() override { return seq.GetImageAt(0); }

  char GetType() override { return 'P'; }

  /** When hit: show the bonus tally and vanish from the world. */
  void OnHit(char cWhat) override;

  void Draw(ScalingDrawer *pDr) override;

private:
  LevelController *pAc;
};

class Mage : public Critter {
public:
  std::string get_class_name() override { return "Mage"; }

  Mage(const Critter &cr, LevelController *pAc_, bool bAngry_);

  Index GetImage() override { return seq.GetImageAt(0); }

  void SummonSlimes();

  char GetType() override { return 'M'; }

  void Update() override;

  void OnHit(char cWhat) override;

private:
  LevelController *pAc;
  bool bAngry;
  bool bCasting;
  Timer tUntilSpell;
  Timer tSpell;
  Timer tSpellAnimate;
  fPoint fMvVel;
};

int RandomBonus(bool bInTower = true);

std::string GetBonusImage(int n);

/** The trader: drops a treasure when felled; first-bonus is tracked elsewhere. */
class Trader : public Critter {
public:
  std::string get_class_name() override { return "Trader"; }

  Trader(const Critter &cr, LevelController *pAc_, bool &bFirstBns_)
      : Critter(cr), pAc(pAc_), bFirstBns(bFirstBns_) {}

  Index GetImage() override { return seq.GetImageAt(0); }

  char GetType() override { return 'T'; }

  void OnHit(char cWhat) override;

  void Draw(ScalingDrawer *pDr) override;

private:
  LevelController *pAc;
  bool &bFirstBns;
};

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
  char GetType() override = 0;

protected:
  LevelController *pAc;
};

/** The knight: a stalwart soldier who marches on the castle. */
class Knight : public Fighter {
public:
  std::string get_class_name() override { return "Knight"; }

  Knight(const Critter &cr, LevelController *pAc_) : Fighter(cr, pAc_) {}

  char GetType() override { return 'K'; }

  void OnHit(char cWhat) override;
};

/** The skeleton: summoned by mages, it slays princesses and traders on
 * contact and destroys bonus pickups it walks over. */
class Skeleton : public Fighter {
public:
  std::string get_class_name() override { return "Skeleton"; }

  Skeleton(const Critter &cr, LevelController *pAc_) : Fighter(cr, pAc_) {}

  char GetType() override { return 'S'; }

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

  char GetType() override { return 'W'; }

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

  char GetType() override { return 'G'; }

  void OnHit(char cWhat) override;

private:
  int nGhostHit;
};

/** The great slime: it may split or merge in the dance of the MegaSlime. */
class MegaSlime : public Critter {
public:
  std::string get_class_name() override { return "MegaSlime"; }

  MegaSlime(fPoint fPos, Rectangle rBound, LevelController *pAc_);

  void Draw(ScalingDrawer *pDr) override { Critter::Draw(pDr); }

  void RandomizeVelocity();

  /**
   * Clear bonuses it has swallowed. When it touches a pickup, the treasure
   * is gone and a sound plays. Each tick: advance the dance; on jump and land,
   * play sounds and either change direction or stand still.
   */
  void Update() override;

  void OnHit(char cWhat) override;

  Index GetImage() override { return seq.GetImageAt(0); }

  char GetType() override { return 'E'; }

private:
  LevelController *pAc;
  int nHealth;
};

/** The ghost's echo: a brief shimmer where the knight fell. */
class Ghostiness : public Entity {
public:
  std::string get_class_name() override { return "Ghostiness"; }

  Ghostiness(Point p_, LevelController *pAdv_, Critter knCp_, int nGhostHit_);

  void Update() override;

private:
  Timer t;
  Point p;
  LevelController *pAdv;
  Critter knCp;
  int nGhostHit;
};

/** A slime: it drifts toward its prey, a timer guiding its steps. */
class Slime : public Critter {
public:
  std::string get_class_name() override { return "Slime"; }

  Slime(fPoint fPos, Rectangle rBound, LevelController *pAc_, int nGeneration_);

  int GetGeneration() { return nGeneration; }

  void RandomizeVelocity();

  void Update() override;

  void OnHit(char cWhat) override;

  Index GetImage() override { return seq.GetImageAt(0); }

  char GetType() override { return 'L'; }

  ~Slime();

private:
  LevelController *pAc;
  Timer t;
  int nGeneration;
};

/** Summons slimes on a timer at a spot in the realm. */
class Sliminess : public Entity {
public:
  std::string get_class_name() override { return "Sliminess"; }

  Sliminess(Point p_, LevelController *pAdv_, bool bFast_, int nGeneration_);

  void Update() override;

  /** End the summoning; the spawn and its shimmer are gone. */
  void Destroy();

  Point GetPosition() override { return p; }

  /** If the shimmer animation exists, add it to out (used when building the
   * full entity draw list). */
  void AppendSlimAnimation(std::vector<Entity *> &out) {
    if (pSlm_)
      out.push_back(pSlm_.get());
  }

  ~Sliminess();

private:
  Timer t;
  Point p;
  bool bFast;
  int nGeneration;
  LevelController *pAdv;
  std::unique_ptr<AnimationOnce> pSlm_;
};

/** Summons the great slimes; keeps the spot and the realm's keeper. */
class MegaSliminess : public Entity {
public:
  std::string get_class_name() override { return "MegaSliminess"; }

  MegaSliminess(Point p_, LevelController *pAdv_);

  void Update() override;

  Point GetPosition() override { return p; }
  void Destroy();

  /** If the shimmer animation exists, add it to out (used when building the
   * full entity draw list). */
  void AppendSlimAnimation(std::vector<Entity *> &out) {
    if (pSlm_)
      out.push_back(pSlm_.get());
  }

private:
  Point p;
  LevelController *pAdv;
  std::unique_ptr<AnimationOnce> pSlm_;
};

/** A drifting slime that moves with position and velocity (e.g. in the menu). */
class FloatingSlime : public SimpleVisualEntity {
public:
  std::string get_class_name() override { return "FloatingSlime"; }

  FloatingSlime(ImageSequence seq, Point pStart, Point pEnd, int nTime);

  Point GetPosition() override { return fPos.ToPnt(); }
  void Update() override;

private:
  fPoint fPos;
  fPoint fVel;
  Timer tTermination;
};

void SummonSkeletons(LevelController *pAc, Point p);

inline int GetTimeUntillSpell() {
  return 8 * nFramesInSecond + rand() % (3 * nFramesInSecond);
}

#endif