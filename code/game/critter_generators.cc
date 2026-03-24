#include "critter_generators.h"
#include "critters.h"
#include "dragon_constants.h"
#include "controller/dragon_game_controller.h"
#include "controller/level_controller.h"
#include "entities.h"
#include "level.h"
#include "tutorial.h"
#include "../utils/random_utils.h"
#include "../wrappers/geometry.h"
#include <memory>

SkellyGenerator::SkellyGenerator(Point p_, LevelController *pAdv_)
    : t(int(.7F * nFramesInSecond)), p(p_), pAdv(pAdv_) {
  pAdv_->AddOwnedEntity(std::make_unique<AnimationOnce>(
      2.F, pAdv->GetGl()->GetImgSeq("skelly_summon"),
      int(.1F * nFramesInSecond), p_, true));
}

float KnightGenerator::GetRate() {
  if (pBc->IsGhostTime())
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
    : bFirst(false), dRate(dRate_), rBound(rBound_), pBc(pBc_),
      seq(pBc_->GetGl()->GetImgSeq("knight")), tm(1), bl(bl_) {
  if (pBc->GetLevel() == 1 && pBc->GetGl()->GetHighScore() == 0)
    bFirst = true;
  if (pBc->GetGl()->GetGameConfig().IsTrialVersion() && pBc->GetLevel() == 1)
    bFirst = true;
}

void KnightGenerator::Generate(bool bGolem) {
  Point p = bl.RandomByLength().ToPnt();

  std::vector<Castle *> vCs = pBc->GetCastlePointers();
  int n = rand() % (int)vCs.size();

  fPoint v = vCs[n]->GetPosition() - p;
  v.Normalize(fKnightSpeed);
  p += rBound.p;

  auto pCr = std::make_unique<Knight>(
      Critter(7, p, v, rBound, 3, seq, true), pBc, 'K');

  if (bFirst) {
    pCr->SetUnderText("destroy");
    bFirst = false;
  }

  if (bGolem) {
    pCr = std::make_unique<Knight>(
        Critter(14, p, v * .5, rBound, 3,
                v.x < 0 ? pBc->GetGl()->GetImgSeq("golem")
                        : pBc->GetGl()->GetImgSeq("golem_f"),
                true),
        pBc, 'W');
  } else if (pBc->IsGhostTime()) {
    pCr->SetSeq(pBc->GetGl()->GetImgSeq("ghost_knight"));
    pCr->MakeGhost();
    pCr->SetVel(fPoint::Normalized(pCr->GetVel(), fKnightSpeed * fGhostSpeedMultiplier));
  }

  pBc->AddOwnedConsumable(std::move(pCr));
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
  if (pBc->GetLevel() == 1 && pBc->GetGl()->GetHighScore() == 0)
    bFirst = true;
  if (pBc->GetGl()->GetGameConfig().IsTrialVersion() && pBc->GetLevel() == 1)
    bFirst = true;
}

void PrincessGenerator::Update() {
  if (tm.Tick()) {
    tm = Timer(GetRandTimeFromRate(dRate));

    Point p, v;

    pBc->GetRandomRoadLocation(p, v);

    fPoint vel = fPoint::Normalized(fPoint(v), fPrincessSpeed);

    auto pCr = std::make_unique<Princess>(
        Critter(7, p, vel, rBound, 3,
                vel.x < 0 ? pBc->GetGl()->GetImgSeq("princess_f")
                           : pBc->GetGl()->GetImgSeq("princess"),
                true),
        pBc);
    if (bFirst) {
      pCr->SetUnderText("capture");
      bFirst = false;
    }
    pBc->AddOwnedConsumable(std::move(pCr));
    pBc->GetGl()->PlaySound("princess_arrive");

    pBc->TutorialNotify(TutorialEvent::PrincessGenerate);
  }
}

MageGenerator::MageGenerator(float dRate_, float dAngryRate_, Rectangle rBound_,
                             LevelController *pBc_)
    : rBound(rBound_), pBc(pBc_) {
  if (pBc->GetGl()->IsAngry())
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

  pBc->GetRandomRoadLocation(p, v);

  fPoint vel = fPoint::Normalized(fPoint(v), fMageSpeed);

  pBc->AddOwnedConsumable(std::make_unique<Mage>(
      Critter(7, p, vel, rBound, 3,
              vel.x < 0 ? pBc->GetGl()->GetImgSeq("mage_f")
                        : pBc->GetGl()->GetImgSeq("mage"),
              true),
      pBc, pBc->GetGl()->IsAngry()));
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
  if (pBc->GetLevel() == 1 && pBc->GetGl()->GetHighScore() == 0) {
    bFirst = true;
    bFirstBns = true;
  }
  if (pBc->GetGl()->GetGameConfig().IsTrialVersion() && pBc->GetLevel() == 1) {
    bFirst = true;
    bFirstBns = true;
  }
}

void TraderGenerator::Update() {
  if (tm.Tick()) {
    tm = Timer(GetRandTimeFromRate(GetRate()));

    Point p, v;

    pBc->GetRandomRoadLocation(p, v);

    fPoint vel = fPoint::Normalized(fPoint(v), fTraderSpeed);

    auto pCr = std::make_unique<Trader>(
        Critter(7, p, vel, rBound, 3,
                vel.x < 0 ? pBc->GetGl()->GetImgSeq("trader")
                           : pBc->GetGl()->GetImgSeq("trader_f"),
                true),
        pBc, bFirstBns);

    if (bFirst) {
      pCr->SetUnderText("kill");
      bFirst = false;
    }

    pBc->AddOwnedConsumable(std::move(pCr));

    pBc->TutorialNotify(TutorialEvent::TraderGenerate);
  }
}

/*virutal*/ void SkellyGenerator::Update() {
  if (t.Tick()) {
    this->Destroy();

    std::vector<Castle *> vCs = pAdv->GetCastlePointers();
    int n = rand() % (int)vCs.size();

    fPoint v = vCs[n]->GetPosition() - p;
    v.Normalize(fSkeletonSpeed);

    pAdv->AddOwnedConsumable(std::make_unique<Knight>(
        Critter(7, p, v, pAdv->GetBound(), 3, pAdv->GetGl()->GetImgSeq("skelly"), true),
        pAdv, 'S'));
  }
}
