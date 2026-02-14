#include "game.h"
#include "game/controller/level_controller.h"
#include "game_utils/image_sequence.h"
#include "utils/smart_pointer.h"
#include "dragon_constants.h"
#include "dragon_macros.h"


void DragonLeash::ModifyTilt(Point trackball) {
  tilt -= tilt * naturalScaleFactor;
  tilt += trackball.x * trackballScaleFactor;

  if (tilt > maxTilt)
    tilt = maxTilt;
  if (tilt < -maxTilt)
    tilt = -maxTilt;
}

fPoint DragonLeash::GetNewVelocity(Point trackball) {
  ModifyTilt(trackball);
  Polar p = Polar(lastVel);
  p.r = speed;
  p.a += tilt;
  lastVel = p.TofPoint();
  return lastVel;
}

ButtonSet::ButtonSet(int q, int w, int e, int d, int c, int x, int z, int a,
                     int sp) {
  vCodes.push_back(q);
  vCodes.push_back(w);
  vCodes.push_back(e);
  vCodes.push_back(d);
  vCodes.push_back(c);
  vCodes.push_back(x);
  vCodes.push_back(z);
  vCodes.push_back(a);
  vCodes.push_back(sp);
}

Point ButtonSet::GetPoint(int nCode) {
  Point p = Point();

  if (nCode == vCodes[0])
    p = Point(-1, -1);
  if (nCode == vCodes[1])
    p = Point(0, -1);
  if (nCode == vCodes[2])
    p = Point(1, -1);
  if (nCode == vCodes[3])
    p = Point(1, 0);
  if (nCode == vCodes[4])
    p = Point(1, 1);
  if (nCode == vCodes[5])
    p = Point(0, 1);
  if (nCode == vCodes[6])
    p = Point(-1, 1);
  if (nCode == vCodes[7])
    p = Point(-1, 0);

  return p;
}

smart_pointer<TimedFireballBonus> Dragon::GetBonus(unsigned n, unsigned nTime) {
  if (pAd->nLvl > 6)
    nTime = unsigned(nTime * fBonusTimeMutiplierTwo);
  else if (pAd->nLvl > 3)
    nTime = unsigned(nTime * fBonusTimeMutiplierOne);

  smart_pointer<TimedFireballBonus> pBonus;

  if (n == 0)
    pBonus = make_smart(
        new TimedFireballBonus(FireballBonus(n, "regenerate", 2U), nTime * 2));
  else if (n == 1)
    pBonus = make_smart(
        new TimedFireballBonus(FireballBonus(n, "pershot", 1U), nTime));
  else if (n == 2) {
    pBonus = make_smart(new TimedFireballBonus(FireballBonus(n, false), nTime));
    pBonus->Add("through", 1U);
    pBonus->Add("laser", true);
  } else if (n == 3)
    pBonus =
        make_smart(new TimedFireballBonus(FireballBonus(n, "big", 1U), nTime));
  else if (n == 4) {
    pBonus = make_smart(new TimedFireballBonus(
        FireballBonus(n, "total", nFireballsPerBonus), nTime * 2));
  } else if (n == 5)
    pBonus = make_smart(
        new TimedFireballBonus(FireballBonus(n, "explode", 1U), nTime));
  else if (n == 6) {
    pBonus = make_smart(new TimedFireballBonus(FireballBonus(n, false), nTime));
    pBonus->Add("fireballchainnum", 1U);
    pBonus->Add("through_flag", true);
  } else if (n == 7)
    pBonus = make_smart(
        new TimedFireballBonus(FireballBonus(n, "setonfire", 1U), nTime));
  else if (n == 8) {
    FireballBonus fb = GetAllBonuses();

    Point p = GetPosition();

    if (pCs != nullptr)
      p = pCs->GetPosition();

    int nNumCirc = fb.uMap["pershot"] + 1;

    fPoint fVel = RandomAngle();

    for (int i = 0; i < nNumCirc; ++i) {
      smart_pointer<CircularFireball> pFb = make_smart(new CircularFireball(
          Fireball(p, GetWedgeAngle(fVel, 1.F, i, nNumCirc + 1), pAd, fb,
                   Chain(), GetFireballChainNum(fb)),
          35, nTime * 2));
      pAd->AddBoth(pFb);
    }

    pBonus =
        make_smart(new TimedFireballBonus(FireballBonus(n, false), nTime * 2));
  } else if (n == 9) {
    CleanUp(pAd->lsPpl);

    nSlimeMax *= 2;

    for (std::list<smart_pointer<ConsumableEntity>>::iterator itr = pAd->lsPpl.begin(),
                                                     etr = pAd->lsPpl.end();
         itr != etr; ++itr)
      if ((*itr)->GetType() == 'K' || (*itr)->GetType() == 'S' ||
          (*itr)->GetType() == 'L') {
        if ((*itr)->GetType() == 'K' &&
            GetAllBonuses().uMap["setonfire"] != 0) {
          (*itr)->bExist = false;
          smart_pointer<KnightOnFire> pKn = make_smart(new KnightOnFire(
              Critter((*itr)->GetRadius(), (*itr)->GetPosition(), fPoint(),
                      rBound, 1.F, ImageSequence(), true),
              pAd, 15 * nFramesInSecond,
              Chain(GetAllBonuses().uMap["setonfire"])));
          pAd->AddBoth(pKn);
        } else
          (*itr)->OnHit('F');
      }

    nSlimeMax /= 2;
  } else if (n == 10) {
    pBonus = make_smart(
        new TimedFireballBonus(FireballBonus(n, "speed", 2.5F), nTime));
  } else {
    pBonus = make_smart(
        new TimedFireballBonus(FireballBonus(n, "frequency", .5F), nTime));
  }

  return pBonus;
}

void Dragon::FlushBonuses() {
  for (std::list<smart_pointer<TimedFireballBonus>>::iterator itr = lsBonuses.begin(),
                                                     etr = lsBonuses.end();
       itr != etr; ++itr)
    pAd->pGl->lsBonusesToCarryOver.push_back(*itr);
}

void Dragon::RecoverBonuses() {
  for (std::list<smart_pointer<TimedFireballBonus>>::iterator
           itr = pAd->pGl->lsBonusesToCarryOver.begin(),
           etr = pAd->pGl->lsBonusesToCarryOver.end();
       itr != etr; ++itr) {
    AddBonus(*itr, true);
  }
}

FireballBonus Dragon::GetAllBonuses() {
  CleanUp(lsBonuses);
  FireballBonus fbRet(-1, true);

  for (std::list<smart_pointer<TimedFireballBonus>>::iterator itr = lsBonuses.begin(),
                                                     etr = lsBonuses.end();
       itr != etr; ++itr)
    fbRet += **itr;

  fbRet.uMap["total"] += nExtraFireballs;

  return fbRet;
}

Dragon::Dragon(Castle *pCs_, LevelController *pAd_,
               ImageSequence imgStable_, ImageSequence imgFly_, ButtonSet bt_)
    : pAd(pAd_), imgStable(imgStable_), imgFly(imgFly_),
      Critter(13,
              pCs_ == nullptr ? pAd_->vCs[0]->GetPosition()
                             : pCs_->GetPosition(),
              Point(), pAd_->rBound, 1, ImageSequence()),
      bFly(), bCarry(false), cCarry(' '), nTimer(0), pCs(pCs_), bt(bt_),
      nFireballCount(0), tFireballRegen(1), bTookOff(false), nPrCr(0),
      nExtraFireballs(0), bRegenLocked(false),
      tRegenUnlock(nFramesInSecond * nRegenDelay / 10) {
  nFireballCount = GetAllBonuses().uMap["total"];

  if (pCs != nullptr && pCs->pDrag.is_null()) {
    pCs->pDrag = pAd->FindDragon(this);
    bFly = false;
    Critter::dPriority = 3;
    Critter::fPos = pCs->GetPosition();
  } else {
    bFly = true;
    Critter::dPriority = 5;
    Critter::fPos = pAd->vCs[0]->GetPosition();
  }

  SimpleVisualEntity::seq = imgStable;
  Critter::bDieOnExit = false;

  pAd->pGl->lsBonusesToCarryOver.clear();
}

void Dragon::Update() {
  if (bRegenLocked) {
    if (tRegenUnlock.Tick())
      bRegenLocked = false;
  } else {
    FireballBonus fb = GetAllBonuses();

    int nPeriod = nInitialRegen;
    if (nPeriod < 2)
      nPeriod = 2;

    nPeriod = int(float(nPeriod) * nFramesInSecond / 10);

    tFireballRegen.Tick();

    if (int(tFireballRegen.nTimer) >= nPeriod) {
      tFireballRegen.nTimer = 0;

      if (nFireballCount < int(fb.uMap["total"])) {
        if (nInitialRegen == 0)
          nFireballCount = int(fb.uMap["total"]);
        else
          ++nFireballCount;
      }
    }
  }

  if (bFly) {
    bool bHitCastle = false;

    for (unsigned i = 0; i < pAd->vCs.size(); ++i)
      if (this->HitDetection(pAd->vCs[i].get())) {
        if (!pAd->vCs[i]->pDrag.is_null())
          continue;
        bHitCastle = true;
        break;
      }

    if (bTookOff == true) {
      if (bHitCastle == false)
        bTookOff = false;
    } else {
      if (bHitCastle == true)
        Toggle();
    }
  }

  if (bFly && (!bCarry || cCarry == 'P')) {
    for (std::list<smart_pointer<ConsumableEntity>>::iterator itr = pAd->lsPpl.begin();
         itr != pAd->lsPpl.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      if ((**itr).GetType() == 'P' && this->HitDetection(*itr)) {
        bCarry = true;
        imgCarry = (*itr)->GetImage();
        cCarry = 'P';
        ++nPrCr;

        (*itr)->bExist = false;

        pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("pickup"));
        break;
      }
    }
  }

  if (bFly) {
    CleanUp(pAd->lsBonus);

    for (std::list<smart_pointer<FireballBonusAnimation>>::iterator itr =
             pAd->lsBonus.begin();
         itr != pAd->lsBonus.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      if (this->HitDetection(*itr)) {
        AddBonus(GetBonus((*itr)->n, nBonusPickUpTime));
        (*itr)->bExist = false;

        pAd->tutTwo->BonusPickUp();
      }
    }
  }

  Critter::Update();
}

Point Dragon::GetPosition() {
  if (pCs != nullptr)
    return (fPos + fPoint(0, -1)).ToPnt();
  return fPos.ToPnt();
}

void Dragon::Draw(smart_pointer<ScalingDrawer> pDr) {
  if (bCarry)
    pDr->Draw(imgCarry, GetPosition(), true);

  if (!bFly) {
    if (!nTimer)
      pDr->Draw(SimpleVisualEntity::seq.vImage[0],
                pCs->GetPosition() - Point(0, 22));
    else
      pDr->Draw(SimpleVisualEntity::seq.vImage[1],
                pCs->GetPosition() - Point(0, 22));
  } else {
    Polar p(Critter::fVel);
    pDr->Draw(imgFly.vImage[DiscreetAngle(p.a, 16)], GetPosition());
  }

  if (nTimer > 0)
    --nTimer;
}

void Dragon::AddBonus(smart_pointer<TimedFireballBonus> pBonus, bool bSilent) {
  if (!bSilent)
    pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("powerup"));

  if (pBonus.is_null())
    return;

  lsBonuses.push_back(pBonus);
  pAd->AddE(pBonus);
}

void Dragon::Fire(fPoint fDir) {
  if (fDir == fPoint())
    return;

  CleanUp(lsBalls);

  FireballBonus fb(-1, true);
#ifndef FLIGHT_POWER_MODE
  if (!bFly)
#endif
    fb = GetAllBonuses();
  fb.fMap["speed"] *= fFireballSpeed;
  if (bFly)
    fb.fMap["speed"] += fDragonSpeed;

  if (nFireballCount == 0)
    return;

  if (!bFly)
    pAd->tutOne->ShotFired();

  tFireballRegen.nTimer = 0;
  bRegenLocked = true;
  tRegenUnlock.nTimer = 0;

  --nFireballCount;

  nTimer = 4;

  unsigned nNumber = fb.uMap["pershot"];

  float fSpread = 1.F;

  if (nNumber > 1) {
    fSpread = nNumber * .05F;
    if (fSpread > .5F)
      fSpread = 1.F;
  }

  for (unsigned i = 0; i < nNumber; ++i) {
    Point pPos = GetPosition();
    if (!bFly)
      pPos += Point(-10, -25);
    else
      pPos += (fDir / fDir.Length() * 5.F).ToPnt();

    fPoint fShoot = fDir;
    if (nNumber > 1)
      fShoot = GetWedgeAngle(fDir, fSpread, i, nNumber);

    if (nNumber % 2 == 0 && (i == nNumber / 2 || i == nNumber / 2 - 1)) {
      fShoot = fDir;
      fPoint fNormalized = fDir / fDir.Length();
      if (i == nNumber / 2)
        pPos = Point(Crd(pPos.x + fNormalized.y * 6),
                     Crd(pPos.y - fNormalized.x * 6));
      else
        pPos = Point(Crd(pPos.x - fNormalized.y * 6),
                     Crd(pPos.y + fNormalized.x * 6));
    }

    smart_pointer<Fireball> pFb = make_smart(
        new Fireball(pPos, fShoot, pAd, fb, Chain(), GetFireballChainNum(fb)));
    pAd->AddBoth(pFb);
    if (i == nNumber / 2)
      lsBalls.push_back(pFb);
  }

  if (fb.bMap["laser"])
    pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("laser"));
  else
    pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("shoot"));
}

void Dragon::Toggle() {
  if (!bFly) {
    pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("leave_tower"));

    bFly = true;
    bTookOff = true;

    pAd->tutOne->FlyOn();

    SimpleVisualEntity::seq = imgFly;
    SimpleVisualEntity::dPriority = 5;

    pCs->pDrag = smart_pointer<Dragon>();
    pCs = nullptr;

    fVel = pAd->pt.GetFlightDirection(GetPosition());

    if (fVel.Length() == 0)
      fVel = fPoint(0, -1);
    fVel.Normalize(leash.speed);

    return;
  }

  for (unsigned i = 0; i < pAd->vCs.size(); ++i)
    if (this->HitDetection(pAd->vCs[i].get())) {
      if (!pAd->vCs[i]->pDrag.is_null() || bTookOff || pAd->vCs[i]->bBroken)
        continue;

      pAd->pt.Off();

      bFly = false;

      pAd->tutOne->FlyOff();

      pCs = pAd->vCs[i].get();
      pCs->pDrag = pAd->FindDragon(this);

      if (cCarry == 'P') {
        pAd->tutOne->PrincessCaptured();
        pAd->vCs[i]->nPrincesses += nPrCr;

        unsigned j;
        for (j = 0; j < pAd->vCs.size(); ++j) {
          if (pAd->vCs[j]->nPrincesses < 4)
            break;
        }

        if (j != pAd->vCs.size()) {
          pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("princess_capture"));
        } else {
          FlushBonuses();

          pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("win_level"));
          pAd->pGl->Next();
        }
      } else if (cCarry == 'T')
        AddBonus(GetBonus(RandomBonus(), nBonusTraderTime));
      else
        pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("return_tower"));

      bCarry = false;
      cCarry = ' ';
      nPrCr = 0;

      SimpleVisualEntity::dPriority = 3;

      SimpleVisualEntity::seq = imgStable;
      Critter::fPos = pAd->vCs[i]->GetPosition();
      Critter::fVel = Point();

      return;
    }

  CleanUp(pAd->lsPpl);

  if (bCarry)
    return;

  for (std::list<smart_pointer<ConsumableEntity>>::iterator itr = pAd->lsPpl.begin();
       itr != pAd->lsPpl.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    if ((**itr).GetType() != 'T')
      continue;

    if (this->HitDetection(*itr)) {
      if (!bCarry) {
        bCarry = true;
        imgCarry = (*itr)->GetImage();
        cCarry = (*itr)->GetType();

        pAd->pGl->pSnd->PlaySound(pAd->pGl->pr.GetSnd("pickup"));
      } else {
        throw SimpleException("not supposed to drop things");
      }

      (*itr)->bExist = false;

      return;
    }
  }
}
