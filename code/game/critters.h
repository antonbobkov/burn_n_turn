#ifndef TOWER_DEFENSE_CRITTERS_H
#define TOWER_DEFENSE_CRITTERS_H

#include "game_utils/draw_utils.h"
#include "dragon_constants.h"
#include "game_utils/image_sequence.h"
#include "entities.h"
#include "utils/smart_pointer.h"

struct LevelController;
struct Dragon;

struct Castle : public Critter {
  std::string get_class_name() override { return "Castle"; }
  unsigned nPrincesses;
  LevelController *pAv;
  Dragon *pDrag;

  bool bBroken;

  Castle(Point p, Rectangle rBound_, LevelController *pAv_);

  void OnKnight(char cWhat);

  void Draw(ScalingDrawer *pDr) override;
};

/** The princess: a soul the dragon can rescue and carry to the castle. */
struct Princess : public Critter, public ConsumableEntity {
  std::string get_class_name() override { return "Princess"; }
  LevelController *pAc;

  Princess(const Critter &cr, LevelController *pAc_) : Critter(cr), pAc(pAc_) {}

  Index GetImage() override { return seq.vImage[0]; }

  char GetType() override { return 'P'; }

  /** When hit: show the bonus tally and vanish from the world. */
  void OnHit(char cWhat) override;

  void Draw(ScalingDrawer *pDr) override;
};

struct Mage : public Critter, public ConsumableEntity {
  std::string get_class_name() override { return "Mage"; }
  LevelController *pAc;

  bool bAngry;
  bool bCasting;
  Timer tUntilSpell;
  Timer tSpell;
  Timer tSpellAnimate;

  fPoint fMvVel;

  Mage(const Critter &cr, LevelController *pAc_, bool bAngry_);

  Index GetImage() override { return seq.vImage[0]; }

  void SummonSlimes();

  char GetType() override { return 'M'; }

  void Update() override;

  void OnHit(char cWhat) override;
};

unsigned RandomBonus(bool bInTower = true);

std::string GetBonusImage(int n);

/** The trader: drops a treasure when felled; first-bonus is tracked elsewhere. */
struct Trader : public Critter, public ConsumableEntity {
  std::string get_class_name() override { return "Trader"; }
  LevelController *pAc;
  bool &bFirstBns;

  Trader(const Critter &cr, LevelController *pAc_, bool &bFirstBns_)
      : Critter(cr), pAc(pAc_), bFirstBns(bFirstBns_) {}

  Index GetImage() override { return seq.vImage[0]; }

  char GetType() override { return 'T'; }

  void OnHit(char cWhat) override;

  void Draw(ScalingDrawer *pDr) override;
};

/** The knight: chases princess and castle, and may rise again as a ghost. */
struct Knight : public Critter, public ConsumableEntity {
  std::string get_class_name() override { return "Knight"; }
  LevelController *pAc;

  char cType;

  int nGhostHit;
  int nGolemHealth;

  Knight(const Critter &cr, LevelController *pAc_, char cType_)
      : Critter(cr), pAc(pAc_), cType(cType_), nGhostHit(1),
        nGolemHealth(nGolemHealthMax) {}

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

  Index GetImage() override { return seq.vImage[0]; }

  char GetType() override { return cType; }
};

/** The great slime: it may split or merge in the dance of the MegaSlime. */
struct MegaSlime : public Critter, public ConsumableEntity {
  std::string get_class_name() override { return "MegaSlime"; }
  LevelController *pAc;

  int nHealth;

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

  Index GetImage() override { return seq.vImage[0]; }

  char GetType() override { return 'E'; }
};

/** The ghost's echo: a brief shimmer where the knight fell. */
struct Ghostiness : public EventEntity {
  std::string get_class_name() override { return "Ghostiness"; }
  Timer t;
  Point p;

  LevelController *pAdv;

  Critter knCp;
  int nGhostHit;

  Ghostiness(Point p_, LevelController *pAdv_, Critter knCp_, int nGhostHit_);

  void Update() override;
};

/** A slime: it drifts toward its prey, a timer guiding its steps. */
struct Slime : public Critter, public ConsumableEntity {
  std::string get_class_name() override { return "Slime"; }
  LevelController *pAc;
  Timer t;
  int nGeneration;

  Slime(fPoint fPos, Rectangle rBound, LevelController *pAc_, int nGeneration_);

  int GetGeneration() { return nGeneration; }

  void RandomizeVelocity();

  void Update() override;

  void OnHit(char cWhat) override;

  Index GetImage() override { return seq.vImage[0]; }

  char GetType() override { return 'L'; }

  ~Slime();
};

/** Summons slimes on a timer at a spot in the realm. */
struct Sliminess : public EventEntity {
  std::string get_class_name() override { return "Sliminess"; }
  Timer t;
  Point p;

  bool bFast;
  int nGeneration;
  LevelController *pAdv;
  smart_pointer<AnimationOnce> pSlm;

  Sliminess(Point p_, LevelController *pAdv_, bool bFast_, int nGeneration_);

  void Update() override;

  /** End the summoning; the spawn and its shimmer are gone. */
  void Kill();

  Point GetPosition() { return p; }

  ~Sliminess();
};

/** Summons the great slimes; keeps the spot and the realm's keeper. */
struct MegaSliminess : public EventEntity {
  std::string get_class_name() override { return "MegaSliminess"; }
  Point p;
  LevelController *pAdv;
  smart_pointer<AnimationOnce> pSlm;

  MegaSliminess(Point p_, LevelController *pAdv_);

  void Update() override;

  Point GetPosition() { return p; }
  void Kill();
};

/** A drifting slime that moves with position and velocity (e.g. in the menu). */
struct FloatingSlime : public SimpleVisualEntity {
  std::string get_class_name() override { return "FloatingSlime"; }
  fPoint fPos;
  fPoint fVel;

  Timer tTermination;

  FloatingSlime(ImageSequence seq, Point pStart, Point pEnd, unsigned nTime);

  Point GetPosition() override { return fPos.ToPnt(); }
  void Update() override;
};

void SummonSkeletons(LevelController *pAc, Point p);

inline unsigned GetTimeUntillSpell() {
  return 8 * nFramesInSecond + rand() % (3 * nFramesInSecond);
}

#endif