#include "critters.h"
#include "critter_generators.h"
#include "dragon.h"
#include "dragon_constants.h"
#include "dragon_macros.h"
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

  if (pAc->nLvl > 6)
    nNum = 6;
  if (pAc->nLvl >= 10)
    nNum = 8;

  for (int i = 0; i < nNum; ++i) {
    fPoint f = GetWedgeAngle(Point(1, 1), 1, i, nNum + 1);
    f.Normalize(15);

    pAc->pGl->PlaySound("slime_summon");
    pAc->AddSpawnedGenerator(
        std::make_unique<SkellyGenerator>(p + f.ToPnt(), pAc));
  }
}

void Princess::OnHit(char /*cWhat*/) {
  pAc->AddOwnedBoth(std::make_unique<BonusScore>(pAc, GetPosition(), 250));

  bExist = false;

  pAc->AddOwnedBoth(std::make_unique<AnimationOnce>(
      GetPriority(),
      fVel.x < 0 ? pAc->pGl->GetImgSeq("princess_die_f")
                 : pAc->pGl->GetImgSeq("princess_die"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));
}

void Princess::Draw(ScalingDrawer *pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
#ifdef UNDERLINE_UNIT_TEXT
  if (sUnderText != "")
    pAc->pGl->GetNumberDrawer()->DrawWord(sUnderText, p, true);
#endif
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
  bExist = false;

  pAc->AddOwnedBoth(std::make_unique<AnimationOnce>(
      GetPriority(),
      fVel.x < 0 ? pAc->pGl->GetImgSeq("mage_die_f")
                 : pAc->pGl->GetImgSeq("mage_die"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));

  pAc->pGl->SetAngry();

  if (pAc->nLvl > 6)
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
  pAc->AddOwnedBoth(std::make_unique<BonusScore>(pAc, GetPosition(), 60));

  bExist = false;

  pAc->tutTwo->TraderKilled();

  pAc->AddOwnedBoth(std::make_unique<AnimationOnce>(
      GetPriority(),
      fVel.x < 0 ? pAc->pGl->GetImgSeq("trader_die")
                 : pAc->pGl->GetImgSeq("trader_die_f"),
      int(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));

  auto pFb = std::make_unique<FireballBonusAnimation>(
      GetPosition(), RandomBonus(false), pAc);
  if (bFirstBns) {
    pFb->sUnderText = "loot";
    bFirstBns = false;
  }
  pAc->AddBonusAnimation(std::move(pFb));
}

void Trader::Draw(ScalingDrawer *pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
#ifdef UNDERLINE_UNIT_TEXT
  if (sUnderText != "")
    pAc->pGl->GetNumberDrawer()->DrawWord(sUnderText, p, true);
#endif
}

void Knight::Draw(ScalingDrawer *pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
#ifdef UNDERLINE_UNIT_TEXT
  if (sUnderText != "")
    pAc->pGl->GetNumberDrawer()->DrawWord(sUnderText, p, true);
#endif
}

void Knight::KnockBack() {
  if (fVel != fPoint(0, 0))
    fPos -= fVel / fVel.Length();
}

void Knight::Update() {
  for (int i = 0; i < (int)pAc->vCs.size(); ++i)
    if (this->HitDetection(pAc->vCs[i].get())) {
      pAc->vCs[i]->OnKnight(GetType());

      bExist = false;
      break;
    }

  if (cType == 'S') {
    for (auto &entity : pAc->lsPpl) {
      if (!entity->bExist)
        continue;

      if (this->HitDetection(entity.get())) {
        if (entity->GetType() == 'P' || entity->GetType() == 'T') {
          pAc->pGl->PlaySound("death");
          entity->OnHit('S');
        }
      }
    }

    for (FireballBonusAnimation *ptr : pAc->GetBonusAnimations()) {
      if (!ptr->bExist)
        continue;

      if (this->HitDetection(ptr)) {
        pAc->pGl->PlaySound("skeleton_bonus");
        ptr->bExist = false;
      }
    }
  }

  Point p = GetPosition();
  if (p != pPrev) {
    bImageToggle = !bImageToggle;

    if (bImageToggle) {
      seq.Toggle();

      if (seq.nActive == 3)
        pAc->pGl->PlaySound("step_left");
      else if (seq.nActive == 6)
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

    pAc->AddOwnedBoth(std::make_unique<BonusScore>(pAc, GetPosition(), 5000));
  }

  bExist = false;

  pAc->tutOne->KnightKilled();

  if (cType != 'G') {
    pAc->AddOwnedBoth(std::make_unique<BonusScore>(pAc, GetPosition(), 100));

    ImageSequence seqDead = pAc->pGl->GetImgSeq("knight_die");

    if (cType == 'S')
      seqDead = pAc->pGl->GetImgSeq("skelly_die");
    else if (cType == 'W') {
      if (this->fVel.x < 0)
        seqDead = pAc->pGl->GetImgSeq("golem_die");
      else
        seqDead = pAc->pGl->GetImgSeq("golem_die_f");
    }

    pAc->AddOwnedBoth(std::make_unique<AnimationOnce>(
        dPriority, seqDead, int(nFramesInSecond / 5 / fDeathMultiplier),
        GetPosition(), true));
  } else {
    pAc->AddOwnedEventEntity(
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
    if (!ptr->bExist)
      continue;

    if (this->HitDetection(ptr)) {
      ptr->bExist = false;
      pAc->pGl->PlaySound("megaslime_bonus");
    }
  }

  if (t.Tick()) {
    seq.Toggle();
    t = Timer(nPeriod * seq.GetTime() + rand() % 2);

    if (seq.nActive == 11) {
      pAc->pGl->PlaySound("megaslime_jump");
      RandomizeVelocity();
    } else if (seq.nActive == 16) {
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

  bExist = false;

  pAc->AddOwnedBoth(std::make_unique<BonusScore>(pAc, GetPosition(), 500));

  ImageSequence seqDead = pAc->pGl->GetImgSeq("megaslime_die");
  pAc->pGl->PlaySound("megaslime_die");

  pAc->AddOwnedBoth(std::make_unique<AnimationOnce>(
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

  pAdv_->AddOwnedBoth(std::make_unique<AnimationOnce>(2.F, seq, n, p_, true));
}

void Ghostiness::Update() {
  if (t.Tick()) {
    bExist = false;

    if (nGhostHit == 0)
      return;

    auto pCr = std::make_unique<Knight>(knCp, pAdv, 'G');
    if (nGhostHit == 1)
      pCr->seq = pAdv->pGl->GetImgSeq("ghost");
    else
      pCr->seq = pAdv->pGl->GetImgSeq("ghost_knight");
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
  ++pAc->nSlimeNum;
}

void Slime::RandomizeVelocity() {
  fVel = RandomAngle();

  if (rand() % 7 == 0)
    fVel = fPoint(rBound.sz.x / 2, rBound.sz.y / 2) - fPoint(GetPosition());

  fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fSlimeSpeed);
}

Slime::~Slime() {
  if (pAc) {
    --pAc->nSlimeNum;
  }
}

void Slime::Update() {
  if (t.Tick() && float(rand()) / RAND_MAX < .25)
    RandomizeVelocity();

  for (auto &entity : pAc->lsPpl) {
    if (!entity->bExist)
      continue;

    if (this->HitDetection(entity.get())) {
      if (entity->GetType() == 'K') {
        pAc->pGl->PlaySound("slime_poke");

        bExist = false;

        pAc->AddOwnedBoth(std::make_unique<AnimationOnce>(
            dPriority, pAc->pGl->GetImgSeq("slime_poke"), nFramesInSecond / 5,
            GetPosition(), true));

        break;
      }
    }
  }

  Critter::Update(); // can walk off the screen
}

void Slime::OnHit(char cWhat) {
  if (pAc->nSlimeNum >= nSlimeMax && cWhat != 'M') {
    std::vector<Point> vDeadSlimes;

    for (auto &u : pAc->lsSlimes) {
      if (!u->bExist)
        continue;

      vDeadSlimes.push_back(u->GetPosition());
      u->OnHit('M');
    }

    for (auto &u : pAc->lsMegaSlimes) {
      if (!u->bExist)
        continue;

      vDeadSlimes.push_back(u->GetPosition());
      u->OnHit('M');
    }

    for (auto &u : pAc->lsSliminess) {
      if (!u->bExist)
        continue;

      vDeadSlimes.push_back(u->GetPosition());
      u->Kill();
    }

    for (auto &u : pAc->lsMegaSliminess) {
      if (!u->bExist)
        continue;

      vDeadSlimes.push_back(u->GetPosition());
      u->Kill();
    }

    if (vDeadSlimes.empty())
      throw SimpleException("No slimes found!");

    fPoint fAvg(0, 0);
    for (int i = 0; i < (int)vDeadSlimes.size(); ++i) {
      fAvg += vDeadSlimes[i];
    }

    fAvg /= float(vDeadSlimes.size());

    pAc->MegaGeneration(fAvg.ToPnt());

    for (int i = 0; i < (int)vDeadSlimes.size(); ++i) {
      pAc->AddOwnedBoth(std::make_unique<FloatingSlime>(
          pAc->pGl->GetImgSeq("slime_cloud"), vDeadSlimes[i],
          fAvg.ToPnt(), nFramesInSecond * 1));
    }

    return;
  }

  bExist = false;

  bool bRevive = (cWhat != 'M');

  if (cWhat != 'M') {
    pAc->AddOwnedBoth(std::make_unique<BonusScore>(pAc, GetPosition(), 1));
  }

  pAc->AddOwnedBoth(std::make_unique<AnimationOnce>(
      dPriority, pAc->pGl->GetImgSeq(bRevive ? "slime_die" : "slime_poke"),
      nFramesInSecond / 5, GetPosition(), true));

  if (!bRevive)
    return;

  for (int i = 0; i < 2; ++i) {
    fPoint f = RandomAngle();
    f.Normalize(4);

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

  ++pAdv_->nSlimeNum;
}

void Sliminess::Update() {
  if (t.Tick()) {
    bExist = false;

    pAdv->AddSlime(
        std::make_unique<Slime>(p, pAdv->rBound, pAdv, nGeneration));
  }
}

void Sliminess::Kill() {
  bExist = false;
  pSlm_->bExist = false;
}

Sliminess::~Sliminess() {
  if (pAdv)
    --pAdv->nSlimeNum;
}

MegaSliminess::MegaSliminess(Point p_, LevelController *pAdv_)
    : p(p_), pAdv(pAdv_), pSlm_() {
  ImageSequence seq = pAdv->pGl->GetImgSeq("megaslime_reproduce");

  pSlm_ = std::make_unique<AnimationOnce>(2.F, seq, int(.1F * nFramesInSecond), p_, true);

  pAdv->pGl->PlaySound("slime_spawn");
}

void MegaSliminess::Update() {
  if (!pSlm_->bExist) {
    bExist = false;

    pAdv->AddMegaSlime(
        std::make_unique<MegaSlime>(p, pAdv->rBound, pAdv));
  }
}

void MegaSliminess::Kill() {
  bExist = false;
  if (pSlm_)
    pSlm_->bExist = false;
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
    bExist = false;
}

void Mage::Update() {
  if (bAngry) {
    if (!bCasting) {
      int i = 0;
      for (; i < (int)pAc->vCs.size(); ++i) {
        fPoint p = pAc->vCs[i]->GetPosition() - fPos;
        if (p.Length() < nSummonRadius)
          break;
      }
      if (i == (int)pAc->vCs.size())
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
    fPoint f = RandomAngle();
    f.Normalize(10);

    pAc->AddSliminess(std::make_unique<Sliminess>(
        GetPosition() + f.ToPnt(), pAc, true, 0));
  }
}

Castle::Castle(Point p, Rectangle rBound_, LevelController *pAv_)
    : Critter(15, p, Point(), rBound_, 3, pAv_->pGl->GetImgSeq("castle")),
      nPrincesses(0), pAv(pAv_), pDrag(), bBroken(false) {}

void Castle::OnKnight(char cWhat) {
  if (pAv->bCh)
    return;

  if (!nPrincesses || cWhat == 'W') {
    if (!bBroken) {
      pAv->pGl->PlaySound("destroy_castle_sound");
      pAv->pSc->nTheme = -1;
      Critter::seq = pAv->pGl->GetImgSeq("destroy_castle");
    }

    if (pAv->tLoseTimer.nPeriod == 0)
      pAv->tLoseTimer = Timer(nFramesInSecond * 3);

    bBroken = true;
    nPrincesses = 0;

    if (pDrag != nullptr) {

      pDrag->bFly = true;
      pDrag->bTookOff = true;

      pDrag->SimpleVisualEntity::seq = pDrag->imgFly;
      pDrag->SimpleVisualEntity::dPriority = 5;

      pDrag->pCs = nullptr;

      pDrag->fVel = pAv->pt.GetDirection(GetPosition());
      if (pDrag->fVel.Length() == 0)
        pDrag->fVel = fPoint(0, -1);
      pDrag->fVel.Normalize(pDrag->leash.speed);

      pDrag = nullptr;
    }

    return;
  }

  if (pDrag != nullptr) {
    pAv->pGl->PlaySound("one_princess");

    --nPrincesses;

    if (cWhat == 'K') {
      fPoint v = RandomAngle();
      v.Normalize(fPrincessSpeed * 3.F);

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
  Critter::seq.nActive = nPrincesses;

  if (nPrincesses > 4)
    Critter::seq.nActive = 4;

  if (bBroken) {
    Critter::seq.nActive = pAv->tLoseTimer.nTimer / 2;
    if (seq.nActive > (int)seq.vImage.size() - 1)
      seq.nActive = (int)seq.vImage.size() - 1;
  }

  Critter::Draw(pDr);
}
