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
  smart_pointer<Dragon> pDrag;

  bool bBroken;

  Castle(Point p, Rectangle rBound_, LevelController *pAv_);

  void OnKnight(char cWhat);

  /*unsigned*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

/** The princess: a soul the dragon can rescue and carry to the castle. */
struct Princess : public Critter, public ConsumableEntity {
  std::string get_class_name() override { return "Princess"; }
  LevelController *pAc;

  Princess(const Critter &cr, LevelController *pAc_) : Critter(cr), pAc(pAc_) {}

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'P'; }

  /** When hit: show the bonus tally and vanish from the world. */
  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
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

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  void SummonSlimes();

  /*virtual*/ char GetType() { return 'M'; }

  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);
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

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'T'; }

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
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

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /** Push the knight back one step along his path. */
  void KnockBack();

  /**
   * Each tick: if the knight reaches a castle, the castle is told and the
   * knight is gone. If he is a skeleton: clear the fallen and treasures, harm
   * or slay princess and trader on touch, and gather bonus pickups. When he
   * moves, advance the walk and play the step at the right moment.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return cType; }
};

/** The great slime: it may split or merge in the dance of the MegaSlime. */
struct MegaSlime : public Critter, public ConsumableEntity {
  std::string get_class_name() override { return "MegaSlime"; }
  LevelController *pAc;

  int nHealth;

  MegaSlime(fPoint fPos, Rectangle rBound, LevelController *pAc_);

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr) {
    Critter::Draw(pDr);
  }

  void RandomizeVelocity();

  /**
   * Clear bonuses it has swallowed. When it touches a pickup, the treasure
   * is gone and a sound plays. Each tick: advance the dance; on jump and land,
   * play sounds and either change direction or stand still.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'E'; }
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

  /*virutal*/ void Update();
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

  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'L'; }

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

  /*virutal*/ void Update();

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

  /*virutal*/ void Update();
};

/** A drifting slime that moves with position and velocity (e.g. in the menu). */
struct FloatingSlime : public SimpleVisualEntity {
  std::string get_class_name() override { return "FloatingSlime"; }
  fPoint fPos;
  fPoint fVel;

  Timer tTermination;

  FloatingSlime(ImageSequence seq, Point pStart, Point pEnd, unsigned nTime);

  /*virtual*/ Point GetPosition() { return fPos.ToPnt(); }
  /*virtual*/ void Update();
};

void SummonSkeletons(LevelController *pAc, Point p);

inline unsigned GetTimeUntillSpell() {
  return 8 * nFramesInSecond + rand() % (3 * nFramesInSecond);
}

#endif
