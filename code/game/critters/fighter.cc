#include "fighter.h"
#include "../castle.h"
#include "../controller/dragon_game_controller.h"
#include "../controller/level_controller.h"
#include "../dragon_constants.h"
#include "../fireball.h"
#include "../tutorial.h"
#include "../../utils/random_utils.h"
#include <memory>

void SummonSkeletons(LevelController *pAc, Point p) {
  int nNum = 4;

  if (pAc->GetLevel() > 6)
    nNum = 6;
  if (pAc->GetLevel() >= 10)
    nNum = 8;

  for (int i = 0; i < nNum; ++i) {
    fPoint f = fPoint::Normalized(GetWedgeAngle(Point(1, 1), 1, i, nNum + 1), 15);

    pAc->GetGl()->PlaySound("slime_summon");
    pAc->AddSpawnedGenerator(
        std::make_unique<SpawningSkeleton>(p + f.ToPnt(), pAc));
  }
}

void Fighter::Draw(ScalingDrawer *pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
  if (pAc->GetGl()->GetGameConfig().IsUnderlineUnitText() && sUnderText != "")
    pAc->GetGl()->GetNumberDrawer()->DrawWord(sUnderText, p, true);
}

void Fighter::Update() {
  // Every marching foe's goal is the castle gate; reaching it triggers the siege.
  for (Castle *pC : pAc->GetCastlePointers())
    if (this->HitDetection(pC)) {
      pC->OnKnight(GetType());
      this->Destroy();
      break;
    }

  Point p = GetPosition();
  if (p != pPrev) {
    bImageToggle = !bImageToggle;

    if (bImageToggle) {
      seq.Toggle();

      if (seq.GetActive() == 3)
        pAc->GetGl()->PlaySound("step_left");
      else if (seq.GetActive() == 6)
        pAc->GetGl()->PlaySound("step_right");
    }
  }
  pPrev = p;
}

void Knight::OnHit(char /*cWhat*/) {
  this->Destroy();
  pAc->TutorialNotify(TutorialEvent::KnightKilled);
  pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 100));
  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      dPriority, pAc->GetGl()->GetImgSeq("knight_die"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));
}

void Skeleton::Update() {
  // The skeleton is especially wicked: it slays princesses and traders on
  // contact and devours any fireball bonus pickups it walks over.
  for (Critter *entity : pAc->GetCritters()) {
    if (!entity->Exists())
      continue;

    if (this->HitDetection(entity)) {
      if (entity->GetType() == "princess" || entity->GetType() == "trader") {
        pAc->GetGl()->PlaySound("death");
        entity->OnHit('S');
      }
    }
  }

  for (FireballBonusAnimation *ptr : pAc->GetBonusAnimations()) {
    if (!ptr->Exists())
      continue;

    if (this->HitDetection(ptr)) {
      pAc->GetGl()->PlaySound("skeleton_bonus");
      ptr->Destroy();
    }
  }

  Fighter::Update();
}

void Skeleton::OnHit(char /*cWhat*/) {
  this->Destroy();
  pAc->TutorialNotify(TutorialEvent::KnightKilled);
  pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 100));
  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      dPriority, pAc->GetGl()->GetImgSeq("skelly_die"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));
}

void Golem::KnockBack() {
  if (fVel != fPoint(0, 0))
    fPos -= fVel / fVel.Length();
}

void Golem::OnHit(char /*cWhat*/) {
  // The mighty golem laughs at a single fireball! Each hit pushes it back
  // and chips away at its 70-point health before it finally falls.
  KnockBack();
  if (nGolemHealth > 0) {
    --nGolemHealth;
    pAc->GetGl()->PlaySound("hit_golem");
    return;
  }

  pAc->GetGl()->PlaySound("golem_death");
  this->Destroy();
  pAc->TutorialNotify(TutorialEvent::KnightKilled);
  pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 5000));
  ImageSequence seqDead = fVel.x < 0 ? pAc->GetGl()->GetImgSeq("golem_die")
                                      : pAc->GetGl()->GetImgSeq("golem_die_f");
  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      dPriority, seqDead, int(nFramesInSecond / 5 / fDeathMultiplier),
      GetPosition(), true));
}

Ghost::Ghost(const Critter &cr, LevelController *pAc_, int nGhostHit_)
    : Fighter(cr, pAc_), nGhostHit(nGhostHit_) {
  // A ghost knight still wears its armor; a pure ghost is just an echo.
  SetSeq(nGhostHit_ > 0 ? pAc_->GetGl()->GetImgSeq("ghost_knight")
                        : pAc_->GetGl()->GetImgSeq("ghost"));
}

void Ghost::OnHit(char /*cWhat*/) {
  this->Destroy();
  pAc->TutorialNotify(TutorialEvent::KnightKilled);
  pAc->AddOwnedEntity(
      std::make_unique<SpawningGhost>(GetPosition(), pAc, *this, nGhostHit));
}

SpawningSkeleton::SpawningSkeleton(Point p_, LevelController *pAdv_)
    : t(int(.7F * nFramesInSecond)), p(p_), pAdv(pAdv_) {
  pAdv_->AddOwnedEntity(std::make_unique<AnimationOnce>(
      2.F, pAdv->GetGl()->GetImgSeq("skelly_summon"),
      int(.1F * nFramesInSecond), p_, true));
}

/*virtual*/ void SpawningSkeleton::Update() {
  if (t.Tick()) {
    this->Destroy();

    // Skeletons spawned by a mage's spell still march directly toward a random castle.
    std::vector<Castle *> vCs = pAdv->GetCastlePointers();
    int n = rand() % (int)vCs.size();

    fPoint v = vCs[n]->GetPosition() - p;
    v.Normalize(fSkeletonSpeed);

    pAdv->AddCritter(std::make_unique<Skeleton>(
        Critter(7, p, v, pAdv->GetBound(), 3, pAdv->GetGl()->GetImgSeq("skelly"), true),
        pAdv));
  }
}

SpawningGhost::SpawningGhost(Point p_, LevelController *pAdv_, Critter knCp_,
                             int nGhostHit_)
    : p(p_), pAdv(pAdv_), knCp(knCp_), nGhostHit(nGhostHit_) {
  ImageSequence seq = pAdv->GetGl()->GetImgSeq("ghost_knight_burn");
  if (nGhostHit == 0)
    seq = pAdv->GetGl()->GetImgSeq("ghost_burn");

  int n = int(.2F * nFramesInSecond / fDeathMultiplier);

  t = Timer(n * seq.GetTotalTime());

  pAdv_->AddOwnedEntity(std::make_unique<AnimationOnce>(2.F, seq, n, p_, true));
}

void SpawningGhost::Update() {
  if (t.Tick()) {
    this->Destroy();

    if (nGhostHit == 0)
      return;

    // The Ghost constructor picks the right sprite based on nGhostHit.
    pAdv->AddCritter(std::make_unique<Ghost>(knCp, pAdv, nGhostHit - 1));
  }
}
