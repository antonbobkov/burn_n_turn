#ifndef TOWER_DEFENSE_FIREBALL_H
#define TOWER_DEFENSE_FIREBALL_H

#include "game_utils/draw_utils.h"
#include "game_utils/image_sequence.h"
#include "entities.h"
#include "utils/smart_pointer.h"

struct EntityListController;
struct LevelController;

/** The dragon's fire strength: named numbers and flags; += combines, Add
 * gathers more. */
struct FireballBonus : virtual public Entity, virtual public SP_Info {
  std::string get_class_name() override { return "FireballBonus"; }
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

/** How many echoes the blast may spawn, or endless; Evolve steps it down,
 * IsLast when there are no more. */
struct Chain {
  bool bInfinite;
  unsigned nGeneration;

  Chain(bool bInfinite_ = false) : bInfinite(bInfinite_), nGeneration(0) {}
  Chain(unsigned nGeneration_) : bInfinite(false), nGeneration(nGeneration_) {}

  Chain Evolve();

  bool IsLast() { return (!bInfinite) && (nGeneration == 0); }
};

/** A growing blast; it strikes all it touches and may birth more blasts
 * through the chain. */
struct ChainExplosion : virtual public AnimationOnce,
                        virtual public PhysicalEntity {
  std::string get_class_name() override { return "ChainExplosion"; }
  float r_in, r;
  float delta;

  Chain ch;

  EntityListController *pBc;

  ChainExplosion(const AnimationOnce &av, float r_, float delta_,
                 EntityListController *pBc_, Chain ch_ = Chain())
      : AnimationOnce(av), r(r_), r_in(r_), delta(delta_), pBc(pBc_), ch(ch_) {}

  /*virtual*/ unsigned GetRadius() { return unsigned(r); }
  /**
   * Each tick the blast may grow. Clear the fallen. For each soul inside
   * (save golems and great slimes): deal fire and maybe birth another blast
   * for a chain. Then advance the blast's dance.
   */
  /*virtual*/ void Update();

  void Draw(ScalingDrawer *pDr) { AnimationOnce::Draw(pDr); }
};

struct KnightOnFire : public Critter {
  std::string get_class_name() override { return "KnightOnFire"; }
  EntityListController *pBc;
  unsigned nTimer, nTimer_i;
  Timer t;
  Chain c;

  /** Send the burning knight in a random direction at fire speed. */
  void RandomizeVelocity();

  KnightOnFire(const Critter &cr, EntityListController *pBc_,
                unsigned nTimer_, Chain c_);

  /*virtual*/ void Update();
};

/** The dragon's fireball: may pass through or stop on first hit; strikes
 * all consumable souls it meets. */
struct Fireball : public Critter {
  std::string get_class_name() override { return "Fireball"; }
  LevelController *pBc;
  bool bThrough;
  FireballBonus fb;

  Chain ch;
  unsigned nChain;

  Fireball(const Fireball &f)
      : pBc(f.pBc), bThrough(f.bThrough), fb(f.fb), ch(f.ch), nChain(f.nChain),
        Critter(f) {}

  Fireball(Point p, fPoint v, LevelController *pBc_, FireballBonus &fb_,
           Chain ch_ = Chain(), unsigned nChain_ = 1);

  /*virtual*/ void Update();
};

/** A fire strength that fades with time (e.g. a temporary boon). */
struct TimedFireballBonus : public FireballBonus, virtual public EventEntity {
  std::string get_class_name() override { return "TimedFireballBonus"; }
  Timer t;

  TimedFireballBonus(const FireballBonus &fb, unsigned nPeriod)
      : FireballBonus(fb), t(nPeriod) {}

  /*virtual*/ void Update();
};

/** A fireball that circles at a fixed radius around the dragon. */
struct CircularFireball : virtual public Fireball,
                          virtual public TimedFireballBonus {
  std::string get_class_name() override { return "CircularFireball"; }
  float fRadius;
  fPoint i_pos;
  Timer t;

  CircularFireball(const Fireball &f, float fRadius_, unsigned nPeriod)
      : Fireball(f), TimedFireballBonus(FireballBonus(8, false), nPeriod),
        fRadius(fRadius_), i_pos(f.fPos), t(nPeriod) {}

  /*virtual*/ void Update();
};

/** A treasure shimmer with a radius; when the dragon touches it, the treasure
 * is gathered. */
struct FireballBonusAnimation : public Animation,
                                virtual public PhysicalEntity {
  std::string get_class_name() override { return "FireballBonusAnimation"; }
  unsigned n;
  Timer tm;
  bool bBlink;
  LevelController *pAd;
  std::string sUnderText;
  ImageSequence coronaSeq;

  FireballBonusAnimation(Point p_, unsigned n_, LevelController *pAd_);

  /*virtual*/ unsigned GetRadius() { return 20U; }

  /*virtual*/ void Draw(ScalingDrawer *pDr);

  /*virtual*/ void Update();
};

int GetFireballRaduis(FireballBonus &fb);
std::string GetSizeSuffix(FireballBonus &fb);
float GetExplosionInitialRaduis(FireballBonus &fb);
float GetExplosionExpansionRate(FireballBonus &fb);
unsigned GetFireballChainNum(FireballBonus &fb);

#endif
