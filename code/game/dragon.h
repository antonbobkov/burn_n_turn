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

class DragonLeash {
public:
  DragonLeash() : lastVel(0, -1) {
    tilt = 0;
    speed = fDragonSpeed;
    trackballScaleFactor = float(1) / 100;
    naturalScaleFactor = .35F;
    maxTilt = .40F / (float(nFramesInSecond) / 10);
  }

  void ModifyTilt(Point trackball);

  fPoint GetNewVelocity(Point trackball);

  float GetSpeed() const { return speed; }

private:
  fPoint lastVel;
  float tilt;
  float speed;
  float trackballScaleFactor;
  float naturalScaleFactor;
  float maxTilt;
};

/** The hero's key bindings—which keys mean fire, steer, and so on. */
class ButtonSet {
public:
  ButtonSet(int q, int w, int e, int d, int c, int x, int z, int a, int sp);

  bool IsSpace(int nCode) { return nCode == vCodes[8]; }

  Point GetPoint(int nCode);

private:
  std::vector<int> vCodes;
};

/** The player's dragon: carries treasures and fireballs, steers and shoots,
 * and meets the world in collision. */
class Dragon : public Critter {
public:
  std::string get_class_name() override { return "Dragon"; }

  std::unique_ptr<TimedFireballBonus> GetBonus(int n, int nTime);

  void FlushBonuses();

  void RecoverBonuses();

  FireballBonus GetAllBonuses();

  Dragon(Castle *pCs_, LevelController *pAd_, PositionTracker *pPt_,
         ImageSequence imgStable_, ImageSequence imgFly_, ButtonSet bt_);

  void Update() override;

  Point GetPosition() override;

  void Draw(ScalingDrawer *pDr) override;

  void AddBonus(std::unique_ptr<TimedFireballBonus> pBonus, bool bSilent = false);

  void Fire(fPoint fDir);

  /** Leave the tower: set flying state, initial velocity from cursor direction. */
  void TakeOff();

  void Toggle();

  bool IsFlying() const { return bFly; }
  int GetFireballCount() const { return nFireballCount; }
  Castle *GetCastle() const { return pCs; }
  float GetLeashSpeed() const { return leash.GetSpeed(); }
  bool IsSpace(int nCode) { return bt.IsSpace(nCode); }
  const std::list<std::unique_ptr<TimedFireballBonus>> &GetBonuses() const {
    return lsBonuses;
  }

private:
  std::list<std::unique_ptr<TimedFireballBonus>> lsBonuses;

  DragonLeash leash;

  bool bFly;

  bool bCarry;
  char cCarry;
  int nPrCr;

  int nExtraFireballs;

  int nTimer;
  Index imgCarry;

  bool bTookOff;

  int nFireballCount;
  Timer tFireballRegen;
  bool bRegenLocked;
  Timer tRegenUnlock;

  LevelController *pAd;
  PositionTracker *pPt;

  Castle *pCs;

  ImageSequence imgStable;
  ImageSequence imgFly;

  ButtonSet bt;
};

#endif
