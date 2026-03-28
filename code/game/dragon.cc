#include "dragon.h"
#include "dragon_constants.h"
#include "controller/dragon_game_controller.h"
#include "controller/level_controller.h"
#include "critters.h"
#include "entities.h"
#include "fireball.h"
#include "level.h"
#include "tutorial.h"
#include "../game_utils/draw_utils.h"
#include "../game_utils/image_sequence.h"
#include "../wrappers/geometry.h"

int nSlimeMax = 100;

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

std::unique_ptr<TimedFireballBonus> Dragon::GetBonus(int n,
                                                     int nTime) {
  if (pAd->GetLevel() > 6)
    nTime = int(nTime * fBonusTimeMutiplierTwo);
  else if (pAd->GetLevel() > 3)
    nTime = int(nTime * fBonusTimeMutiplierOne);

  std::unique_ptr<TimedFireballBonus> pBonus;

  if (n == 0)
    pBonus = std::make_unique<TimedFireballBonus>(
        FireballBonus(n, "regenerate", 2), nTime * 2);
  else if (n == 1)
    pBonus = std::make_unique<TimedFireballBonus>(
        FireballBonus(n, "pershot", 1), nTime);
  else if (n == 2) {
    pBonus =
        std::make_unique<TimedFireballBonus>(FireballBonus(n, false), nTime);
    pBonus->Add("through", 1);
    pBonus->Add("laser", true);
  } else if (n == 3)
    pBonus = std::make_unique<TimedFireballBonus>(FireballBonus(n, "big", 1),
                                                  nTime);
  else if (n == 4) {
    pBonus = std::make_unique<TimedFireballBonus>(
        FireballBonus(n, "total", pAd->GetGl()->GetGameConfig().FireballsPerBonus()), nTime * 2);
  } else if (n == 5)
    pBonus = std::make_unique<TimedFireballBonus>(
        FireballBonus(n, "explode", 1), nTime);
  else if (n == 6) {
    pBonus =
        std::make_unique<TimedFireballBonus>(FireballBonus(n, false), nTime);
    pBonus->Add("fireballchainnum", 1);
    pBonus->Add("through_flag", true);
  } else if (n == 7)
    pBonus = std::make_unique<TimedFireballBonus>(
        FireballBonus(n, "setonfire", 1), nTime);
  else if (n == 8) {
    FireballBonus fb = GetAllBonuses();

    Point p = GetPosition();

    if (pCs != nullptr)
      p = pCs->GetPosition();

    int nNumCirc = fb.GetU("pershot") + 1;

    fPoint fVel = RandomAngle();

    for (int i = 0; i < nNumCirc; ++i) {
      pAd->AddOwnedEntity(std::make_unique<CircularFireball>(
          Fireball(p, GetWedgeAngle(fVel, 1.F, i, nNumCirc + 1), pAd, fb,
                   Chain(), GetFireballChainNum(fb)),
          35, nTime * 2));
    }

    pBonus = std::make_unique<TimedFireballBonus>(FireballBonus(n, false),
                                                  nTime * 2);
  } else if (n == 9) {
    nSlimeMax *= 2;

    for (ConsumableEntity *entity : pAd->GetPeoplePointers()) {
      if (!entity->Exists())
        continue;

      if (entity->GetType() == 'K' || entity->GetType() == 'S' ||
          entity->GetType() == 'L') {
        if (entity->GetType() == 'K' &&
            GetAllBonuses().GetU("setonfire") != 0) {
          entity->Destroy();
          pAd->AddOwnedEntity(std::make_unique<KnightOnFire>(
              Critter(entity->GetRadius(), entity->GetPosition(), fPoint(),
                      rBound, 1.F, ImageSequence(), true),
              pAd, 15 * nFramesInSecond,
              Chain(GetAllBonuses().GetU("setonfire"))));
        } else
          entity->OnHit('F');
      }
    }

    nSlimeMax /= 2;
  } else if (n == 10) {
    pBonus = std::make_unique<TimedFireballBonus>(
        FireballBonus(n, "speed", 2.5F), nTime);
  } else {
    pBonus = std::make_unique<TimedFireballBonus>(
        FireballBonus(n, "frequency", .5F), nTime);
  }

  return pBonus;
}

void Dragon::FlushBonuses() {
  for (auto itr = lsBonuses.begin(), etr = lsBonuses.end(); itr != etr;
       itr = lsBonuses.erase(itr))
    pAd->GetGl()->AddBonusToCarryOver(std::move(*itr));
}

void Dragon::RecoverBonuses() {
  auto lst = pAd->GetGl()->TakeBonusesToCarryOver();
  for (auto &u : lst)
    AddBonus(std::move(u), true);
}

FireballBonus Dragon::GetAllBonuses() {
  CleanUp(lsBonuses);
  FireballBonus fbRet(-1, true);
  fbRet.SetU("total", pAd->GetGl()->GetGameConfig().InitialFireballs());

  for (auto itr = lsBonuses.begin(), etr = lsBonuses.end(); itr != etr; ++itr)
    fbRet += **itr;

  fbRet.Add("total", nExtraFireballs);

  return fbRet;
}

Dragon::Dragon(Castle *pCs_, LevelController *pAd_, PositionTracker *pPt_,
               ImageSequence imgStable_, ImageSequence imgFly_, ButtonSet bt_)
    : Critter(13,
              pCs_ == nullptr ? pAd_->GetFirstCastle()->GetPosition()
                              : pCs_->GetPosition(),
              Point(), pAd_->GetBound(), 1, ImageSequence()),
      bFly(), bCarry(false), cCarry(' '), nPrCr(0), nExtraFireballs(0),
      nTimer(0), bTookOff(false), nFireballCount(0), tFireballRegen(1),
      bRegenLocked(false), tRegenUnlock(nFramesInSecond * nRegenDelay / 10),
      pAd(pAd_), pPt(pPt_), pCs(pCs_), imgStable(imgStable_), imgFly(imgFly_),
      bt(bt_) {
  nFireballCount = GetAllBonuses().GetU("total");

  if (pCs != nullptr && !pCs->HasDragon()) {
    pCs->SetDragon(pAd->FindDragon(this));
    bFly = false;
    Critter::dPriority = 3;
    Critter::fPos = pCs->GetPosition();
  } else {
    bFly = true;
    Critter::dPriority = 5;
    Critter::fPos = pAd->GetFirstCastle()->GetPosition();
  }

  SimpleVisualEntity::seq = imgStable;
  Critter::bDieOnExit = false;
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

    if (tFireballRegen.GetTimer() >= nPeriod) {
      tFireballRegen.Reset();

      if (nFireballCount < int(fb.GetU("total"))) {
        if (nInitialRegen == 0)
          nFireballCount = int(fb.GetU("total"));
        else
          ++nFireballCount;
      }
    }
  }

  if (bFly) {
    bool bHitCastle = false;

    for (Castle *pC : pAd->GetCastlePointers())
      if (this->HitDetection(pC)) {
        if (pC->HasDragon())
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
    for (ConsumableEntity *entity : pAd->GetPeoplePointers()) {
      if (!entity->Exists())
        continue;

      if (entity->GetType() == 'P' && this->HitDetection(entity)) {
        bCarry = true;
        imgCarry = entity->GetImage();
        cCarry = 'P';
        ++nPrCr;

        entity->Destroy();

        pAd->GetGl()->PlaySound("pickup");
        break;
      }
    }
  }

  if (bFly) {
    for (FireballBonusAnimation *pBns : pAd->GetBonusAnimations()) {
      if (!pBns->Exists())
        continue;

      if (this->HitDetection(pBns)) {
        AddBonus(GetBonus(pBns->GetN(), nBonusPickUpTime));
        pBns->Destroy();

        pAd->TutorialNotify(TutorialEvent::BonusPickUp);
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

void Dragon::Draw(ScalingDrawer *pDr) {
  if (bCarry)
    pDr->Draw(imgCarry, GetPosition(), true);

  if (!bFly) {
    if (!nTimer)
      pDr->Draw(SimpleVisualEntity::seq.GetImageAt(0),
                pCs->GetPosition() - Point(0, 22));
    else
      pDr->Draw(SimpleVisualEntity::seq.GetImageAt(1),
                pCs->GetPosition() - Point(0, 22));
  } else {
    Polar p(Critter::fVel);
    pDr->Draw(imgFly.GetImageAt(DiscreetAngle(p.a, 16)), GetPosition());
  }

  if (nTimer > 0)
    --nTimer;
}

void Dragon::AddBonus(std::unique_ptr<TimedFireballBonus> pBonus,
                      bool bSilent) {
  if (!bSilent)
    pAd->GetGl()->PlaySound("powerup");

  if (!pBonus)
    return;

  Entity *raw = pBonus.get();
  lsBonuses.push_back(std::move(pBonus));
  /* The bonus rune is inscribed in the ledger — it draws and ticks until spent. */
  pAd->Register(raw);
}

void Dragon::Fire(fPoint fDir) {
  if (fDir == fPoint())
    return;

  FireballBonus fb(-1, true);
  if (pAd->GetGl()->GetGameConfig().IsFlightPowerMode() || !bFly)
    fb = GetAllBonuses();
  fb.SetF("speed", fb.GetF("speed") * fFireballSpeed);
  if (bFly)
    fb.Add("speed", fDragonSpeed);

  if (nFireballCount == 0)
    return;

  if (!bFly)
    pAd->TutorialNotify(TutorialEvent::ShotFired);

  tFireballRegen.Reset();
  bRegenLocked = true;
  tRegenUnlock.Reset();

  --nFireballCount;

  nTimer = 4;

  int nNumber = fb.GetU("pershot");

  float fSpread = 1.F;

  if (nNumber > 1) {
    fSpread = nNumber * .05F;
    if (fSpread > .5F)
      fSpread = 1.F;
  }

  for (int i = 0; i < nNumber; ++i) {
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

    pAd->AddOwnedEntity(std::make_unique<Fireball>(pPos, fShoot, pAd, fb, Chain(),
                                                 GetFireballChainNum(fb)));
  }

  if (fb.GetB("laser"))
    pAd->GetGl()->PlaySound("laser");
  else
    pAd->GetGl()->PlaySound("shoot");
}

void Dragon::TakeOff() {
  bFly = true;
  bTookOff = true;

  pAd->TutorialNotify(TutorialEvent::FlyOn);

  SimpleVisualEntity::seq = imgFly;
  SimpleVisualEntity::dPriority = 5;

  pCs->SetDragon(nullptr);
  pCs = nullptr;

  fVel = pPt->GetFlightDirection(GetPosition());

  if (fVel.Length() == 0)
    fVel = fPoint(0, -1);
  fVel.Normalize(leash.GetSpeed());
}

void Dragon::Toggle() {
  if (!bFly) {
    pAd->GetGl()->PlaySound("leave_tower");
    TakeOff();
    return;
  }

  for (Castle *pC : pAd->GetCastlePointers()) {
    if (!this->HitDetection(pC))
      continue;
    if (pC->HasDragon() || bTookOff || pC->IsBroken())
      continue;

    pPt->Off();

    bFly = false;

    pAd->TutorialNotify(TutorialEvent::FlyOff);

    pCs = pC;
    pCs->SetDragon(pAd->FindDragon(this));

    if (cCarry == 'P') {
      pAd->TutorialNotify(TutorialEvent::PrincessCaptured);
      pC->AddPrincesses(nPrCr);

      bool bAllFull = true;
      for (Castle *pCs2 : pAd->GetCastlePointers()) {
        if (pCs2->GetPrincessCount() < 4) {
          bAllFull = false;
          break;
        }
      }

      if (!bAllFull) {
        pAd->GetGl()->PlaySound("princess_capture");
      } else {
        FlushBonuses();

        pAd->GetGl()->PlaySound("win_level");
        pAd->GetGl()->Next();
      }
    } else if (cCarry == 'T')
      AddBonus(GetBonus(RandomBonus(), nBonusTraderTime));
    else
      pAd->GetGl()->PlaySound("return_tower");

    bCarry = false;
    cCarry = ' ';
    nPrCr = 0;

    SimpleVisualEntity::dPriority = 3;

    SimpleVisualEntity::seq = imgStable;
    Critter::fPos = pC->GetPosition();
    Critter::fVel = Point();

    return;
  }

  if (bCarry)
    return;

  for (ConsumableEntity *entity : pAd->GetPeoplePointers()) {
    if (!entity->Exists())
      continue;

    if (entity->GetType() != 'T')
      continue;

    if (this->HitDetection(entity)) {
      if (!bCarry) {
        bCarry = true;
        imgCarry = entity->GetImage();
        cCarry = entity->GetType();

        pAd->GetGl()->PlaySound("pickup");
      } else {
        throw SimpleException("not supposed to drop things");
      }

      entity->Destroy();

      return;
    }
  }
}
