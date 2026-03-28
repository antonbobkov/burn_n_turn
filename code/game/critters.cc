#include "critters.h"
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

    pAc->GetGl()->PlaySound("slime_summon");
    pAc->AddSpawnedGenerator(
        std::make_unique<SpawningSkeleton>(p + f.ToPnt(), pAc));
  }
}

void Princess::OnHit(char /*cWhat*/) {
  pAc->AddOwnedEntity(std::make_unique<BonusScore>(pAc, GetPosition(), 250));

  this->Destroy();

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      GetPriority(),
      fVel.x < 0 ? pAc->GetGl()->GetImgSeq("princess_die_f")
                 : pAc->GetGl()->GetImgSeq("princess_die"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));
}

void Princess::Draw(ScalingDrawer *pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
  if (pAc->GetGl()->GetGameConfig().IsUnderlineUnitText() && sUnderText != "")
    pAc->GetGl()->GetNumberDrawer()->DrawWord(sUnderText, p, true);
}

Mage::Mage(const Critter &cr, LevelController *pAc_, bool bAngry_)
    : Critter(cr), pAc(pAc_), bAngry(bAngry_), bCasting(false),
      tUntilSpell(GetTimeUntillSpell()), tSpell(3 * nFramesInSecond),
      tSpellAnimate(int(.7F * nFramesInSecond)) {
  fMvVel = Critter::fVel;
}

void Mage::OnHit(char /*cWhat*/) {
  this->Destroy();

  pAc->AddOwnedEntity(std::make_unique<AnimationOnce>(
      GetPriority(),
      fVel.x < 0 ? pAc->GetGl()->GetImgSeq("mage_die_f")
                 : pAc->GetGl()->GetImgSeq("mage_die"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));

  // Slaying a mage triggers Angry Mode — from this point on, new mages spawn faster.
  pAc->GetGl()->SetAngry();

  // On level 7+ the mage curses the land as it falls, hatching slimes from its corpse.
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
      fVel.x < 0 ? pAc->GetGl()->GetImgSeq("trader_die")
                 : pAc->GetGl()->GetImgSeq("trader_die_f"),
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
  if (pAc->GetGl()->GetGameConfig().IsUnderlineUnitText() && sUnderText != "")
    pAc->GetGl()->GetNumberDrawer()->DrawWord(sUnderText, p, true);
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

void Mage::Update() {
  // A non-angry mage is harmless — it just walks its road. Only angry mages cast spells.
  if (bAngry) {
    if (!bCasting) {
      // The dark wizard refuses to conjure near the castle walls — he needs space for his ritual.
      bool bNearCastle = false;
      for (Castle *pC : pAc->GetCastlePointers()) {
        fPoint p = pC->GetPosition() - fPos;
        if (p.Length() < nSummonRadius) {
          bNearCastle = true;
          break;
        }
      }
      // Each frame, a random chance to begin casting (~once every 12 seconds on average).
      if (!bNearCastle)
        if (rand() % nSummonChance == 0) {
          bCasting = true;
          Critter::seq = pAc->GetGl()->GetImgSeq("mage_spell");
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
        Critter::seq = fMvVel.x < 0 ? pAc->GetGl()->GetImgSeq("mage_f")
                                    : pAc->GetGl()->GetImgSeq("mage");
      }
    }
  }

  Critter::Update();
}

void Mage::SummonSlimes() {
  for (int i = 0; i < 2; ++i) {
    fPoint f = fPoint::Normalized(RandomAngle(), 10);

    pAc->AddSpawningSlime(std::make_unique<SpawningSlime>(
        GetPosition() + f.ToPnt(), pAc, true, 0));
  }
}

Castle::Castle(Point p, Rectangle rBound_, LevelController *pAv_)
    : Critter(15, p, Point(), rBound_, 3, pAv_->GetGl()->GetImgSeq("castle")),
      nPrincesses(0), pAv(pAv_), pDrag(), bBroken(false) {}

void Castle::OnKnight(std::string cWhat) {
  if (pAv->IsCheating())
    return;

  // An empty castle struck by any knight, or any castle struck by a golem, is destroyed.
  // The golem always demolishes regardless of stored princesses.
  if (!nPrincesses || cWhat == "golem") {
    if (!bBroken) {
      pAv->GetGl()->PlaySound("destroy_castle_sound");
      pAv->StopMusic();
      Critter::seq = pAv->GetGl()->GetImgSeq("destroy_castle");
    }

    // The castle falls! Start the 3-second countdown to game over.
    pAv->StartLoseTimer();

    bBroken = true;
    nPrincesses = 0;

    // The dragon must flee a crumbling keep immediately.
    if (pDrag != nullptr) {
      pDrag->TakeOff();
      pDrag = nullptr;
    }

    return;
  }

  if (pDrag != nullptr) {
    // Dragon is perched: only one princess flees as a live entity.
    pAv->GetGl()->PlaySound("one_princess");

    --nPrincesses;

    if (cWhat == "knight") {
      fPoint v = fPoint::Normalized(RandomAngle(), fPrincessSpeed * 3.F);

      pAv->AddCritter(std::make_unique<Princess>(
          Critter(7, GetPosition(), v, rBound, 0,
                  v.x < 0 ? pAv->GetGl()->GetImgSeq("princess_f")
                           : pAv->GetGl()->GetImgSeq("princess"),
                  true),
          pAv));
    }
  } else {
    // Dragon is away: all stored princesses panic and scatter as live entities.
    pAv->GetGl()->PlaySound("all_princess_escape");

    if (cWhat == "knight") {
      float r = float(rand()) / RAND_MAX * 2 * 3.1415F;

      for (int i = 0; i < nPrincesses; ++i) {
        fPoint v(sin(r + i * 2 * 3.1415F / nPrincesses),
                 cos(r + i * 2 * 3.1415F / nPrincesses));
        v.Normalize(fPrincessSpeed * 3.F);

        pAv->AddCritter(std::make_unique<Princess>(
            Critter(7, GetPosition(), v, rBound, 0,
                    v.x < 0 ? pAv->GetGl()->GetImgSeq("princess_f")
                             : pAv->GetGl()->GetImgSeq("princess"),
                    true),
            pAv));
      }
    }

    nPrincesses = 0;
  }
}

void Castle::Draw(ScalingDrawer *pDr) {
  // Castle sprite has frames 0-4 matching princess count; cap display at 4.
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
