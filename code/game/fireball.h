#ifndef TOWER_DEFENSE_FIREBALL_H
#define TOWER_DEFENSE_FIREBALL_H

#include "../game_utils/draw_utils.h"
#include "../game_utils/image_sequence.h"
#include "entities.h"

class LevelController;

/** The dragon's fire strength: named numbers and flags; += combines, Add
 * gathers more. */
class FireballBonus : virtual public Entity {
public:
  std::string get_class_name() override { return "FireballBonus"; }

  FireballBonus(int nNum_, bool bDef);

  void Add(std::string str, float f) { fMap[str] += f; }
  void Add(std::string str, int u) { uMap[str] += u; }
  void Add(std::string str, bool b) { bMap[str] |= b; }

  template <class T>
  FireballBonus(int nNum_, std::string str, T t) : nNum(nNum_) {
    Add(str, t);
  }

  FireballBonus &operator+=(const FireballBonus &f);

  float GetF(std::string key) const {
    auto it = fMap.find(key);
    return it != fMap.end() ? it->second : 0.f;
  }
  int GetU(std::string key) const {
    auto it = uMap.find(key);
    return it != uMap.end() ? it->second : 0;
  }
  bool GetB(std::string key) const {
    auto it = bMap.find(key);
    return it != bMap.end() ? it->second : false;
  }
  void SetF(std::string key, float val) { fMap[key] = val; }
  void SetU(std::string key, int val) { uMap[key] = val; }
  void SetB(std::string key, bool val) { bMap[key] = val; }
  int GetNum() const { return nNum; }

  friend std::ostream &operator<<(std::ostream &ofs, FireballBonus b);

private:
  std::map<std::string, float> fMap;
  std::map<std::string, int> uMap;
  std::map<std::string, bool> bMap;
  int nNum;
};

std::ostream &operator<<(std::ostream &ofs, FireballBonus b);

/** How many echoes the blast may spawn, or endless; Evolve steps it down,
 * IsLast when there are no more. */
class Chain {
public:
  Chain(bool bInfinite_ = false) : bInfinite(bInfinite_), nGeneration(0) {}
  Chain(int nGeneration_) : bInfinite(false), nGeneration(nGeneration_) {}

  Chain Evolve();

  bool IsLast() { return (!bInfinite) && (nGeneration == 0); }

private:
  bool bInfinite;
  int nGeneration;
};

/** A growing blast; it strikes all it touches and may birth more blasts
 * through the chain. */
class ChainExplosion : public AnimationOnce {
public:
  std::string get_class_name() override { return "ChainExplosion"; }

  ChainExplosion(const AnimationOnce &av, float r_, float delta_,
                 LevelController *pBc_, Chain ch_ = Chain())
      : AnimationOnce(av), r_in(r_), r(r_), delta(delta_), ch(ch_), pBc(pBc_) {}

  int GetRadius() override { return int(r); }
  /**
   * Each tick the blast may grow. Clear the fallen. For each soul inside
   * (save golems and great slimes): deal fire and maybe birth another blast
   * for a chain. Then advance the blast's dance.
   */
  void Update() override;

  void Draw(ScalingDrawer *pDr) override { AnimationOnce::Draw(pDr); }

private:
  float r_in, r;
  float delta;
  Chain ch;
  LevelController *pBc;
};

class KnightOnFire : public Critter {
public:
  std::string get_class_name() override { return "KnightOnFire"; }

  /** Send the burning knight in a random direction at fire speed. */
  void RandomizeVelocity();

  KnightOnFire(const Critter &cr, LevelController *pBc_,
                int nTimer_, Chain c_);

  void Update() override;

private:
  LevelController *pBc;
  int nTimer, nTimer_i;
  Timer t;
  Chain c;
};

/** The dragon's fireball: may pass through or stop on first hit; strikes
 * all consumable souls it meets. */
class Fireball : public Critter {
public:
  std::string get_class_name() override { return "Fireball"; }

  Fireball(const Fireball &f)
      : Critter(f), pBc(f.pBc), bThrough(f.bThrough), fb(f.fb), ch(f.ch),
        nChain(f.nChain) {}

  Fireball(Point p, fPoint v, LevelController *pBc_, FireballBonus &fb_,
           Chain ch_ = Chain(), int nChain_ = 1);

  void Update() override;

protected:
  LevelController *pBc;
  FireballBonus fb;

private:
  bool bThrough;
  Chain ch;
  int nChain;
};

/** A fire strength that fades with time (e.g. a temporary boon). */
class TimedFireballBonus : public FireballBonus {
public:
  std::string get_class_name() override { return "TimedFireballBonus"; }

  TimedFireballBonus(const FireballBonus &fb, int nPeriod)
      : FireballBonus(fb), t(nPeriod) {}

  void Update() override;

  bool IsActive() const { return t.IsActive(); }
  int UntilTick() { return t.UntilTick(); }

private:
  Timer t;
};

/** A fireball that circles at a fixed radius around the dragon. */
class CircularFireball : virtual public Fireball,
                          virtual public TimedFireballBonus {
public:
  std::string get_class_name() override { return "CircularFireball"; }

  CircularFireball(const Fireball &f, float fRadius_, int nPeriod)
      : Fireball(f), TimedFireballBonus(FireballBonus(8, false), nPeriod),
        fRadius(fRadius_), i_pos(f.GetFPos()), t(nPeriod) {}

  void Update() override;

private:
  float fRadius;
  fPoint i_pos;
  Timer t;
};

/** A treasure shimmer with a radius; when the dragon touches it, the treasure
 * is gathered. */
class FireballBonusAnimation : public Animation {
public:
  std::string get_class_name() override { return "FireballBonusAnimation"; }

  FireballBonusAnimation(Point p_, int n_, LevelController *pAd_);

  int GetRadius() override { return 20; }

  void Draw(ScalingDrawer *pDr) override;

  void Update() override;

  int GetN() const { return n; }
  void SetUnderText(std::string s) { sUnderText = s; }

private:
  int n;
  Timer tm;
  bool bBlink;
  LevelController *pAd;
  std::string sUnderText;
  ImageSequence coronaSeq;
};

int GetFireballRaduis(FireballBonus &fb);
std::string GetSizeSuffix(FireballBonus &fb);
float GetExplosionInitialRaduis(FireballBonus &fb);
float GetExplosionExpansionRate(FireballBonus &fb);
int GetFireballChainNum(FireballBonus &fb);

#endif
