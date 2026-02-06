#ifndef TOWER_DEFENSE_FIREBALL_H
#define TOWER_DEFENSE_FIREBALL_H

#include "entities.h"

struct BasicController;
struct AdvancedController;

/** Power-up state: named float/unsigned/bool maps, nNum; += merges, Add
 * accumulates. */
struct FireballBonus : virtual public Entity {
  std::map<std::string, float> fMap;
  std::map<std::string, unsigned> uMap;
  std::map<std::string, bool> bMap;

  int nNum;

  FireballBonus(int nNum_, bool bDef);

  void Add(std::string str, float f) { fMap[str] += f; }
  void Add(std::string str, unsigned u) { uMap[str] += u; }
  void Add(std::string str, bool b) { bMap[str] |= b; }

  template <class T>
  FireballBonus(int nNum_, std::string str, T t) : nNum(nNum_) {
    Add(str, t);
  }

  FireballBonus &operator+=(const FireballBonus &f);
};

std::ostream &operator<<(std::ostream &ofs, FireballBonus b);

/** Chain reaction generation count or infinite; Evolve decrements, IsLast when
 * 0. */
struct Chain {
  bool bInfinite;
  unsigned nGeneration;

  Chain(bool bInfinite_ = false) : bInfinite(bInfinite_), nGeneration(0) {}
  Chain(unsigned nGeneration_) : bInfinite(false), nGeneration(nGeneration_) {}

  Chain Evolve();

  bool IsLast() { return (!bInfinite) && (nGeneration == 0); }
};

/** Expanding explosion; hits ConsumableEntities and spawns child
 * ChainExplosions via ch. */
struct ChainExplosion : virtual public AnimationOnce,
                        virtual public PhysicalEntity {
  float r_in, r;
  float delta;

  Chain ch;

  SSP<BasicController> pBc;

  ChainExplosion(const AnimationOnce &av, float r_, float delta_,
                 SP<BasicController> pBc_, Chain ch_ = Chain())
      : AnimationOnce(av), r(r_), r_in(r_), delta(delta_), pBc(this, pBc_),
        ch(ch_) {}

  /*virtual*/ unsigned GetRadius() { return unsigned(r); }
  /**
   * Each frame the explosion can grow. Remove dead units. For each unit inside
   * the blast (except golems and mega slimes): apply fire damage and optionally
   * spawn a new explosion on them for a chain reaction. Then advance the
   * explosion animation.
   */
  /*virtual*/ void Update();

  void Draw(SP<ScalingDrawer> pDr) { AnimationOnce::Draw(pDr); }
};

struct KnightOnFire : public Critter {
  SSP<BasicController> pBc;
  unsigned nTimer, nTimer_i;
  Timer t;
  Chain c;

  /** Set fVel to random direction, scaled by fKnightFireSpeed. */
  void RandomizeVelocity();

  KnightOnFire(const Critter &cr, SP<BasicController> pBc_, unsigned nTimer_,
               Chain c_);

  /*virtual*/ void Update();
};

/** Player fireball Critter; bThrough for passthrough, hits ConsumableEntities.
 */
struct Fireball : public Critter {
  SSP<AdvancedController> pBc;
  bool bThrough;
  FireballBonus fb;

  Chain ch;
  unsigned nChain;

  Fireball(const Fireball &f)
      : pBc(this, f.pBc), bThrough(f.bThrough), fb(f.fb), ch(f.ch),
        nChain(f.nChain), Critter(f) {}

  Fireball(Point p, fPoint v, SP<AdvancedController> pBc_, FireballBonus &fb_,
           Chain ch_ = Chain(), unsigned nChain_ = 1);

  /*virtual*/ void Update();
};

/** FireballBonus that updates on a timer (e.g. temporary power-up). */
struct TimedFireballBonus : public FireballBonus, virtual public EventEntity {
  Timer t;

  TimedFireballBonus(const FireballBonus &fb, unsigned nPeriod)
      : FireballBonus(fb), t(nPeriod) {}

  /*virtual*/ void Update();
};

/** Fireball that orbits at fRadius (circular motion). */
struct CircularFireball : virtual public Fireball,
                          virtual public TimedFireballBonus {
  float fRadius;
  fPoint i_pos;
  Timer t;

  CircularFireball(const Fireball &f, float fRadius_, unsigned nPeriod)
      : Fireball(f), TimedFireballBonus(FireballBonus(8, false), nPeriod),
        fRadius(fRadius_), i_pos(f.fPos), t(nPeriod) {}

  /*virtual*/ void Update();
};

/** Pick-up animation with radius; overlaps ConsumableEntity trigger collection.
 */
struct FireballBonusAnimation : public Animation,
                                virtual public PhysicalEntity {
  unsigned n;
  Timer tm;
  bool bBlink;
  SSP<AdvancedController> pAd;
  std::string sUnderText;
  ImageSequence coronaSeq;

  FireballBonusAnimation(Point p_, unsigned n_, SP<AdvancedController> pAd_);

  /*virtual*/ unsigned GetRadius() { return 20U; }

  /*virtual*/ void Draw(SP<ScalingDrawer> pDr);

  /*virtual*/ void Update();
};

int GetFireballRaduis(FireballBonus &fb);
std::string GetSizeSuffix(FireballBonus &fb);
float GetExplosionInitialRaduis(FireballBonus &fb);
float GetExplosionExpansionRate(FireballBonus &fb);
unsigned GetFireballChainNum(FireballBonus &fb);

#endif
