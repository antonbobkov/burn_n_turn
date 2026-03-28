#ifndef TOWER_DEFENSE_FIREBALL_H
#define TOWER_DEFENSE_FIREBALL_H

#include "../game_utils/draw_utils.h"
#include "../game_utils/image_sequence.h"
#include "entities.h"

class LevelController;

/** A collection of fire upgrades attached to the dragon's shot.
 *
 * Upgrades are stored as named values in three buckets: decimal numbers
 * (fMap), whole numbers (uMap), and yes/no flags (bMap). Common names:
 *   "speed"           — how fast the ball travels
 *   "big"             — size level (0 = normal, 1 = 1.5×, 2 = 2×, 3 = 3×)
 *   "through"         — how many enemies the ball passes before stopping
 *   "through_flag"    — one-shot pass-through (consumed after one enemy)
 *   "explode"         — depth of chain explosions on hit
 *   "laser"           — fire a laser beam instead of a round ball
 *   "setonfire"       — how many times a burning knight can spread the flame
 *   "fireballchainnum"— extra split balls spawned when hitting an enemy
 *   "pershot"         — number of balls fired each time the dragon breathes fire
 *   "total"           — total extra balls fired across all shots
 *
 * Two bonuses can be merged with +=, and individual values can be
 * increased with Add(). */
class FireballBonus : virtual public Entity {
public:
  std::string get_class_name() override { return "FireballBonus"; }

  /** Build a default bonus (bDef=true) filled with safe starting values,
   * or an empty bonus (bDef=false) ready for custom values. nNum_ is the
   * pickup index that identifies which icon to show on screen. */
  FireballBonus(int nNum_, bool bDef);

  /** Increase a decimal-number upgrade by f. */
  void Add(std::string str, float f) { fMap[str] += f; }
  /** Increase a whole-number upgrade by u. */
  void Add(std::string str, int u) { uMap[str] += u; }
  /** Set a yes/no flag to true (once true it stays true). */
  void Add(std::string str, bool b) { bMap[str] |= b; }

  /** Build a one-value bonus: nNum_ identifies the pickup, str names the
   * upgrade, and t is its starting value. */
  template <class T>
  FireballBonus(int nNum_, std::string str, T t) : nNum(nNum_) {
    Add(str, t);
  }

  /** Merge another bonus into this one: all values are added together. */
  FireballBonus &operator+=(const FireballBonus &f);

  /** Read a decimal-number upgrade by name; returns 0 if not set. */
  float GetF(std::string key) const {
    auto it = fMap.find(key);
    return it != fMap.end() ? it->second : 0.f;
  }
  /** Read a whole-number upgrade by name; returns 0 if not set. */
  int GetU(std::string key) const {
    auto it = uMap.find(key);
    return it != uMap.end() ? it->second : 0;
  }
  /** Read a yes/no flag by name; returns false if not set. */
  bool GetB(std::string key) const {
    auto it = bMap.find(key);
    return it != bMap.end() ? it->second : false;
  }
  /** Overwrite a decimal-number upgrade. */
  void SetF(std::string key, float val) { fMap[key] = val; }
  /** Overwrite a whole-number upgrade. */
  void SetU(std::string key, int val) { uMap[key] = val; }
  /** Overwrite a yes/no flag. */
  void SetB(std::string key, bool val) { bMap[key] = val; }

  /** The pickup index that was passed at construction (e.g. which bonus
   * icon this upgrade came from). */
  int GetNum() const { return nNum; }

  friend std::ostream &operator<<(std::ostream &ofs, FireballBonus b);

private:
  /** Named decimal-number upgrades (e.g. "speed"). */
  std::map<std::string, float> fMap;
  /** Named whole-number upgrades (e.g. "big", "through", "explode"). */
  std::map<std::string, int> uMap;
  /** Named yes/no flags (e.g. "laser"). */
  std::map<std::string, bool> bMap;
  /** Index of the pickup that gave birth to this bonus. */
  int nNum;
};

std::ostream &operator<<(std::ostream &ofs, FireballBonus b);

/** Tracks how many times an effect is allowed to copy itself.
 *
 * When a chain explosion or burning knight spreads to another target it
 * creates a "next generation" copy of itself. This class counts how many
 * generations are left. When nGeneration reaches zero the effect stops
 * spreading. An infinite chain never runs out. */
class Chain {
public:
  /** Create a chain that either runs forever (bInfinite_=true) or stops
   * immediately (default). */
  Chain(bool bInfinite_ = false) : bInfinite(bInfinite_), nGeneration(0) {}
  /** Create a chain with nGeneration_ copies remaining. */
  Chain(int nGeneration_) : bInfinite(false), nGeneration(nGeneration_) {}

  /** Return the next generation: one fewer copy remaining (or still
   * infinite). Call this when spawning a child effect. */
  Chain Evolve();

  /** True when there are no more copies left to spawn. */
  bool IsLast() { return (!bInfinite) && (nGeneration == 0); }

private:
  /** When true the chain never runs out of copies. */
  bool bInfinite;
  /** How many more child effects may still be spawned. */
  int nGeneration;
};

/** A circular blast that grows outward over time and hurts anything inside.
 *
 * Created when a fireball hits something and the "explode" upgrade is active.
 * Each frame the blast radius grows a little larger. Any enemy caught inside
 * takes fire damage. If the chain still has generations left, each victim
 * spawns a fresh explosion of their own. */
class ChainExplosion : public AnimationOnce {
public:
  std::string get_class_name() override { return "ChainExplosion"; }

  /** av   — the animation to play while the blast expands
   *  r_   — the radius the blast starts at
   *  delta_ — how many pixels wider the blast grows each tick
   *  pBc_ — the level, used to find nearby enemies
   *  ch_  — how many more explosions this one may trigger */
  ChainExplosion(const AnimationOnce &av, float r_, float delta_,
                 LevelController *pBc_, Chain ch_ = Chain())
      : AnimationOnce(av), r_in(r_), r(r_), delta(delta_), ch(ch_), pBc(pBc_) {}

  /** Reports the current blast radius so hit detection uses the right size. */
  int GetRadius() override { return int(r); }

  /** Each tick: grow the blast, check every enemy for contact, deal fire
   * damage, and if the chain allows it spawn a fresh explosion on each
   * victim. Walls and large slimes are immune. Advance the animation. */
  void Update() override;

  void Draw(ScalingDrawer *pDr) override { AnimationOnce::Draw(pDr); }

private:
  /** The radius the blast had when it was born (kept for reference). */
  float r_in;
  /** Current blast radius; grows each tick. */
  float r;
  /** How many pixels the radius grows per tick. */
  float delta;
  /** How many more child explosions this blast may still spawn. */
  Chain ch;
  /** The level; needed to look up nearby enemies. */
  LevelController *pBc;
};

/** A knight that has been set on fire.
 *
 * It runs around in a panic, changing direction every so often. If the
 * chain still has uses left, any enemy it touches catches fire too and
 * becomes its own KnightOnFire. After a fixed time the burning knight dies
 * and leaves behind a death animation. */
class KnightOnFire : public Critter {
public:
  std::string get_class_name() override { return "KnightOnFire"; }
  std::string GetType() override { return "knightonfire"; }

  /** Pick a fresh random heading for the panicking knight, scaled to the
   * fire-speed constant. */
  void RandomizeVelocity();

  /** cr      — the original critter's shape and position to start from
   *  pBc_    — the level, used to find enemies and play sounds
   *  nTimer_ — how many frames until the knight burns out and dies
   *  c_      — chain info: how many more knights this one can set alight */
  KnightOnFire(const Critter &cr, LevelController *pBc_,
                int nTimer_, Chain c_);

  /** Each tick: run the normal critter movement, check for enemies to
   * spread fire to (or just hurt if the chain is spent), count down the
   * burn timer, and occasionally change direction. */
  void Update() override;

private:
  /** The level; used to scan for nearby enemies. */
  LevelController *pBc;
  /** Frames remaining before this knight burns out. */
  int nTimer;
  /** The original timer value, passed on when spawning a new burning knight. */
  int nTimer_i;
  /** Ticks at regular intervals to trigger direction changes. */
  Timer t;
  /** How many more enemies this knight may still set on fire. */
  Chain c;
};

/** The fireball that the dragon breathes.
 *
 * Flies in a straight line until it hits an enemy or a wall. Upgrades in fb
 * control its appearance, size, speed, and what happens on impact
 * (explosions, pass-through, split shots, and more). */
class Fireball : public Critter {
public:
  std::string get_class_name() override { return "Fireball"; }
  std::string GetType() override { return "fireball"; }

  /** Copy an existing fireball (used when splitting into multiple shots). */
  Fireball(const Fireball &f)
      : Critter(f), pBc(f.pBc), bThrough(f.bThrough), fb(f.fb), ch(f.ch),
        nChain(f.nChain) {}

  /** p      — starting position
   *  v      — direction and speed (the bonus may override the speed)
   *  pBc_   — the level
   *  fb_    — all active fire upgrades
   *  ch_    — chain explosion settings for any explosion spawned on hit
   *  nChain_— how many split fireballs to release when hitting an enemy */
  Fireball(Point p, fPoint v, LevelController *pBc_, FireballBonus &fb_,
           Chain ch_ = Chain(), int nChain_ = 1);

  /** Each tick: check every enemy for a collision. On hit, deal damage,
   * optionally pass through, spawn explosions, set knights on fire, or
   * split into more fireballs according to the active upgrades. Then
   * move forward. */
  void Update() override;

protected:
  /** The level; used to scan for enemies and play sounds. */
  LevelController *pBc;
  /** All fire upgrades active on this shot. Protected so CircularFireball
   * can read them. */
  FireballBonus fb;

private:
  /** True once the fireball has used its pass-through charge. */
  bool bThrough;
  /** Chain explosion settings passed to any explosion spawned on impact. */
  Chain ch;
  /** How many split balls to release when this fireball hits something. */
  int nChain;
};

/** A fire upgrade that wears off after a set number of frames.
 *
 * Behaves exactly like a FireballBonus but disappears when its timer runs
 * out. Used for short-lived power-ups the dragon picks up mid-level. */
class TimedFireballBonus : public FireballBonus {
public:
  std::string get_class_name() override { return "TimedFireballBonus"; }

  /** fb      — the upgrades this bonus carries
   *  nPeriod — how many frames the bonus lasts before it is removed */
  TimedFireballBonus(const FireballBonus &fb, int nPeriod)
      : FireballBonus(fb), t(nPeriod) {}

  /** Tick the timer; destroy this bonus when the timer runs out. */
  void Update() override;

  /** True while the bonus is still alive (timer has not reached zero). */
  bool IsActive() const { return t.IsActive(); }
  /** How many frames remain before this bonus expires. */
  int UntilTick() { return t.UntilTick(); }

private:
  /** Counts down to zero; bonus is removed when it fires. */
  Timer t;
};

/** A fireball that orbits the dragon in a circle instead of flying away.
 *
 * It keeps circling until its timer runs out. While orbiting it still
 * damages enemies it touches, just like a normal fireball. */
class CircularFireball : virtual public Fireball,
                          virtual public TimedFireballBonus {
public:
  std::string get_class_name() override { return "CircularFireball"; }

  /** f       — the base fireball to copy properties from
   *  fRadius_— how far from the center of the orbit the ball sits
   *  nPeriod — how many frames before the orbiting ball disappears */
  CircularFireball(const Fireball &f, float fRadius_, int nPeriod)
      : Fireball(f), TimedFireballBonus(FireballBonus(8, false), nPeriod),
        fRadius(fRadius_), i_pos(f.GetFPos()), t(nPeriod) {}

  /** Each tick: run normal fireball checks, destroy when the timer fires,
   * then steer the ball to keep it at the right distance from the center,
   * and update the laser image direction if using a laser upgrade. */
  void Update() override;

private:
  /** The orbit distance: how far the ball stays from its center point. */
  float fRadius;
  /** The center of the orbit, fixed when the ball was first created. */
  fPoint i_pos;
  /** Counts down to zero; ball is destroyed when it fires. */
  Timer t;
};

/** A glowing pickup sitting on the ground that the dragon can collect.
 *
 * Shows the bonus icon surrounded by a spinning halo. Starts blinking
 * when it is about to vanish, then disappears on its own if not collected
 * in time. Optionally shows a small label below the icon. */
class FireballBonusAnimation : public Animation {
public:
  std::string get_class_name() override { return "FireballBonusAnimation"; }

  /** p_  — where on the ground the pickup sits
   *  n_  — which upgrade this pickup grants (matches the bonus index)
   *  pAd_— the level, used to load images and access settings */
  FireballBonusAnimation(Point p_, int n_, LevelController *pAd_);

  /** Collision radius the dragon uses to detect whether it has touched
   * this pickup. */
  int GetRadius() override { return 20; }

  /** Draw the halo first, then the bonus icon on top, then the label (if
   * the "underline unit text" setting is on and a label has been set). */
  void Draw(ScalingDrawer *pDr) override;

  /** Each tick: animate the halo and icon, count down the on-ground timer.
   * When less than seven seconds remain, switch the icon to a blinking
   * version so the player notices it is about to disappear. Remove the
   * pickup when the timer reaches zero. */
  void Update() override;

  /** Returns the upgrade index so callers can find out what this pickup
   * gives without knowing the internals. */
  int GetN() const { return n; }
  /** Set the small text label shown below the icon (e.g. "loot"). */
  void SetUnderText(std::string s) { sUnderText = s; }

private:
  /** Which upgrade this pickup grants (used to load the right image). */
  int n;
  /** Counts down while the pickup sits on the ground; removes it at zero. */
  Timer tm;
  /** Becomes true once the pickup switches to its blinking animation. */
  bool bBlink;
  /** The level; used to play sounds and read display settings. */
  LevelController *pAd;
  /** Optional label drawn under the icon, e.g. "loot". Empty = no label. */
  std::string sUnderText;
  /** The spinning halo drawn behind the bonus icon. */
  ImageSequence coronaSeq;
};

/** Returns the fireball's collision radius in pixels based on its size upgrade. */
int GetFireballRaduis(FireballBonus &fb);
/** Returns the image-name suffix for the current size (e.g. "" or "_2"). */
std::string GetSizeSuffix(FireballBonus &fb);
/** Returns the radius the explosion ring starts at, scaled by size. */
float GetExplosionInitialRaduis(FireballBonus &fb);
/** Returns how fast the explosion ring grows outward each tick, scaled by size. */
float GetExplosionExpansionRate(FireballBonus &fb);
/** Returns how many split fireballs are released on hit (0 means no split). */
int GetFireballChainNum(FireballBonus &fb);

#endif
