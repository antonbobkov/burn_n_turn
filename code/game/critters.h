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
class Princess : public Critter, public ConsumableEntity {
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

class Mage : public Critter, public ConsumableEntity {
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
class Trader : public Critter, public ConsumableEntity {
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

/** The knight: chases princess and castle, and may rise again as a ghost. */
class Knight : public Critter, public ConsumableEntity {
public:
  std::string get_class_name() override { return "Knight"; }

  Knight(const Critter &cr, LevelController *pAc_, char cType_,
         int nGhostHit_ = 1)
      : Critter(cr), pAc(pAc_), cType(cType_), nGhostHit(nGhostHit_),
        nGolemHealth(nGolemHealthMax) {}

  /** Change the type to ghost ('G'), used when a knight spawns as a ghost. */
  void SetAsGhost() { cType = 'G'; }

  void Draw(ScalingDrawer *pDr) override;

  /** Push the knight back one step along his path. */
  void KnockBack();

  /**
   * Each tick: if the knight reaches a castle, the castle is told and the
   * knight is gone. If he is a skeleton: clear the fallen and treasures, harm
   * or slay princess and trader on touch, and gather bonus pickups. When he
   * moves, advance the walk and play the step at the right moment.
   */
  void Update() override;

  void OnHit(char cWhat) override;

  Index GetImage() override { return seq.GetImageAt(0); }

  char GetType() override { return cType; }

private:
  LevelController *pAc;
  char cType;
  int nGhostHit;
  int nGolemHealth;
};

/** The great slime: it may split or merge in the dance of the MegaSlime. */
class MegaSlime : public Critter, public ConsumableEntity {
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
class Slime : public Critter, public ConsumableEntity {
public:
  std::string get_class_name() override { return "Slime"; }
  LevelController *pAc;
  Timer t;
  int nGeneration;

  Slime(fPoint fPos, Rectangle rBound, LevelController *pAc_, int nGeneration_);

  int GetGeneration() { return nGeneration; }

  void RandomizeVelocity();

  void Update() override;

  void OnHit(char cWhat) override;

  Index GetImage() override { return seq.GetImageAt(0); }

  char GetType() override { return 'L'; }

  ~Slime();
};

/** Summons slimes on a timer at a spot in the realm. */
class Sliminess : public Entity {
public:
  std::string get_class_name() override { return "Sliminess"; }
  Timer t;
  Point p;

  bool bFast;
  int nGeneration;
  LevelController *pAdv;
  std::unique_ptr<AnimationOnce> pSlm_;

  Sliminess(Point p_, LevelController *pAdv_, bool bFast_, int nGeneration_);

  void Update() override;

  /** End the summoning; the spawn and its shimmer are gone. */
  void Destroy();

  Point GetPosition() override { return p; }

  ~Sliminess();
};

/** Summons the great slimes; keeps the spot and the realm's keeper. */
class MegaSliminess : public Entity {
public:
  std::string get_class_name() override { return "MegaSliminess"; }
  Point p;
  LevelController *pAdv;
  std::unique_ptr<AnimationOnce> pSlm_;

  MegaSliminess(Point p_, LevelController *pAdv_);

  void Update() override;

  Point GetPosition() override { return p; }
  void Destroy();
};

/** A drifting slime that moves with position and velocity (e.g. in the menu). */
class FloatingSlime : public SimpleVisualEntity {
public:
  std::string get_class_name() override { return "FloatingSlime"; }
  fPoint fPos;
  fPoint fVel;

  Timer tTermination;

  FloatingSlime(ImageSequence seq, Point pStart, Point pEnd, int nTime);

  Point GetPosition() override { return fPos.ToPnt(); }
  void Update() override;
};

void SummonSkeletons(LevelController *pAc, Point p);

inline int GetTimeUntillSpell() {
  return 8 * nFramesInSecond + rand() % (3 * nFramesInSecond);
}

#endif