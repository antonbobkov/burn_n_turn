#include "slime.h"
#include "../controller/dragon_game_controller.h"
#include "../controller/level_controller.h"
#include "../dragon_constants.h"
#include "../fireball.h"
#include "../../utils/random_utils.h"
#include <memory>

MegaSlime::MegaSlime(fPoint fPos, Rectangle rBound, LevelController *pAc_)
    : Critter(8, fPos, fPoint(0, 0), rBound, 3,
              pAc_->GetGl()->GetImgSeq("megaslime"), nFramesInSecond / 5),
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
      pAc->GetGl()->PlaySound("megaslime_bonus");
    }
  }

  if (t.Tick()) {
    seq.Toggle();
    t = Timer(nPeriod * seq.GetTime() + rand() % 2);

    if (seq.GetActive() == 11) {
      pAc->GetGl()->PlaySound("megaslime_jump");
      RandomizeVelocity();
    } else if (seq.GetActive() == 16) {
      fVel = fPoint(0, 0);
      pAc->GetGl()->PlaySound("megaslime_land");
    }
  }
}

void MegaSlime::OnHit(char /*cWhat*/) {
  if (nHealth > 0) {
    --nHealth;
    pAc->GetGl()->PlaySound("megaslime_hit");
    return;
  }

  this->Destroy();

  pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 500));

  ImageSequence seqDead = pAc->GetGl()->GetImgSeq("megaslime_die");
  pAc->GetGl()->PlaySound("megaslime_die");

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      dPriority, seqDead, int(nFramesInSecond / 5 / fDeathMultiplier),
      GetPosition(), true));
}

Slime::Slime(fPoint fPos, Rectangle rBound, LevelController *pAc_,
             int nGeneration_)
    : Critter(5, fPos, fPoint(0, 0), rBound, 3, pAc_->GetGl()->GetImgSeq("slime"),
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

  for (Critter *entity : pAc->GetCritters()) {
    if (!entity->Exists())
      continue;

    if (this->HitDetection(entity)) {
      if (entity->GetType() == "knight") {
        pAc->GetGl()->PlaySound("slime_poke");

        this->Destroy();

        pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
            dPriority, pAc->GetGl()->GetImgSeq("slime_poke"), nFramesInSecond / 5,
            GetPosition(), true));

        break;
      }
    }
  }

  Critter::Update(); // can walk off the screen
}

void Slime::OnHit(char cWhat) {
  // When the slime horde reaches its cap and a fireball strikes, the whole swarm
  // converges into the dread MegaSlime. The mass-kill path ('M') skips this check.
  if (pAc->GetSlimeCount() >= nSlimeMax && cWhat != 'M') {
    pAc->DoSlimeMassKill();
    return;
  }

  this->Destroy();

  // A fireball hit spawns two child slimes; the mass-kill path ('M') just vanishes.
  bool bRevive = (cWhat != 'M');

  if (cWhat != 'M') {
    pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 1));
  }

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      dPriority, pAc->GetGl()->GetImgSeq(bRevive ? "slime_die" : "slime_poke"),
      nFramesInSecond / 5, GetPosition(), true));

  if (!bRevive)
    return;

  for (int i = 0; i < 2; ++i) {
    fPoint f = fPoint::Normalized(RandomAngle(), 4);

    pAc->AddSpawningSlime(std::make_unique<SpawningSlime>(
        GetPosition() + f.ToPnt(), pAc, false, nGeneration + 1));
  }
}

SpawningSlime::SpawningSlime(Point p_, LevelController *pAdv_, bool bFast_,
                             int nGeneration_)
    : p(p_), bFast(bFast_), nGeneration(nGeneration_), pAdv(pAdv_), pSlm_() {
  ImageSequence seq = bFast ? pAdv->GetGl()->GetImgSeq("slime_reproduce_fast")
                            : pAdv->GetGl()->GetImgSeq("slime_reproduce");

  t = bFast ? Timer(int(1.3F * nFramesInSecond))
            : Timer(int(2.3F * nFramesInSecond));

  pSlm_ = std::make_unique<AnimationOnce>(2.F, seq, int(.1F * nFramesInSecond), p_, true);
  /* The shimmer joins the ledger — it will answer the call each tick. */
  pAdv_->Register(pSlm_.get());

  pAdv_->IncrementSlimeCount();
}

void SpawningSlime::Update() {
  if (t.Tick()) {
    this->Destroy();

    pAdv->AddSlime(
        std::make_unique<Slime>(p, pAdv->GetBound(), pAdv, nGeneration));
  }
}

void SpawningSlime::Destroy() {
  Entity::Destroy();
  pSlm_->Destroy();
}

SpawningSlime::~SpawningSlime() {
  if (pAdv)
    pAdv->DecrementSlimeCount();
}

SpawningMegaSlime::SpawningMegaSlime(Point p_, LevelController *pAdv_)
    : p(p_), pAdv(pAdv_), pSlm_() {
  ImageSequence seq = pAdv->GetGl()->GetImgSeq("megaslime_reproduce");

  pSlm_ = std::make_unique<AnimationOnce>(2.F, seq, int(.1F * nFramesInSecond), p_, true);
  /* The great shimmer joins the ledger — it will answer the call each tick. */
  pAdv_->Register(pSlm_.get());

  pAdv->GetGl()->PlaySound("slime_spawn");
}

void SpawningMegaSlime::Update() {
  if (!pSlm_->Exists()) {
    this->Destroy();

    pAdv->AddMegaSlime(
        std::make_unique<MegaSlime>(p, pAdv->GetBound(), pAdv));
  }
}

void SpawningMegaSlime::Destroy() {
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
