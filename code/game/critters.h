#ifndef TOWER_DEFENSE_CRITTERS_H
#define TOWER_DEFENSE_CRITTERS_H

#include "entities.h"
#include "smart_pointer.h"

struct AdvancedController;
struct Dragon;

struct Castle : public Critter {
  unsigned nPrincesses;
  SSP<AdvancedController> pAv;
  SSP<Dragon> pDrag;

  bool bBroken;

  Castle(Point p, Rectangle rBound_, smart_pointer<AdvancedController> pAv_);

  void OnKnight(char cWhat);

  /*unsigned*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

/** Princess unit: Critter + ConsumableEntity, captured by dragon. */
struct Princess : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;

  Princess(const Critter &cr, smart_pointer<AdvancedController> pAc_)
      : Critter(cr), pAc(this, pAc_) {}

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'P'; }

  /** Spawn BonusScore, set bExist false. */
  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

struct Mage : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;

  bool bAngry;
  bool bCasting;
  Timer tUntilSpell;
  Timer tSpell;
  Timer tSpellAnimate;

  fPoint fMvVel;

  Mage(const Critter &cr, smart_pointer<AdvancedController> pAc_, bool bAngry_);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  void SummonSlimes();

  /*virtual*/ char GetType() { return 'M'; }

  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);
};

unsigned RandomBonus(bool bInTower = true);

ImageSequence GetBonusImage(int n, Preloader &pr);

/** Trader unit: drops bonus, bFirstBns ref for first-bonus logic. */
struct Trader : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;
  bool &bFirstBns;

  Trader(const Critter &cr, smart_pointer<AdvancedController> pAc_,
         bool &bFirstBns_)
      : Critter(cr), pAc(this, pAc_), bFirstBns(bFirstBns_) {}

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'T'; }

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

/** Knight unit: chases princess/castle, can become ghost (Ghostiness). */
struct Knight : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;

  char cType;

  int nGhostHit;
  int nGolemHealth;

  Knight(const Critter &cr, smart_pointer<AdvancedController> pAc_, char cType_)
      : Critter(cr), pAc(this, pAc_), cType(cType_), nGhostHit(1),
        nGolemHealth(nGolemHealthMax) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /** Move position back one unit along current velocity. */
  void KnockBack();

  /**
   * Each frame: if the knight is on a castle, notify the castle and remove the
   * knight. If it is a skeleton: remove dead units and bonuses, then damage or
   * kill princesses and traders on hit, and collect bonus pickups. When the
   * knight moves, advance the walk animation and play step sounds at the right
   * moments.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return cType; }
};

/** Large slime unit: splits or merges (MegaSlime logic). */
struct MegaSlime : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;

  int nHealth;

  MegaSlime(fPoint fPos, Rectangle rBound,
            smart_pointer<AdvancedController> pAc_);

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr) {
    Critter::Draw(pDr);
  }

  void RandomizeVelocity();

  /**
   * Remove collected bonuses. When the slime touches a bonus pickup, remove it
   * and play a sound. On the timer: advance the animation; on jump and land
   * frames, play sounds and either randomize movement or stop.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'E'; }
};

/** Ghost knight effect: timed visual at a position. */
struct Ghostiness : public EventEntity {
  Timer t;
  Point p;

  SSP<AdvancedController> pAdv;

  Critter knCp;
  int nGhostHit;

  Ghostiness(Point p_, smart_pointer<AdvancedController> pAdv_, Critter knCp_,
             int nGhostHit_);

  /*virutal*/ void Update();
};

/** Slime unit: moves toward target, timer for behavior. */
struct Slime : public Critter, public ConsumableEntity {
  SSP<AdvancedController> pAc;
  Timer t;
  int nGeneration;

  Slime(fPoint fPos, Rectangle rBound, smart_pointer<AdvancedController> pAc_,
        int nGeneration_);

  int GetGeneration() { return nGeneration; }

  void RandomizeVelocity();

  /*virtual*/ void Update();

  /*virtual*/ void OnHit(char cWhat);

  /*virtual*/ Index GetImage() { return seq.vImage[0]; }

  /*virtual*/ char GetType() { return 'L'; }

  ~Slime();
};

/** Spawns slimes on a timer at a position. */
struct Sliminess : public EventEntity {
  Timer t;
  Point p;

  bool bFast;
  int nGeneration;
  SSP<AdvancedController> pAdv;
  SSP<AnimationOnce> pSlm;

  Sliminess(Point p_, smart_pointer<AdvancedController> pAdv_, bool bFast_,
            int nGeneration_);

  /*virutal*/ void Update();

  /** Cancels the spawn (this and the visual both removed). */
  void Kill();

  Point GetPosition() { return p; }

  ~Sliminess();
};

/** Spawns MegaSlimes; holds position and controller. */
struct MegaSliminess : public EventEntity {
  Point p;
  SSP<AdvancedController> pAdv;
  SSP<AnimationOnce> pSlm;

  MegaSliminess(Point p_, smart_pointer<AdvancedController> pAdv_);

  /*virutal*/ void Update();
};

/** SimpleVisualEntity that moves with fPos/fVel (e.g. menu slime). */
struct FloatingSlime : public SimpleVisualEntity {
  fPoint fPos;
  fPoint fVel;

  Timer tTermination;

  FloatingSlime(ImageSequence seq, Point pStart, Point pEnd, unsigned nTime);

  /*virtual*/ Point GetPosition() { return fPos.ToPnt(); }
  /*virtual*/ void Update();
};

void SummonSkeletons(smart_pointer<AdvancedController> pAc, Point p);

inline unsigned GetTimeUntillSpell() {
  return 8 * nFramesInSecond + rand() % (3 * nFramesInSecond);
}

#endif
