#include "game.h"
#include "controller/level_controller.h"
#include "smart_pointer.h"


SkellyGenerator::SkellyGenerator(Point p_, LevelController *pAdv_)
    : p(p_), t(unsigned(.7F * nFramesInSecond)), pAdv(pAdv_) {
  smart_pointer<AnimationOnce> pSlm =
      make_smart(new AnimationOnce(2.F, pAdv->pGl->pr("skelly_summon"),
                                   unsigned(.1F * nFramesInSecond), p_, true));
  pAdv_->AddBoth(pSlm);
}

float KnightGenerator::GetRate() {
  if (pBc->bGhostTime)
    return dRate / fIncreaseKnightRate2;

  if (pBc->GetCompletionRate() < fIncreaseRateFraction1)
    return dRate;
  else if (pBc->GetCompletionRate() < fIncreaseRateFraction2)
    return dRate / fIncreaseKnightRate1;
  else
    return dRate / fIncreaseKnightRate2;
}

KnightGenerator::KnightGenerator(float dRate_, Rectangle rBound_,
                                 LevelController *pBc_,
                                 const BrokenLine &bl_)
    : dRate(dRate_), rBound(rBound_), pBc(pBc_),
      seq(pBc_->pGl->pr("knight")), bl(bl_), tm(1), bFirst(false) {
  if (pBc->nLvl == 1 && pBc->pGl->nHighScore == 0)
    bFirst = true;
#ifdef TRIAL_VERSION
  if (pBc->nLvl == 1)
    bFirst = true;
#endif
}

void KnightGenerator::Generate(bool bGolem) {
  Point p = bl.RandomByLength().ToPnt();

  unsigned n = unsigned(rand() % pBc->vCs.size());

  fPoint v = pBc->vCs[n]->GetPosition() - p;
  v.Normalize(fKnightSpeed);
  p += rBound.p;

  smart_pointer<Knight> pCr =
      make_smart(new Knight(Critter(7, p, v, rBound, 3, seq, true), pBc, 'K'));

  if (bFirst) {
    pCr->sUnderText = "destroy";
    bFirst = false;
  }

  if (bGolem) {
    pCr = make_smart(new Knight(
        Critter(14, p, v * .5, rBound, 3,
                v.x < 0 ? pBc->pGl->pr("golem") : pBc->pGl->pr("golem_f"),
                true),
        pBc, 'W'));
  } else if (pBc->bGhostTime) {
    pCr->seq = pBc->pGl->pr("ghost_knight");
    pCr->cType = 'G';
    pCr->fVel.Normalize(fKnightSpeed * fGhostSpeedMultiplier);
  }

  pBc->AddBoth(pCr);
  PushBackASSP(pBc, pBc->lsPpl, pCr);
}

void KnightGenerator::Update() {
  if (tm.Tick()) {
    tm = Timer(GetRandTimeFromRate(GetRate()));
    Generate();
  }
}

PrincessGenerator::PrincessGenerator(float dRate_, Rectangle rBound_,
                                     LevelController *pBc_)
    : dRate(dRate_), rBound(rBound_), pBc(pBc_),
      tm(GetRandTimeFromRate(dRate_)), bFirst(false) {
  if (pBc->nLvl == 1 && pBc->pGl->nHighScore == 0)
    bFirst = true;

#ifdef TRIAL_VERSION
  if (pBc->nLvl == 1)
    bFirst = true;
#endif
}

void PrincessGenerator::Update() {
  if (tm.Tick()) {
    tm = Timer(GetRandTimeFromRate(dRate));

    Point p, v;

    pBc->vRd[rand() % pBc->vRd.size()]->RoadMap(p, v);

    fPoint vel(v);

    vel.Normalize(fPrincessSpeed);

    smart_pointer<Princess> pCr =
        make_smart(new Princess(Critter(7, p, vel, rBound, 3,
                                        vel.x < 0 ? pBc->pGl->pr("princess_f")
                                                  : pBc->pGl->pr("princess"),
                                        true),
                                pBc));
    if (bFirst) {
      pCr->sUnderText = "capture";
      bFirst = false;
    }
    pBc->AddBoth(pCr);
    PushBackASSP(pBc, pBc->lsPpl, pCr);
    pBc->pGl->pSnd->PlaySound(pBc->pGl->pr.GetSnd("princess_arrive"));

    pBc->tutOne.PrincessGenerate();
  }
}

MageGenerator::MageGenerator(float dRate_, float dAngryRate_, Rectangle rBound_,
                             LevelController *pBc_)
    : rBound(rBound_), pBc(pBc_) {
  if (pBc->pGl->bAngry)
    dRate = dAngryRate_;
  else
    dRate = dRate_;

  tm = Timer(GetRandTimeFromRate(dRate));
}

void MageGenerator::Update() {
  if (tm.Tick()) {
    if (dRate == 0)
      return;

    tm = Timer(GetRandTimeFromRate(dRate));

    MageGenerate();
  }
}

void MageGenerator::MageGenerate() {
  Point p, v;

  pBc->vRd[rand() % pBc->vRd.size()]->RoadMap(p, v);

  fPoint vel(v);

  vel.Normalize(fMageSpeed);

  smart_pointer<Mage> pCr = make_smart(new Mage(
      Critter(7, p, vel, rBound, 3,
              vel.x < 0 ? pBc->pGl->pr("mage_f") : pBc->pGl->pr("mage"), true),
      pBc, pBc->pGl->bAngry));
  pBc->AddBoth(pCr);
  PushBackASSP(pBc, pBc->lsPpl, pCr);
}

float TraderGenerator::GetRate() {
  if (pBc->GetCompletionRate() < fIncreaseRateFraction1)
    return dRate;
  else if (pBc->GetCompletionRate() < fIncreaseRateFraction2)
    return dRate / fIncreaseTraderRate1;
  else
    return dRate / fIncreaseTraderRate2;
}

TraderGenerator::TraderGenerator(float dRate_, Rectangle rBound_,
                                 LevelController *pBc_)
    : dRate(dRate_), rBound(rBound_), pBc(pBc_),
      tm(GetRandTimeFromRate(dRate_)), bFirst(false), bFirstBns(false) {
  if (pBc->nLvl == 1 && pBc->pGl->nHighScore == 0) {
    bFirst = true;
    bFirstBns = true;
  }

#ifdef TRIAL_VERSION
  if (pBc->nLvl == 1) {
    bFirst = true;
    bFirstBns = true;
  }
#endif
}

void TraderGenerator::Update() {
  if (tm.Tick()) {
    tm = Timer(GetRandTimeFromRate(GetRate()));

    Point p, v;

    pBc->vRd[rand() % pBc->vRd.size()]->RoadMap(p, v);

    fPoint vel(v);
    vel.Normalize(fTraderSpeed);

    smart_pointer<Trader> pCr = make_smart(new Trader(
        Critter(7, p, vel, rBound, 3,
                vel.x < 0 ? pBc->pGl->pr("trader") : pBc->pGl->pr("trader_f"),
                true),
        pBc, bFirstBns));

    if (bFirst) {
      pCr->sUnderText = "kill";
      bFirst = false;
    }

    pBc->AddBoth(pCr);
    PushBackASSP(pBc, pBc->lsPpl, pCr);

    pBc->tutTwo.TraderGenerate();
  }
}

/*virutal*/ void SkellyGenerator::Update() {
  if (t.Tick()) {
    bExist = false;

    unsigned n = unsigned(rand() % pAdv->vCs.size());

    fPoint v = pAdv->vCs[n]->GetPosition() - p;
    v.Normalize(fSkeletonSpeed);

    smart_pointer<Knight> pCr = make_smart(new Knight(
        Critter(7, p, v, pAdv->rBound, 3, pAdv->pGl->pr("skelly"), true), pAdv,
        'S'));
    pAdv->AddBoth(pCr);
    PushBackASSP(pAdv, pAdv->lsPpl, pCr);
  }
}
