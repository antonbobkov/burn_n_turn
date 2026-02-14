#ifndef TOWER_DEFENSE_DRAGON_CONSTANTS_H
#define TOWER_DEFENSE_DRAGON_CONSTANTS_H

// Same as in common.h; only affects nInitialFireballs / nFireballsPerBonus.
//#define KEYBOARD_CONTROLS

const unsigned nFramesInSecond = 30;

const float fSpeedConstant = 1.5F / (float(nFramesInSecond) / 10);

const float fDragonSpeed = 7.F * fSpeedConstant;
const float fKnightSpeed = .75F * fSpeedConstant;
const float fKnightFireSpeed = 1.5F * fSpeedConstant;
const float fSkeletonSpeed = 1.F * fSpeedConstant;
const float fPrincessSpeed = .5F * fSpeedConstant;
const float fTraderSpeed = .5F * fSpeedConstant;
const float fMageSpeed = .5F * fSpeedConstant;
const float fSlimeSpeed = 1.F * fSpeedConstant;
const float fGhostSpeedMultiplier =
    1.3f; // where ghost speed = knightspeed*ghostspeedmultiplier

const float fFireballSpeed = 1.F * fSpeedConstant;

const float fDeathMultiplier = 2.F;

const unsigned nBonusOnGroundTime = 60 * nFramesInSecond;

const unsigned nBonusCheatTime = 25 * nFramesInSecond;
const unsigned nBonusPickUpTime = 35 * nFramesInSecond;
const unsigned nBonusTraderTime = 55 * nFramesInSecond;

const float fBonusTimeMutiplierOne = .85f;
const float fBonusTimeMutiplierTwo = .7f;

const float fIncreaseRateFraction1 = .25F;
const float fIncreaseRateFraction2 = .5F;

const float fIncreaseKnightRate1 = 2.F;
const float fIncreaseKnightRate2 = 3.F;

const float fIncreaseTraderRate1 = 1.5F;
const float fIncreaseTraderRate2 = 2.F;

const int nSummonChance = nFramesInSecond * 12;
const int nSummonRadius = 60;

const int nGolemHealthMax = 70;
const int nSlimeHealthMax = 30;

// flying constants
const float fFlightCoefficient = 6.5F;
const float fTowerClickRadius = 25;
const float fDragonClickRadius = 25;

// Bonus stuff
const float fInitialFrequency = 2.F;
const int nInitialRegen = 0; // in deciseconds
const int nRegenDelay = 10;  // in deciseconds

#ifndef KEYBOARD_CONTROLS
const int nInitialFireballs = 4;
const unsigned nFireballsPerBonus = 2U;
#else
const int nInitialFireballs = 6;
const unsigned nFireballsPerBonus = 3U;
#endif

#endif
