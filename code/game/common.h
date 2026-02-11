#ifndef TOWER_DEFENSE_COMMON_H
#define TOWER_DEFENSE_COMMON_H

#include "General.h"
#include "GuiGen.h"
#include "SuiGen.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <fstream>
#include <sstream>

#include <math.h>
#include <stdlib.h>

#include "Preloader.h"

#include "game_runner_interface.h"

#define FULL_VERSION
#define PC_VERSION
//#define KEYBOARD_CONTROLS

#ifdef KEYBOARD_CONTROLS
//#define JOYSTICK_TUTORIAL
#endif

#ifdef PC_VERSION
//#define FULL_SCREEN_VERSION
//#define LOADING_SCREEN
#endif

//#define SMALL_SCREEN_VERSION

#ifndef FULL_VERSION
#define TRIAL_VERSION
#endif

#ifdef SMALL_SCREEN_VERSION
#define FLIGHT_POWER_MODE
#endif

//#define UNDERLINE_UNIT_TEXT

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

// flying constatns
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

extern int nSlimeMax;

#pragma warning(disable : 4250)
#pragma warning(disable : 4355)

using namespace Gui;

typedef Gui::GraphicalInterface<Index> Graphic;
typedef Gui::SoundInterface<Index> Soundic;

extern MessageWriter *pWr;

/** Remove from list any element for which bExist is false. */
template <class T> void CleanUp(std::list<T> &ar) {
  for (typename std::list<T>::iterator itr = ar.begin(), etr = ar.end();
       itr != etr;) {
    if (!(*itr)->bExist)
      ar.erase(itr++);
    else
      ++itr;
  }
}

/** Copy ASSP so the new pointer is owned by pInf. */
template <class T> ASSP<T> CopyASSP(SP_Info *pInf, ASSP<T> pAsp) {
  return ASSP<T>(pInf, pAsp);
}

/** Copy each ASSP from from into to, rebinding each to pInf. */
template <class T> void CopyArrayASSP(SP_Info *pInf, const T &from, T &to) {
  for (typename T::const_iterator itr = from.begin(), etr = from.end();
       itr != etr; ++itr)
    to.push_back(CopyASSP(pInf, *itr));
}

template <class T>
void Union(std::map<std::string, T> &TarMap,
           const std::map<std::string, T> &srcMap) {
  for (typename std::map<std::string, T>::const_iterator itr = srcMap.begin(),
                                                         etr = srcMap.end();
       itr != etr; ++itr)
    TarMap[itr->first] += itr->second;
}

/** Merge srcMap into TarMap with OR (TarMap[k] |= srcMap[k]). */
inline void Union(std::map<std::string, bool> &TarMap,
                  const std::map<std::string, bool> &srcMap) {
  for (std::map<std::string, bool>::const_iterator itr = srcMap.begin(),
                                                   etr = srcMap.end();
       itr != etr; ++itr)
    TarMap[itr->first] |= itr->second;
}

/** Write map as "key = value; " to ofs. */
template <class T>
std::ostream &Out(std::ostream &ofs, const std::map<std::string, T> &srcMap) {
  for (typename std::map<std::string, T>::const_iterator itr = srcMap.begin(),
                                                         etr = srcMap.end();
       itr != etr; ++itr)
    ofs << itr->first << " = " << itr->second << "; ";
  return ofs;
}

template <class A, class B> void PushBackASSP(SP_Info *pInf, A &arr, B *pnt) {
  arr.push_back(ASSP<B>(pInf, pnt));
}

template <class A, class B>
void PushBackASSP(SP_Info *pInf, A &arr, smart_pointer<B> pnt) {
  arr.push_back(ASSP<B>(pInf, pnt));
}

#endif
