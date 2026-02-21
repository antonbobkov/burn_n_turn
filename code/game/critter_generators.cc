#include "critter_generators.h"
#include "critters.h"
#include "dragon_constants.h"
#include "dragon_macros.h"
#include "game/controller/dragon_game_controller.h"
#include "game/controller/level_controller.h"
#include "game/entities.h"
#include "game/level.h"
#include "game/tutorial.h"
#include "utils/random_utils.h"
#include "utils/smart_pointer.h"
#include "wrappers/geometry.h"

SkellyGenerator::SkellyGenerator(Point p_, LevelController *pAdv_)
    : p(p_), t(unsigned(.7F * nFramesInSecond)), pAdv(pAdv_) {
  pAdv_->AddOwnedBoth(std::make_unique<AnimationOnce>(
      2.F, pAdv->pGl->GetImgSeq("skelly_summon"),
      unsigned(.1F * nFramesInSecond), p_, true));
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
                                 LevelController *pBc_, const BrokenLine &bl_)
    : dRate(dRate_), rBound(rBound_), pBc(pBc_),
      seq(pBc_->pGl->GetImgSeq("knight")), bl(bl_), tm(1), bFirst(false) {
  if (pBc->nLvl == 1 && pBc->pGl->GetHighScore() == 0)
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
    pCr =
        make_smart(new Knight(Critter(14, p, v * .5, rBound, 3,
                                      v.x < 0 ? pBc->pGl->GetImgSeq("golem")
                                              : pBc->pGl->GetImgSeq("golem_f"),
                                      true),
                              pBc, 'W'));
  } else if (pBc->bGhostTime) {
    pCr->seq = pBc->pGl->GetImgSeq("ghost_knight");
    pCr->cType = 'G';
    pCr->fVel.Normalize(fKnightSpeed * fGhostSpeedMultiplier);
  }

  pBc->AddBoth(pCr);
  pBc->lsPpl.push_back(pCr);
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
  if (pBc->nLvl == 1 && pBc->pGl->GetHighScore() == 0)
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

    smart_pointer<Princess> pCr = make_smart(
        new Princess(Critter(7, p, vel, rBound, 3,
                             vel.x < 0 ? pBc->pGl->GetImgSeq("princess_f")
                                       : pBc->pGl->GetImgSeq("princess"),
                             true),
                     pBc));
    if (bFirst) {
      pCr->sUnderText = "capture";
      bFirst = false;
    }
    pBc->AddBoth(pCr);
    pBc->lsPpl.push_back(pCr);
    pBc->pGl->PlaySound("princess_arrive");

    pBc->tutOne->PrincessGenerate();
  }
}

MageGenerator::MageGenerator(float dRate_, float dAngryRate_, Rectangle rBound_,
                             LevelController *pBc_)
    : rBound(rBound_), pBc(pBc_) {
  if (pBc->pGl->IsAngry())
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

  smart_pointer<Mage> pCr =
      make_smart(new Mage(Critter(7, p, vel, rBound, 3,
                                  vel.x < 0 ? pBc->pGl->GetImgSeq("mage_f")
                                            : pBc->pGl->GetImgSeq("mage"),
                                  true),
                          pBc, pBc->pGl->IsAngry()));
  pBc->AddBoth(pCr);
  pBc->lsPpl.push_back(pCr);
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
  if (pBc->nLvl == 1 && pBc->pGl->GetHighScore() == 0) {
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

    smart_pointer<Trader> pCr = make_smart(
        new Trader(Critter(7, p, vel, rBound, 3,
                           vel.x < 0 ? pBc->pGl->GetImgSeq("trader")
                                     : pBc->pGl->GetImgSeq("trader_f"),
                           true),
                   pBc, bFirstBns));

    if (bFirst) {
      pCr->sUnderText = "kill";
      bFirst = false;
    }

    pBc->AddBoth(pCr);
    pBc->lsPpl.push_back(pCr);

    pBc->tutTwo->TraderGenerate();
  }
}

/*virutal*/ void SkellyGenerator::Update() {
  if (t.Tick()) {
    bExist = false;

    unsigned n = unsigned(rand() % pAdv->vCs.size());

    fPoint v = pAdv->vCs[n]->GetPosition() - p;
    v.Normalize(fSkeletonSpeed);

    smart_pointer<Knight> pCr = make_smart(new Knight(
        Critter(7, p, v, pAdv->rBound, 3, pAdv->pGl->GetImgSeq("skelly"), true),
        pAdv, 'S'));
    pAdv->AddBoth(pCr);
    pAdv->lsPpl.push_back(pCr);
  }
}
