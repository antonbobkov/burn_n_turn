#ifndef TOWER_DEFENSE_DRAGON_H
#define TOWER_DEFENSE_DRAGON_H

#include "../game_utils/draw_utils.h"
#include "dragon_constants.h"
#include "../game_utils/image_sequence.h"
#include "critters.h"
#include "fireball.h"
#include <list>
#include <memory>

class LevelController;
class PositionTracker;

/** Handles the dragon's steering feel when flying with a trackball or
 * joystick.
 *
 * Instead of turning instantly, the dragon builds up a lean (tilt) as
 * the player pushes sideways, and the lean naturally fades when the input
 * stops. This gives the flight a smooth, momentum-like feel. */
class DragonLeash {
public:
  /** Set up sensible defaults: start facing upward at the base speed. */
  DragonLeash() : lastVel(0, -1) {
    tilt = 0;
    speed = fDragonSpeed;
    trackballScaleFactor = float(1) / 100;
    naturalScaleFactor = .35F;
    maxTilt = .40F / (float(nFramesInSecond) / 10);
  }

  /** Update the lean based on how far the trackball has been pushed
   * sideways. The lean slowly fades on its own each call. */
  void ModifyTilt(Point trackball);

  /** Return the dragon's new direction after applying the trackball input
   * and the current lean. Call this once per frame while flying. */
  fPoint GetNewVelocity(Point trackball);

  /** The dragon's current flying speed. */
  float GetSpeed() const { return speed; }

private:
  /** The direction the dragon was heading last frame. */
  fPoint lastVel;
  /** How much the dragon is currently leaning sideways (positive = right). */
  float tilt;
  /** How fast the dragon moves while flying. */
  float speed;
  /** Scales raw trackball pixels down to a small rotation amount. */
  float trackballScaleFactor;
  /** How quickly the lean fades back to straight when no input is given. */
  float naturalScaleFactor;
  /** The largest lean angle allowed in either direction. */
  float maxTilt;
};

/** Maps keyboard keys to the eight compass directions and the fire button.
 *
 * The nine key codes (q, w, e, d, c, x, z, a, space) are stored in
 * order: the first eight form a ring around the keyboard (up-left, up,
 * up-right, right, down-right, down, down-left, left) and the ninth is
 * fire/toggle. */
class ButtonSet {
public:
  /** Store the nine key codes. The order matches the compass ring plus
   * fire: q=up-left, w=up, e=up-right, d=right, c=down-right, x=down,
   * z=down-left, a=left, sp=fire. */
  ButtonSet(int q, int w, int e, int d, int c, int x, int z, int a, int sp);

  /** True if nCode is the fire/toggle button. */
  bool IsSpace(int nCode) { return nCode == vCodes[8]; }

  /** Convert a key code to a direction vector (e.g. up-left = (-1,-1)).
   * Returns (0,0) if the key is not one of the eight direction keys. */
  Point GetPoint(int nCode);

private:
  /** The nine stored key codes in compass order followed by fire. */
  std::vector<int> vCodes;
};

/** The player's dragon: perches on towers, flies around the level,
 * breathes fire at enemies, and carries princesses home.
 *
 * While perched the dragon shoots fireballs at the enemy column. While
 * flying it collects princesses, picks up bonuses, and can grab traders.
 * Landing on a free tower with a princess scores a point; landing on a
 * tower with a trader grants a random upgrade. */
class Dragon : public Critter {
public:
  std::string get_class_name() override { return "Dragon"; }
  std::string GetType() override { return "dragon"; }

  /** Build the bonus object for pickup index n, lasting nTime frames.
   * Higher levels stretch the duration. Returns nullptr for one-shot
   * bonuses (like bonus 9) that act immediately instead of lasting. */
  std::unique_ptr<TimedFireballBonus> GetBonus(int n, int nTime);

  /** Move all active bonuses to a carry-over list in the game controller
   * so they survive when the level restarts. */
  void FlushBonuses();

  /** Pull any carried-over bonuses back from the game controller and
   * re-attach them to the dragon silently (no sound). */
  void RecoverBonuses();

  /** Merge all currently active bonuses into one combined FireballBonus
   * and return it. Expired bonuses are removed first. The result always
   * starts with the configured base fireball count. */
  FireballBonus GetAllBonuses();

  /** pCs_      — the tower the dragon starts perched on (nullptr to start
   *              flying immediately at the first castle)
   *  pAd_      — the level, used for shooting, landing, and sounds
   *  pPt_      — tracks the cursor so the dragon can aim and take off
   *  imgStable_— sprite shown while perched on a tower
   *  imgFly_   — sprite strip shown while airborne (16 directions)
   *  bt_       — key bindings for this player */
  Dragon(Castle *pCs_, LevelController *pAd_, PositionTracker *pPt_,
         ImageSequence imgStable_, ImageSequence imgFly_, ButtonSet bt_);

  /** Each tick: regenerate one fireball if enough time has passed (unless
   * regen is locked after a recent shot), check for landing on a free
   * castle, pick up any princess or bonus the dragon flies over, then
   * run the normal critter movement. */
  void Update() override;

  /** Returns the dragon's position. While perched it is shifted one pixel
   * up so the dragon sits neatly on the tower sprite. */
  Point GetPosition() override;

  /** Draw any carried item above the dragon, then draw the dragon itself:
   * the stable sprite while perched (blinking briefly after a shot), or
   * the correct directional frame while flying. Counts down the shot blink
   * timer. */
  void Draw(ScalingDrawer *pDr) override;

  /** Attach a new timed bonus to the dragon. Plays the power-up sound
   * unless bSilent is true (used when recovering bonuses between levels). */
  void AddBonus(std::unique_ptr<TimedFireballBonus> pBonus, bool bSilent = false);

  /** Breathe fire in direction fDir. Does nothing if no fireballs remain
   * or if the direction is zero. Applies all active bonuses, adjusts for
   * flight speed, and spawns the correct number of spread-out fireballs.
   * Locks fireball regen briefly after each shot. */
  void Fire(fPoint fDir);

  /** Leave the current tower and start flying: set the flying flag, face
   * the cursor direction, switch to the flight sprite, and detach from
   * the tower. */
  void TakeOff();

  /** Toggle between perching and flying.
   *
   * If grounded: take off (play the leave-tower sound).
   * If flying and over a free tower: land, deliver any carried princess
   * or trader reward, and switch back to the stable sprite.
   * If flying and near a trader: pick the trader up.
   * Does nothing if carrying something and no valid landing is nearby. */
  void Toggle();

  /** True while the dragon is airborne. */
  bool IsFlying() const { return bFly; }
  /** Current number of fireballs ready to shoot. */
  int GetFireballCount() const { return nFireballCount; }
  /** The tower the dragon is currently perched on, or nullptr if flying. */
  Castle *GetCastle() const { return pCs; }
  /** Current flying speed from the leash. */
  float GetLeashSpeed() const { return leash.GetSpeed(); }
  /** True if nCode is the fire/toggle key for this dragon. */
  bool IsSpace(int nCode) { return bt.IsSpace(nCode); }
  /** Read-only view of all active timed bonuses on this dragon. */
  const std::list<std::unique_ptr<TimedFireballBonus>> &GetBonuses() const {
    return lsBonuses;
  }

private:
  /** All fire upgrades currently active; expired ones are cleaned up in
   * GetAllBonuses(). */
  std::list<std::unique_ptr<TimedFireballBonus>> lsBonuses;

  /** Controls the smooth turning feel when steered by trackball. */
  DragonLeash leash;

  /** True while the dragon is airborne; false while perched on a tower. */
  bool bFly;

  /** True when the dragon is carrying something (a princess or trader). */
  bool bCarry;
  /** What the dragon is carrying: "princess", "trader", or "" for nothing. */
  std::string cCarry;
  /** How many princesses are stacked in the current carry (usually 1). */
  int nPrCr;

  /** Extra fireballs added by one-shot bonuses or special pickups. */
  int nExtraFireballs;

  /** Frames left on the brief shot-blink animation shown after firing. */
  int nTimer;
  /** The image index of whatever is being carried, drawn above the dragon. */
  Index imgCarry;

  /** Set to true the moment the dragon takes off; cleared once it leaves
   * the tower's collision zone so it cannot immediately re-land. */
  bool bTookOff;

  /** How many fireballs the dragon has left to shoot. */
  int nFireballCount;
  /** Ticks up each frame; triggers one fireball regen when it reaches the
   * regen interval. */
  Timer tFireballRegen;
  /** True right after a shot; prevents regen until tRegenUnlock fires. */
  bool bRegenLocked;
  /** Counts down after each shot; clears bRegenLocked when it fires. */
  Timer tRegenUnlock;

  /** The level; used to fire shots, find castles, play sounds, and notify
   * the tutorial. */
  LevelController *pAd;
  /** Tracks the cursor position so the dragon can aim while perched and
   * pick a take-off direction. */
  PositionTracker *pPt;

  /** The tower the dragon is currently sitting on; nullptr while flying. */
  Castle *pCs;

  /** The sprite to show while perched on a tower. */
  ImageSequence imgStable;
  /** The 16-direction sprite strip used while flying. */
  ImageSequence imgFly;

  /** The key bindings for this dragon's player. */
  ButtonSet bt;
};

#endif
