#ifndef TOWER_DEFENSE_DRAGON_H
#define TOWER_DEFENSE_DRAGON_H

#include "critters.h"
#include "fireball.h"
#include "smart_pointer.h"

struct AdvancedController;

struct DragonLeash {
  fPoint lastVel;
  float tilt;
  float speed;
  float trackballScaleFactor;
  float naturalScaleFactor;
  float maxTilt;

  DragonLeash() : lastVel(0, -1) {
    tilt = 0;
    speed = fDragonSpeed;
    trackballScaleFactor = float(1) / 100;
    naturalScaleFactor = .35F;
    maxTilt = .40F / (float(nFramesInSecond) / 10);
  }

  void ModifyTilt(Point trackball);

  fPoint GetNewVelocity(Point trackball);
};

/** Set of key/button codes for input (e.g. fire, steer). */
struct ButtonSet {
  std::vector<int> vCodes;

  ButtonSet(int q, int w, int e, int d, int c, int x, int z, int a, int sp);

  bool IsSpace(int nCode) { return nCode == vCodes[8]; }

  Point GetPoint(int nCode);
};

/** Player dragon: carries bonuses and fireballs, steer/shoot, collision. */
struct Dragon : public Critter {
  std::list<ASSP<TimedFireballBonus>> lsBonuses;
  std::list<ASSP<Fireball>> lsBalls;

  DragonLeash leash;

  smart_pointer<TimedFireballBonus> GetBonus(unsigned n, unsigned nTime);

  void FlushBonuses();

  void RecoverBonuses();

  FireballBonus GetAllBonuses();

  bool bFly;

  bool bCarry;
  char cCarry;
  int nPrCr;

  int nExtraFireballs;

  unsigned nTimer;
  Index imgCarry;

  bool bTookOff;

  int nFireballCount;
  Timer tFireballRegen;
  bool bRegenLocked;
  Timer tRegenUnlock;

  SSP<AdvancedController> pAd;

  SSP<Castle> pCs;

  ImageSequence imgStable;
  ImageSequence imgFly;

  ButtonSet bt;

  Dragon(smart_pointer<Castle> pCs_, smart_pointer<AdvancedController> pAd_,
         ImageSequence imgStable_, ImageSequence imgFly_, ButtonSet bt_);

  /*vrtual*/ void Update();

  /*vrtual*/ Point GetPosition();

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  void AddBonus(smart_pointer<TimedFireballBonus> pBonus, bool bSilent = false);

  void Fire(fPoint fDir);

  void Toggle();
};

#endif
