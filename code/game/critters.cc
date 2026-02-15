#include "game.h"
#include "game/controller/level_controller.h"
#include "game_utils/draw_utils.h"
#include "game_utils/image_sequence.h"
#include "utils/random_utils.h"
#include "utils/smart_pointer.h"
#include "dragon_constants.h"
#include "dragon_macros.h"


void SummonSkeletons(LevelController *pAc, Point p) {
  int nNum = 4;

  if (pAc->nLvl > 6)
    nNum = 6;
  if (pAc->nLvl >= 10)
    nNum = 8;

  for (int i = 0; i < nNum; ++i) {
    fPoint f = GetWedgeAngle(Point(1, 1), 1, i, nNum + 1);
    f.Normalize(15);

    pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("slime_summon"));
    smart_pointer<SkellyGenerator> pSkel =
        make_smart(new SkellyGenerator(p + f.ToPnt(), pAc));
    pAc->AddE(pSkel);
  }
}

void Princess::OnHit(char cWhat) {
  smart_pointer<BonusScore> pB =
      make_smart(new BonusScore(pAc, GetPosition(), 250));
  pAc->AddBoth(pB);

  bExist = false;

  smart_pointer<AnimationOnce> pAn = make_smart(new AnimationOnce(
      GetPriority(),
      fVel.x < 0 ? pAc->pGl->pr("princess_die_f")
                 : pAc->pGl->pr("princess_die"),
      unsigned(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));

  pAc->AddBoth(pAn);
}

void Princess::Draw(smart_pointer<ScalingDrawer> pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
#ifdef UNDERLINE_UNIT_TEXT
  if (sUnderText != "")
    pAc->pGl->pNum->DrawWord(sUnderText, p, true);
#endif
}

Mage::Mage(const Critter &cr, LevelController *pAc_, bool bAngry_)
    : Critter(cr), pAc(pAc_), bAngry(bAngry_), bCasting(false),
      tUntilSpell(GetTimeUntillSpell()), tSpell(3 * nFramesInSecond),
      tSpellAnimate(unsigned(.7F * nFramesInSecond)) {
  fMvVel = Critter::fVel;

  bAngry = true;
  pAc->pGl->bAngry = true;
}

void Mage::OnHit(char cWhat) {
  bExist = false;

  smart_pointer<AnimationOnce> pAn = make_smart(new AnimationOnce(
      GetPriority(),
      fVel.x < 0 ? pAc->pGl->pr("mage_die_f") : pAc->pGl->pr("mage_die"),
      unsigned(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));

  pAc->AddBoth(pAn);

  pAc->pGl->bAngry = true;

  if (pAc->nLvl > 6)
    SummonSlimes();
}

ImageSequence GetBonusImage(int n, Preloader &pr) {
  if (n == 0)
    return pr("void_bonus");
  if (n == 1)
    return pr("pershot_bonus");
  if (n == 2)
    return pr("laser_bonus");
  if (n == 3)
    return pr("big_bonus");
  if (n == 4)
    return pr("totnum_bonus");
  if (n == 5)
    return pr("explode_bonus");
  if (n == 6)
    return pr("split_bonus");
  if (n == 7)
    return pr("burning_bonus");
  if (n == 8)
    return pr("ring_bonus");
  if (n == 9)
    return pr("nuke_bonus");
  if (n == 10)
    return pr("speed_bonus");

  return pr("void_bonus");
}

unsigned RandomBonus(bool bInTower) {
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

void Trader::OnHit(char cWhat) {
  smart_pointer<BonusScore> pB =
      make_smart(new BonusScore(pAc, GetPosition(), 60));
  pAc->AddBoth(pB);

  bExist = false;

  pAc->tutTwo->TraderKilled();

  smart_pointer<AnimationOnce> pAn = make_smart(new AnimationOnce(
      GetPriority(),
      fVel.x < 0 ? pAc->pGl->pr("trader_die") : pAc->pGl->pr("trader_die_f"),
      unsigned(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));

  pAc->AddBoth(pAn);

  smart_pointer<FireballBonusAnimation> pFb = make_smart(
      new FireballBonusAnimation(GetPosition(), RandomBonus(false), pAc));
  if (bFirstBns) {
    pFb->sUnderText = "loot";
    bFirstBns = false;
  }
  pAc->AddBoth(pFb);
  pAc->lsBonus.push_back(pFb);
}

void Trader::Draw(smart_pointer<ScalingDrawer> pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
#ifdef UNDERLINE_UNIT_TEXT
  if (sUnderText != "")
    pAc->pGl->pNum->DrawWord(sUnderText, p, true);
#endif
}

void Knight::Draw(smart_pointer<ScalingDrawer> pDr) {
  Critter::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;
#ifdef UNDERLINE_UNIT_TEXT
  if (sUnderText != "")
    pAc->pGl->pNum->DrawWord(sUnderText, p, true);
#endif
}

void Knight::KnockBack() {
  if (fVel != fPoint(0, 0))
    fPos -= fVel / fVel.Length();
}

void Knight::Update() {
  for (unsigned i = 0; i < pAc->vCs.size(); ++i)
    if (this->HitDetection(pAc->vCs[i].get())) {
      pAc->vCs[i]->OnKnight(GetType());

      bExist = false;
      break;
    }

  if (cType == 'S') {
    CleanUp(pAc->lsPpl);

    for (std::list<smart_pointer<ConsumableEntity>>::iterator itr = pAc->lsPpl.begin();
         itr != pAc->lsPpl.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      if (this->HitDetection(*itr)) {

        if ((*itr)->GetType() == 'P' || (*itr)->GetType() == 'T') {
          pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("death"));
          (*itr)->OnHit('S');
        }
      }
    }

    CleanUp(pAc->lsBonus);

    for (std::list<smart_pointer<FireballBonusAnimation>>::iterator itr =
             pAc->lsBonus.begin();
         itr != pAc->lsBonus.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      if (this->HitDetection(*itr)) {
        pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("skeleton_bonus"));
        (*itr)->bExist = false;
      }
    }
  }

  Point p = GetPosition();
  if (p != pPrev) {
    bImageToggle = !bImageToggle;

    if (bImageToggle) {
      seq.Toggle();

      if (seq.nActive == 3)
        pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("step_left"));
      else if (seq.nActive == 6)
        pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("step_right"));
    }
  }
  pPrev = p;
}

void Knight::OnHit(char cWhat) {
  if (cType == 'W') {
    KnockBack();
    if (nGolemHealth > 0) {
      --nGolemHealth;
      pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("hit_golem"));
      return;
    }

    pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("golem_death"));

    smart_pointer<BonusScore> pB =
        make_smart(new BonusScore(pAc, GetPosition(), 5000));
  }

  bExist = false;

  pAc->tutOne->KnightKilled();

  if (cType != 'G') {
    smart_pointer<BonusScore> pB =
        make_smart(new BonusScore(pAc, GetPosition(), 100));
    pAc->AddBoth(pB);

    ImageSequence seqDead = pAc->pGl->pr("knight_die");

    if (cType == 'S')
      seqDead = pAc->pGl->pr("skelly_die");
    else if (cType == 'W') {
      if (this->fVel.x < 0)
        seqDead = pAc->pGl->pr("golem_die");
      else
        seqDead = pAc->pGl->pr("golem_die_f");
    }

    smart_pointer<AnimationOnce> pAn = make_smart(new AnimationOnce(
        dPriority, seqDead, unsigned(nFramesInSecond / 5 / fDeathMultiplier),
        GetPosition(), true));
    pAc->AddBoth(pAn);
  } else {
    smart_pointer<Ghostiness> pGhs =
        make_smart(new Ghostiness(GetPosition(), pAc, *this, nGhostHit));
    pAc->AddE(pGhs);
  }
}

MegaSlime::MegaSlime(fPoint fPos, Rectangle rBound, LevelController *pAc_)
    : Critter(8, fPos, fPoint(0, 0), rBound, 3, pAc_->pGl->pr("megaslime"),
              nFramesInSecond / 5),
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
  CleanUp(pAc->lsBonus);

  for (std::list<smart_pointer<FireballBonusAnimation>>::iterator itr =
           pAc->lsBonus.begin();
       itr != pAc->lsBonus.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    if (this->HitDetection(*itr)) {
      (*itr)->bExist = false;
      pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_bonus"));
    }
  }

  if (t.Tick()) {
    seq.Toggle();
    t = Timer(nPeriod * seq.GetTime() + rand() % 2);

    if (seq.nActive == 11) {
      pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_jump"));
      RandomizeVelocity();
    } else if (seq.nActive == 16) {
      fVel = fPoint(0, 0);
      pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_land"));
    }
  }
}

void MegaSlime::OnHit(char cWhat) {
  if (nHealth > 0) {
    --nHealth;
    pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_hit"));
    return;
  }

  bExist = false;

  smart_pointer<BonusScore> pB =
      make_smart(new BonusScore(pAc, GetPosition(), 500));
  pAc->AddBoth(pB);

  ImageSequence seqDead = pAc->pGl->pr("megaslime_die");
  pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("megaslime_die"));

  smart_pointer<AnimationOnce> pAn = make_smart(new AnimationOnce(
      dPriority, seqDead, unsigned(nFramesInSecond / 5 / fDeathMultiplier),
      GetPosition(), true));
  pAc->AddBoth(pAn);
}

Ghostiness::Ghostiness(Point p_, LevelController *pAdv_, Critter knCp_,
                       int nGhostHit_)
    : p(p_), pAdv(pAdv_), knCp(knCp_), nGhostHit(nGhostHit_) {
  ImageSequence seq = pAdv->pGl->pr("ghost_knight_burn");
  if (nGhostHit == 0)
    seq = pAdv->pGl->pr("ghost_burn");

  unsigned n = unsigned(.2F * nFramesInSecond / fDeathMultiplier);

  t = Timer(n * seq.GetTotalTime());

  smart_pointer<AnimationOnce> pFire =
      make_smart(new AnimationOnce(2.F, seq, n, p_, true));
  pAdv_->AddBoth(pFire);
}

void Ghostiness::Update() {
  if (t.Tick()) {
    bExist = false;

    if (nGhostHit == 0)
      return;

    smart_pointer<Knight> pCr = make_smart(new Knight(knCp, pAdv, 'G'));
    if (nGhostHit == 1)
      pCr->seq = pAdv->pGl->pr("ghost");
    else
      pCr->seq = pAdv->pGl->pr("ghost_knight");
    pCr->nGhostHit = nGhostHit - 1;

    pAdv->AddBoth(pCr);
    pAdv->lsPpl.push_back(pCr);
  }
}

Slime::Slime(fPoint fPos, Rectangle rBound, LevelController *pAc_,
             int nGeneration_)
    : Critter(5, fPos, fPoint(0, 0), rBound, 3, pAc_->pGl->pr("slime"), true),
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

  for (std::list<smart_pointer<ConsumableEntity>>::iterator itr = pAc->lsPpl.begin();
       itr != pAc->lsPpl.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    if (this->HitDetection(*itr)) {
      if ((*itr)->GetType() == 'K') {
        pAc->pGl->pSnd->PlaySound(pAc->pGl->pr.GetSnd("slime_poke"));

        bExist = false;

        smart_pointer<AnimationOnce> pAn = make_smart(
            new AnimationOnce(dPriority, pAc->pGl->pr("slime_poke"),
                              nFramesInSecond / 5, GetPosition(), true));
        pAc->AddBoth(pAn);

        break;
      }
    }
  }

  Critter::Update(); // can walk off the screen
}

void Slime::OnHit(char cWhat) {
  if (pAc->nSlimeNum >= nSlimeMax && cWhat != 'M') {
    std::vector<Point> vDeadSlimes;

    for (std::list<smart_pointer<Slime>>::iterator itr = pAc->lsSlimes.begin();
         itr != pAc->lsSlimes.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      vDeadSlimes.push_back((*itr)->GetPosition());
      (*itr)->OnHit('M');
    }

    for (std::list<smart_pointer<Sliminess>>::iterator itr = pAc->lsSliminess.begin();
         itr != pAc->lsSliminess.end(); ++itr) {
      if (!(*itr)->bExist)
        continue;

      vDeadSlimes.push_back((*itr)->GetPosition());
      (*itr)->Kill();
    }

    if (vDeadSlimes.empty())
      throw SimpleException("No slimes found!");

    fPoint fAvg(0, 0);
    for (unsigned i = 0; i < vDeadSlimes.size(); ++i) {
      fAvg += vDeadSlimes[i];
    }

    fAvg /= float(vDeadSlimes.size());

    pAc->MegaGeneration(fAvg.ToPnt());

    for (unsigned i = 0; i < vDeadSlimes.size(); ++i) {
      smart_pointer<FloatingSlime> pSlm = make_smart(
          new FloatingSlime(pAc->pGl->pr("slime_cloud"), vDeadSlimes[i],
                            fAvg.ToPnt(), nFramesInSecond * 1));
      pAc->AddBoth(pSlm);
    }

    return;
  }

  bExist = false;

  bool bRevive = (cWhat != 'M');

  if (cWhat != 'M') {
    smart_pointer<BonusScore> pB =
        make_smart(new BonusScore(pAc, GetPosition(), 1));
    pAc->AddBoth(pB);
  }

  smart_pointer<AnimationOnce> pAn = make_smart(new AnimationOnce(
      dPriority, pAc->pGl->pr(bRevive ? "slime_die" : "slime_poke"),
      nFramesInSecond / 5, GetPosition(), true));
  pAc->AddBoth(pAn);

  if (!bRevive)
    return;

  for (int i = 0; i < 2; ++i) {
    fPoint f = RandomAngle();
    f.Normalize(4);

    smart_pointer<Sliminess> pSlm = make_smart(
        new Sliminess(GetPosition() + f.ToPnt(), pAc, false, nGeneration + 1));
    pAc->AddE(pSlm);
    pAc->lsSliminess.push_back(pSlm);
  }
}

Sliminess::Sliminess(Point p_, LevelController *pAdv_, bool bFast_,
                     int nGeneration_)
    : p(p_), pAdv(pAdv_), bFast(bFast_), nGeneration(nGeneration_),
      pSlm() {
  ImageSequence seq = bFast ? pAdv->pGl->pr("slime_reproduce_fast")
                            : pAdv->pGl->pr("slime_reproduce");

  t = bFast ? Timer(unsigned(1.3F * nFramesInSecond))
            : Timer(unsigned(2.3F * nFramesInSecond));

  smart_pointer<AnimationOnce> pSlmTmp = make_smart(
      new AnimationOnce(2.F, seq, unsigned(.1F * nFramesInSecond), p_, true));
  pSlm = pSlmTmp;
  pAdv_->AddBoth(pSlmTmp);

  ++pAdv_->nSlimeNum;
}

void Sliminess::Update() {
  if (t.Tick()) {
    bExist = false;

    smart_pointer<Slime> pSlm =
        make_smart(new Slime(p, pAdv->rBound, pAdv, nGeneration));
    pAdv->AddBoth(pSlm);
    pAdv->lsPpl.push_back(pSlm);
    pAdv->lsSlimes.push_back(pSlm);
  }
}

void Sliminess::Kill() {
  bExist = false;
  pSlm->bExist = false;
}

Sliminess::~Sliminess() {
  if (pAdv)
    --pAdv->nSlimeNum;
}

MegaSliminess::MegaSliminess(Point p_, LevelController *pAdv_)
    : p(p_), pAdv(pAdv_), pSlm() {
  ImageSequence seq = pAdv->pGl->pr("megaslime_reproduce");

  smart_pointer<AnimationOnce> pSlmTmp = make_smart(
      new AnimationOnce(2.F, seq, unsigned(.1F * nFramesInSecond), p_, true));
  pSlm = pSlmTmp;
  pAdv_->AddBoth(pSlmTmp);

  pAdv->pGl->pSnd->PlaySound(pAdv->pGl->pr.GetSnd("slime_spawn"));
}

void MegaSliminess::Update() {
  if (pSlm->bExist == false) {
    bExist = false;

    smart_pointer<MegaSlime> pSlm =
        make_smart(new MegaSlime(p, pAdv->rBound, pAdv));
    pAdv->AddBoth(pSlm);
    pAdv->lsPpl.push_back(pSlm);
  }
}

FloatingSlime::FloatingSlime(ImageSequence seq, Point pStart, Point pEnd,
                             unsigned nTime)
    : SimpleVisualEntity(2.F, seq, true, unsigned(.1F * nFramesInSecond)) {
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
      unsigned i = 0;
      for (; i < pAc->vCs.size(); ++i) {
        fPoint p = pAc->vCs[i]->GetPosition() - fPos;
        if (p.Length() < nSummonRadius)
          break;
      }
      if (i == pAc->vCs.size())
        if (rand() % nSummonChance == 0) {
          bCasting = true;
          Critter::seq = pAc->pGl->pr("mage_spell");
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
        Critter::seq =
            fMvVel.x < 0 ? pAc->pGl->pr("mage_f") : pAc->pGl->pr("mage");
      }
    }
  }

  Critter::Update();
}

void Mage::SummonSlimes() {
  for (int i = 0; i < 2; ++i) {
    fPoint f = RandomAngle();
    f.Normalize(10);

    smart_pointer<Sliminess> pSlm =
        make_smart(new Sliminess(GetPosition() + f.ToPnt(), pAc, true, 0));
    pAc->AddE(pSlm);
  }
}

Castle::Castle(Point p, Rectangle rBound_, LevelController *pAv_)
    : Critter(15, p, Point(), rBound_, 3, pAv_->pGl->pr("castle")),
      nPrincesses(0), pAv(pAv_), pDrag(), bBroken(false) {}

void Castle::OnKnight(char cWhat) {
  if (pAv->bCh)
    return;

  if (!nPrincesses || cWhat == 'W') {
    if (!bBroken) {
      pAv->pGl->pSnd->PlaySound(pAv->pGl->pr.GetSnd("destroy_castle_sound"));
      pAv->pSc->nTheme = -1;
      Critter::seq = pAv->pGl->pr("destroy_castle");
    }

    if (pAv->tLoseTimer.nPeriod == 0)
      pAv->tLoseTimer = Timer(nFramesInSecond * 3);

    bBroken = true;
    nPrincesses = 0;

    if (!pDrag.is_null()) {

      pDrag->bFly = true;
      pDrag->bTookOff = true;

      pDrag->SimpleVisualEntity::seq = pDrag->imgFly;
      pDrag->SimpleVisualEntity::dPriority = 5;

      pDrag->pCs = nullptr;

      pDrag->fVel = pAv->pt.GetDirection(GetPosition());
      if (pDrag->fVel.Length() == 0)
        pDrag->fVel = fPoint(0, -1);
      pDrag->fVel.Normalize(pDrag->leash.speed);

      pDrag = smart_pointer<Dragon>();
    }

    return;
  }

  if (!pDrag.is_null()) {
    pAv->pGl->pSnd->PlaySound(pAv->pGl->pr.GetSnd("one_princess"));

    --nPrincesses;

    if (cWhat == 'K') {
      fPoint v = RandomAngle();
      v.Normalize(fPrincessSpeed * 3.F);

      smart_pointer<Princess> pCr =
          make_smart(new Princess(Critter(7, GetPosition(), v, rBound, 0,
                                          v.x < 0 ? pAv->pGl->pr("princess_f")
                                                  : pAv->pGl->pr("princess"),
                                          true),
                                  pAv));
      pAv->AddBoth(pCr);
      pAv->lsPpl.push_back(pCr);
    }
  } else {
    pAv->pGl->pSnd->PlaySound(pAv->pGl->pr.GetSnd("all_princess_escape"));

    if (cWhat == 'K') {
      float r = float(rand()) / RAND_MAX * 2 * 3.1415F;

      for (unsigned i = 0; i < nPrincesses; ++i) {
        fPoint v(sin(r + i * 2 * 3.1415F / nPrincesses),
                 cos(r + i * 2 * 3.1415F / nPrincesses));
        v.Normalize(fPrincessSpeed * 3.F);

        smart_pointer<Princess> pCr =
            make_smart(new Princess(Critter(7, GetPosition(), v, rBound, 0,
                                            v.x < 0 ? pAv->pGl->pr("princess_f")
                                                    : pAv->pGl->pr("princess"),
                                            true),
                                    pAv));
        pAv->AddBoth(pCr);
        pAv->lsPpl.push_back(pCr);
      }
    }

    nPrincesses = 0;
  }
}

void Castle::Draw(smart_pointer<ScalingDrawer> pDr) {
  Critter::seq.nActive = nPrincesses;

  if (nPrincesses > 4)
    Critter::seq.nActive = 4;

  if (bBroken) {
    Critter::seq.nActive = pAv->tLoseTimer.nTimer / 2;
    if (seq.nActive > seq.vImage.size() - 1)
      seq.nActive = seq.vImage.size() - 1;
  }

  Critter::Draw(pDr);
}
