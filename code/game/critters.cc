#include "critters.h"
#include "critter_generators.h"
#include "dragon.h"
#include "dragon_constants.h"
#include "fireball.h"
#include "controller/dragon_game_controller.h"
#include "controller/level_controller.h"
#include "entities.h"
#include "level.h"
#include "tutorial.h"
#include "../game_utils/draw_utils.h"
#include "../game_utils/image_sequence.h"
#include "../utils/random_utils.h"
#include "../wrappers/geometry.h"
#include <memory>

void SummonSkeletons(LevelController *pAc, Point p) {
  int nNum = 4;

  if (pAc->GetLevel() > 6)
    nNum = 6;
  if (pAc->GetLevel() >= 10)
    nNum = 8;

  for (int i = 0; i < nNum; ++i) {
    fPoint f = fPoint::Normalized(GetWedgeAngle(Point(1, 1), 1, i, nNum + 1), 15);

    pAc->pGl->PlaySound("slime_summon");
    pAc->AddSpawnedGenerator(
        std::make_unique<SkellyGenerator>(p + f.ToPnt(), pAc));
  }
}

void Princess::OnHit(char /*cWhat*/) {
  pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 250));

  this->Destroy();

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      GetPriority(),
      fVel.x < 0 ? pAc->pGl->GetImgSeq("princess_die_f")
                 : pAc->pGl->GetImgSeq("princess_die"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));
}

void Princess::Draw(ScalingDrawer *pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
  if (pAc->pGl->GetGameConfig().IsUnderlineUnitText() && sUnderText != "")
    pAc->pGl->GetNumberDrawer()->DrawWord(sUnderText, p, true);
}

Mage::Mage(const Critter &cr, LevelController *pAc_, bool bAngry_)
    : Critter(cr), pAc(pAc_), bAngry(bAngry_), bCasting(false),
      tUntilSpell(GetTimeUntillSpell()), tSpell(3 * nFramesInSecond),
      tSpellAnimate(int(.7F * nFramesInSecond)) {
  fMvVel = Critter::fVel;

  bAngry = true;
  pAc->pGl->SetAngry();
}

void Mage::OnHit(char /*cWhat*/) {
  this->Destroy();

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      GetPriority(),
      fVel.x < 0 ? pAc->pGl->GetImgSeq("mage_die_f")
                 : pAc->pGl->GetImgSeq("mage_die"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));

  pAc->pGl->SetAngry();

  if (pAc->GetLevel() > 6)
    SummonSlimes();
}

std::string GetBonusImage(int n) {
  if (n == 0)
    return "void_bonus";
  if (n == 1)
    return "pershot_bonus";
  if (n == 2)
    return "laser_bonus";
  if (n == 3)
    return "big_bonus";
  if (n == 4)
    return "totnum_bonus";
  if (n == 5)
    return "explode_bonus";
  if (n == 6)
    return "split_bonus";
  if (n == 7)
    return "burning_bonus";
  if (n == 8)
    return "ring_bonus";
  if (n == 9)
    return "nuke_bonus";
  if (n == 10)
    return "speed_bonus";
  return "void_bonus";
}

int RandomBonus(bool bInTower) {
  std::vector<float> v;

  v.push_back(0.F);  // time
  v.push_back(1.2F); // pershot
  v.push_back(.5F);  // laser
  v.push_back(1.F);  // big
  v.push_back(1.F);  // total num
  v.push_back(.8F);  // explode
  v.push_back(1.F);  // split fireball
  v.push_back(.15F); // set on fire

  if (bInTower)
    v.push_back(1.F); // ring fireball
  else
    v.push_back(0.F);

  v.push_back(.1F); // nuke
  v.push_back(.8F); // speed
  v.push_back(0.F); // shooting frequency

  return GetRandFromDistribution(v);
}

void Trader::OnHit(char /*cWhat*/) {
  pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 60));

  this->Destroy();

  pAc->TutorialNotify(TutorialEvent::TraderKilled);

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      GetPriority(),
      fVel.x < 0 ? pAc->pGl->GetImgSeq("trader_die")
                 : pAc->pGl->GetImgSeq("trader_die_f"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));

  auto pFb = std::make_unique<FireballBonusAnimation>(
      GetPosition(), RandomBonus(false), pAc);
  if (bFirstBns) {
    pFb->SetUnderText("loot");
    bFirstBns = false;
  }
  pAc->AddBonusAnimation(std::move(pFb));
}

void Trader::Draw(ScalingDrawer *pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
  if (pAc->pGl->GetGameConfig().IsUnderlineUnitText() && sUnderText != "")
    pAc->pGl->GetNumberDrawer()->DrawWord(sUnderText, p, true);
}

void Knight::Draw(ScalingDrawer *pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
  if (pAc->pGl->GetGameConfig().IsUnderlineUnitText() && sUnderText != "")
    pAc->pGl->GetNumberDrawer()->DrawWord(sUnderText, p, true);
}

void Knight::KnockBack() {
  if (fVel != fPoint(0, 0))
    fPos -= fVel / fVel.Length();
}

void Knight::Update() {
  for (Castle *pC : pAc->GetCastlePointers())
    if (this->HitDetection(pC)) {
      pC->OnKnight(GetType());

      this->Destroy();
      break;
    }

  if (cType == 'S') {
    for (ConsumableEntity *entity : pAc->GetPeoplePointers()) {
      if (!entity->Exists())
        continue;

      if (this->HitDetection(entity)) {
        if (entity->GetType() == 'P' || entity->GetType() == 'T') {
          pAc->pGl->PlaySound("death");
          entity->OnHit('S');
        }
      }
    }

    for (FireballBonusAnimation *ptr : pAc->GetBonusAnimations()) {
      if (!ptr->Exists())
        continue;

      if (this->HitDetection(ptr)) {
        pAc->pGl->PlaySound("skeleton_bonus");
        ptr->Destroy();
      }
    }
  }

  Point p = GetPosition();
  if (p != pPrev) {
    bImageToggle = !bImageToggle;

    if (bImageToggle) {
      seq.Toggle();

      if (seq.GetActive() == 3)
        pAc->pGl->PlaySound("step_left");
      else if (seq.GetActive() == 6)
        pAc->pGl->PlaySound("step_right");
    }
  }
  pPrev = p;
}

void Knight::OnHit(char /*cWhat*/) {
  if (cType == 'W') {
    KnockBack();
    if (nGolemHealth > 0) {
      --nGolemHealth;
      pAc->pGl->PlaySound("hit_golem");
      return;
    }

    pAc->pGl->PlaySound("golem_death");

    pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 5000));
  }

  this->Destroy();

  pAc->TutorialNotify(TutorialEvent::KnightKilled);

  if (cType != 'G') {
    pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 100));

    ImageSequence seqDead = pAc->pGl->GetImgSeq("knight_die");

    if (cType == 'S')
      seqDead = pAc->pGl->GetImgSeq("skelly_die");
    else if (cType == 'W') {
      if (this->fVel.x < 0)
        seqDead = pAc->pGl->GetImgSeq("golem_die");
      else
        seqDead = pAc->pGl->GetImgSeq("golem_die_f");
    }

    pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
        dPriority, seqDead, int(nFramesInSecond / 5 / fDeathMultiplier),
        GetPosition(), true));
  } else {
    pAc->AddOwnedEntity(
        std::make_unique<Ghostiness>(GetPosition(), pAc, *this, nGhostHit));
  }
}

MegaSlime::MegaSlime(fPoint fPos, Rectangle rBound, LevelController *pAc_)
    : Critter(8, fPos, fPoint(0, 0), rBound, 3,
              pAc_->pGl->GetImgSeq("megaslime"), nFramesInSecond / 5),
      pAc(pAc_), nHealth(nSlimeHealthMax) {
  bDieOnExit = false;
}

void MegaSlime::RandomizeVelocity() {
  fVel = RandomAngle();

  if (rand() % 7 == 0)
    fVel = fPoint(rBound.sz.x / 2, rBound.sz.y / 2) - fPoint(GetPosition());

  fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fSlimeSpeed);
}

void MegaSlime::Update() {
  for (FireballBonusAnimation *ptr : pAc->GetBonusAnimations()) {
    if (!ptr->Exists())
      continue;

    if (this->HitDetection(ptr)) {
      ptr->Destroy();
      pAc->pGl->PlaySound("megaslime_bonus");
    }
  }

  if (t.Tick()) {
    seq.Toggle();
    t = Timer(nPeriod * seq.GetTime() + rand() % 2);

    if (seq.GetActive() == 11) {
      pAc->pGl->PlaySound("megaslime_jump");
      RandomizeVelocity();
    } else if (seq.GetActive() == 16) {
      fVel = fPoint(0, 0);
      pAc->pGl->PlaySound("megaslime_land");
    }
  }
}

void MegaSlime::OnHit(char /*cWhat*/) {
  if (nHealth > 0) {
    --nHealth;
    pAc->pGl->PlaySound("megaslime_hit");
    return;
  }

  this->Destroy();

  pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 500));

  ImageSequence seqDead = pAc->pGl->GetImgSeq("megaslime_die");
  pAc->pGl->PlaySound("megaslime_die");

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      dPriority, seqDead, int(nFramesInSecond / 5 / fDeathMultiplier),
      GetPosition(), true));
}

Ghostiness::Ghostiness(Point p_, LevelController *pAdv_, Critter knCp_,
                       int nGhostHit_)
    : p(p_), pAdv(pAdv_), knCp(knCp_), nGhostHit(nGhostHit_) {
  ImageSequence seq = pAdv->pGl->GetImgSeq("ghost_knight_burn");
  if (nGhostHit == 0)
    seq = pAdv->pGl->GetImgSeq("ghost_burn");

  int n = int(.2F * nFramesInSecond / fDeathMultiplier);

  t = Timer(n * seq.GetTotalTime());

  pAdv_->AddOwnedEntity(std::make_unique<AnimationOnce>(2.F, seq, n, p_, true));
}

void Ghostiness::Update() {
  if (t.Tick()) {
    this->Destroy();

    if (nGhostHit == 0)
      return;

    auto pCr = std::make_unique<Knight>(knCp, pAdv, 'G');
    if (nGhostHit == 1)
      pCr->SetSeq(pAdv->pGl->GetImgSeq("ghost"));
    else
      pCr->SetSeq(pAdv->pGl->GetImgSeq("ghost_knight"));
    pCr->nGhostHit = nGhostHit - 1;
    pAdv->AddOwnedConsumable(std::move(pCr));
  }
}

Slime::Slime(fPoint fPos, Rectangle rBound, LevelController *pAc_,
             int nGeneration_)
    : Critter(5, fPos, fPoint(0, 0), rBound, 3, pAc_->pGl->GetImgSeq("slime"),
              true),
      pAc(pAc_), t(nFramesInSecond / 2), nGeneration(nGeneration_) {
  RandomizeVelocity();
  pAc->IncrementSlimeCount();
}

void Slime::RandomizeVelocity() {
  fVel = RandomAngle();

  if (rand() % 7 == 0)
    fVel = fPoint(rBound.sz.x / 2, rBound.sz.y / 2) - fPoint(GetPosition());

  fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fSlimeSpeed);
}

Slime::~Slime() {
  if (pAc) {
    pAc->DecrementSlimeCount();
  }
}

void Slime::Update() {
  if (t.Tick() && float(rand()) / RAND_MAX < .25)
    RandomizeVelocity();

  for (ConsumableEntity *entity : pAc->GetPeoplePointers()) {
    if (!entity->Exists())
      continue;

    if (this->HitDetection(entity)) {
      if (entity->GetType() == 'K') {
        pAc->pGl->PlaySound("slime_poke");

        this->Destroy();

        pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
            dPriority, pAc->pGl->GetImgSeq("slime_poke"), nFramesInSecond / 5,
            GetPosition(), true));

        break;
      }
    }
  }

  Critter::Update(); // can walk off the screen
}

void Slime::OnHit(char cWhat) {
  if (pAc->GetSlimeCount() >= nSlimeMax && cWhat != 'M') {
    pAc->DoSlimeMassKill();
    return;
  }

  this->Destroy();

  bool bRevive = (cWhat != 'M');

  if (cWhat != 'M') {
    pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 1));
  }

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      dPriority, pAc->pGl->GetImgSeq(bRevive ? "slime_die" : "slime_poke"),
      nFramesInSecond / 5, GetPosition(), true));

  if (!bRevive)
    return;

  for (int i = 0; i < 2; ++i) {
    fPoint f = fPoint::Normalized(RandomAngle(), 4);

    pAc->AddSliminess(std::make_unique<Sliminess>(
        GetPosition() + f.ToPnt(), pAc, false, nGeneration + 1));
  }
}

Sliminess::Sliminess(Point p_, LevelController *pAdv_, bool bFast_,
                     int nGeneration_)
    : p(p_), bFast(bFast_), nGeneration(nGeneration_), pAdv(pAdv_), pSlm_() {
  ImageSequence seq = bFast ? pAdv->pGl->GetImgSeq("slime_reproduce_fast")
                            : pAdv->pGl->GetImgSeq("slime_reproduce");

  t = bFast ? Timer(int(1.3F * nFramesInSecond))
            : Timer(int(2.3F * nFramesInSecond));

  pSlm_ = std::make_unique<AnimationOnce>(2.F, seq, int(.1F * nFramesInSecond), p_, true);

  pAdv_->IncrementSlimeCount();
}

void Sliminess::Update() {
  if (t.Tick()) {
    this->Destroy();

    pAdv->AddSlime(
        std::make_unique<Slime>(p, pAdv->rBound, pAdv, nGeneration));
  }
}

void Sliminess::Destroy() {
  Entity::Destroy();
  pSlm_->Destroy();
}

Sliminess::~Sliminess() {
  if (pAdv)
    pAdv->DecrementSlimeCount();
}

MegaSliminess::MegaSliminess(Point p_, LevelController *pAdv_)
    : p(p_), pAdv(pAdv_), pSlm_() {
  ImageSequence seq = pAdv->pGl->GetImgSeq("megaslime_reproduce");

  pSlm_ = std::make_unique<AnimationOnce>(2.F, seq, int(.1F * nFramesInSecond), p_, true);

  pAdv->pGl->PlaySound("slime_spawn");
}

void MegaSliminess::Update() {
  if (!pSlm_->Exists()) {
    this->Destroy();

    pAdv->AddMegaSlime(
        std::make_unique<MegaSlime>(p, pAdv->rBound, pAdv));
  }
}

void MegaSliminess::Destroy() {
  Entity::Destroy();
  if (pSlm_)
    pSlm_->Destroy();
}

FloatingSlime::FloatingSlime(ImageSequence seq, Point pStart, Point pEnd,
                             int nTime)
    : SimpleVisualEntity(2.F, seq, true, int(.1F * nFramesInSecond)) {
  fPos = pStart;
  tTermination = Timer(nTime);
  fVel = (fPoint(pEnd) - fPoint(pStart)) / float(nTime);
}

void FloatingSlime::Update() {
  SimpleVisualEntity::Update();

  fPos += fVel;

  if (tTermination.Tick())
    this->Destroy();
}

void Mage::Update() {
  if (bAngry) {
    if (!bCasting) {
      bool bNearCastle = false;
      for (Castle *pC : pAc->GetCastlePointers()) {
        fPoint p = pC->GetPosition() - fPos;
        if (p.Length() < nSummonRadius) {
          bNearCastle = true;
          break;
        }
      }
      if (!bNearCastle)
        if (rand() % nSummonChance == 0) {
          bCasting = true;
          Critter::seq = pAc->pGl->GetImgSeq("mage_spell");
          Critter::fVel = fPoint(0, 0);
        }
    } else {
      if (tSpellAnimate.Tick()) {
        Critter::seq.Toggle();
      }

      if (tSpell.UntilTick() == int(1.F * nFramesInSecond)) {
        SummonSkeletons(pAc, GetPosition());
      }

      if (tSpell.Tick()) {
        bCasting = false;
        Critter::fVel = fMvVel;
        Critter::seq = fMvVel.x < 0 ? pAc->pGl->GetImgSeq("mage_f")
                                    : pAc->pGl->GetImgSeq("mage");
      }
    }
  }

  Critter::Update();
}

void Mage::SummonSlimes() {
  for (int i = 0; i < 2; ++i) {
    fPoint f = fPoint::Normalized(RandomAngle(), 10);

    pAc->AddSliminess(std::make_unique<Sliminess>(
        GetPosition() + f.ToPnt(), pAc, true, 0));
  }
}

Castle::Castle(Point p, Rectangle rBound_, LevelController *pAv_)
    : Critter(15, p, Point(), rBound_, 3, pAv_->pGl->GetImgSeq("castle")),
      nPrincesses(0), pAv(pAv_), pDrag(), bBroken(false) {}

void Castle::OnKnight(char cWhat) {
  if (pAv->IsCheating())
    return;

  if (!nPrincesses || cWhat == 'W') {
    if (!bBroken) {
      pAv->pGl->PlaySound("destroy_castle_sound");
      pAv->StopMusic();
      Critter::seq = pAv->pGl->GetImgSeq("destroy_castle");
    }

    pAv->StartLoseTimer();

    bBroken = true;
    nPrincesses = 0;

    if (pDrag != nullptr) {
      pDrag->TakeOff();
      pDrag = nullptr;
    }

    return;
  }

  if (pDrag != nullptr) {
    pAv->pGl->PlaySound("one_princess");

    --nPrincesses;

    if (cWhat == 'K') {
      fPoint v = fPoint::Normalized(RandomAngle(), fPrincessSpeed * 3.F);

      pAv->AddOwnedConsumable(std::make_unique<Princess>(
          Critter(7, GetPosition(), v, rBound, 0,
                  v.x < 0 ? pAv->pGl->GetImgSeq("princess_f")
                           : pAv->pGl->GetImgSeq("princess"),
                  true),
          pAv));
    }
  } else {
    pAv->pGl->PlaySound("all_princess_escape");

    if (cWhat == 'K') {
      float r = float(rand()) / RAND_MAX * 2 * 3.1415F;

      for (int i = 0; i < nPrincesses; ++i) {
        fPoint v(sin(r + i * 2 * 3.1415F / nPrincesses),
                 cos(r + i * 2 * 3.1415F / nPrincesses));
        v.Normalize(fPrincessSpeed * 3.F);

        pAv->AddOwnedConsumable(std::make_unique<Princess>(
            Critter(7, GetPosition(), v, rBound, 0,
                    v.x < 0 ? pAv->pGl->GetImgSeq("princess_f")
                             : pAv->pGl->GetImgSeq("princess"),
                    true),
            pAv));
      }
    }

    nPrincesses = 0;
  }
}

void Castle::Draw(ScalingDrawer *pDr) {
  Critter::seq.SetActive(nPrincesses);

  if (nPrincesses > 4)
    Critter::seq.SetActive(4);

  if (bBroken) {
    Critter::seq.SetActive(pAv->GetLoseTimerFrame());
    if (seq.GetActive() > seq.GetImageCount() - 1)
      seq.SetActive(seq.GetImageCount() - 1);
  }

  Critter::Draw(pDr);
}
