#include "dragon_constants.h"
#include "dragon_macros.h"
#include "fireball.h"
#include "game/controller/dragon_game_controller.h"
#include "game/controller/level_controller.h"
#include "game/critters.h"
#include "game/entities.h"
#include "game/level.h"
#include "game_utils/draw_utils.h"
#include "game_utils/image_sequence.h"
#include "utils/smart_pointer.h"
#include "wrappers/geometry.h"

#include <map>
#include <ostream>
#include <string>

template <class T>
static void Union(std::map<std::string, T> &TarMap,
                  const std::map<std::string, T> &srcMap) {
  for (typename std::map<std::string, T>::const_iterator itr = srcMap.begin(),
                                                         etr = srcMap.end();
       itr != etr; ++itr)
    TarMap[itr->first] += itr->second;
}

static void Union(std::map<std::string, bool> &TarMap,
                  const std::map<std::string, bool> &srcMap) {
  for (std::map<std::string, bool>::const_iterator itr = srcMap.begin(),
                                                   etr = srcMap.end();
       itr != etr; ++itr)
    TarMap[itr->first] |= itr->second;
}

template <class T>
static std::ostream &Out(std::ostream &ofs,
                         const std::map<std::string, T> &srcMap) {
  for (typename std::map<std::string, T>::const_iterator itr = srcMap.begin(),
                                                         etr = srcMap.end();
       itr != etr; ++itr)
    ofs << itr->first << " = " << itr->second << "; ";
  return ofs;
}

FireballBonus::FireballBonus(int nNum_, bool bDef) : nNum(nNum_) {
  if (bDef) {
    fMap["speed"] = 5;
    fMap["frequency"] = fInitialFrequency;
    uMap["pershot"] = 1;
    uMap["total"] = nInitialFireballs;
  }
}

FireballBonus &FireballBonus::operator+=(const FireballBonus &f) {
  Union(fMap, f.fMap);
  Union(uMap, f.uMap);
  Union(bMap, f.bMap);

  return *this;
}

std::ostream &operator<<(std::ostream &ofs, FireballBonus b) {
  Out(ofs, b.fMap) << "\n";
  Out(ofs, b.uMap) << "\n";
  Out(ofs, b.bMap) << "\n";

  return ofs;
}

Chain Chain::Evolve() {
  if (bInfinite)
    return Chain(true);
  else if (nGeneration == 0)
    return Chain();
  else
    return Chain(nGeneration - 1);
}

void ChainExplosion::Update() {
  if (SimpleVisualEntity::t.Check()) {
    r += delta;
  }

  CleanUp(pBc->lsPpl);

  for (std::list<smart_pointer<ConsumableEntity>>::iterator itr =
           pBc->lsPpl.begin();
       itr != pBc->lsPpl.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    if (this->HitDetection(*itr)) {
      if ((*itr)->GetType() == 'W')
        continue;
      if ((*itr)->GetType() == 'E')
        continue;

      (*itr)->OnHit('F');

      if (!ch.IsLast()) {
        smart_pointer<ChainExplosion> pCE = make_smart(
            new ChainExplosion(AnimationOnce(GetPriority(), Reset(seq),
                                             SimpleVisualEntity::t.nPeriod,
                                             (*itr)->GetPosition(), true),
                               r_in, delta, pBc, ch.Evolve()));
        pBc->AddBoth(pCE);
      }
    }
  }

  AnimationOnce::Update();
}

void KnightOnFire::RandomizeVelocity() {
  fVel = RandomAngle();
  fVel.Normalize((float(rand()) / RAND_MAX + .5F) * fKnightFireSpeed);
}

KnightOnFire::KnightOnFire(const Critter &cr, EntityListController *pBc_,
                           unsigned nTimer_, Chain c_)
    : Critter(cr), pBc(pBc_), nTimer(nTimer_), nTimer_i(nTimer_), c(c_),
      t(nFramesInSecond / 5) {
  Critter::seq = pBc->pGl->GetImgSeq("knight_fire");
  RandomizeVelocity();
}

void KnightOnFire::Update() {
  Critter::Update();

  for (std::list<smart_pointer<ConsumableEntity>>::iterator itr =
           pBc->lsPpl.begin();
       itr != pBc->lsPpl.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    if (this->HitDetection(*itr)) {
      char cType = (*itr)->GetType();

      if (cType == 'W' || cType == 'E')
        continue;

      if (c.IsLast() || cType != 'K')
        (*itr)->OnHit('F');
      else {
        (*itr)->bExist = false;
        smart_pointer<KnightOnFire> pKn = make_smart(new KnightOnFire(
            Critter(GetRadius(), (*itr)->GetPosition(), fPoint(), rBound,
                    GetPriority(), ImageSequence(), true),
            pBc, nTimer_i, c.Evolve()));
        pBc->AddBoth(pKn);
      }
    }
  }

  if (nTimer != 0 && --nTimer == 0) {
    bExist = false;

    smart_pointer<AnimationOnce> pAn = make_smart(new AnimationOnce(
        dPriority, pBc->pGl->GetImgSeq("knight_die"),
        unsigned(nFramesInSecond / 5 / fDeathMultiplier), GetPosition(), true));
    pBc->AddBoth(pAn);
  }

  if (t.Tick() && float(rand()) / RAND_MAX < .25)
    RandomizeVelocity();
}

int GetFireballRaduis(FireballBonus &fb) {
  int n = fb.uMap["big"];

  if (n == 0)
    return 6;
  else if (n == 1)
    return 9;
  else if (n == 2)
    return 12;
  else
    return 18;
}

std::string GetSizeSuffix(FireballBonus &fb) {
  int n = fb.uMap["big"];

  if (n == 0)
    return "";
  else if (n == 1)
    return "_15";
  else if (n == 2)
    return "_2";
  else
    return "_3";
}

float GetExplosionInitialRaduis(FireballBonus &fb) {
  int n = fb.uMap["big"];
  float fCf;

  if (n == 0)
    fCf = 1;
  else if (n == 1)
    fCf = 1.5;
  else if (n == 2)
    fCf = 2;
  else
    fCf = 3;

  return 3 * fCf;
}

float GetExplosionExpansionRate(FireballBonus &fb) {
  int n = fb.uMap["big"];
  float fCf;

  if (n == 0)
    fCf = 1;
  else if (n == 1)
    fCf = 1.5;
  else if (n == 2)
    fCf = 2;
  else
    fCf = 3;

  return 3.9F * fCf;
}

unsigned GetFireballChainNum(FireballBonus &fb) {
  int nRet = fb.uMap["fireballchainnum"];

  if (nRet != 0)
    ++nRet;

  return nRet;
}

Fireball::Fireball(Point p, fPoint v, LevelController *pBc_, FireballBonus &fb_,
                   Chain ch_, unsigned nChain_)
    : Critter(GetFireballRaduis(fb_), p, v, pBc_->rBound, 5.F, ImageSequence(),
              nFramesInSecond / 10),
      pBc(pBc_), fb(fb_), ch(ch_), nChain(nChain_) {
  Critter::fVel.Normalize(fb.fMap["speed"]);

  if (!fb.bMap["laser"])
    Critter::seq = pBc->pGl->GetImgSeq("fireball" + GetSizeSuffix(fb));
  else {
    Polar pol(Critter::fVel);
    unsigned n = DiscreetAngle(pol.a, 16);
    Critter::seq = ImageSequence(
        pBc->pGl->GetImgSeq("laser" + GetSizeSuffix(fb)).vImage[n]);
  }
}

void Fireball::Update() {
  CleanUp(pBc->lsPpl);

  bool bMultiHit = false;

  for (std::list<smart_pointer<ConsumableEntity>>::iterator itr =
           pBc->lsPpl.begin();
       itr != pBc->lsPpl.end(); ++itr) {
    if (!(*itr)->bExist)
      continue;

    if (this->HitDetection(*itr)) {
      char cType = (*itr)->GetType();

      if (cType == 'W' || cType == 'E') {
        (*itr)->OnHit('F');

        bExist = false;
        return;
      } else
        pBc->pGl->pSnd->PlaySound(pBc->pGl->GetSnd("death"));

      if ((*itr)->GetType() != 'K' || (fb.uMap["setonfire"] == 0))
        (*itr)->OnHit('F');
      else {
        (*itr)->bExist = false;
        smart_pointer<KnightOnFire> pKn = make_smart(new KnightOnFire(
            Critter((*itr)->GetRadius(), (*itr)->GetPosition(), fPoint(),
                    rBound, 1.F, ImageSequence(), true),
            pBc, 15 * nFramesInSecond, Chain(fb.uMap["setonfire"])));
        pBc->AddBoth(pKn);
      }

      if (!bMultiHit) {
        bool bKeepGoing = (fb.uMap["through"] != 0) || fb.bMap["through_flag"];

        if (nChain != 0 || !bKeepGoing)
          bExist = false;

        if (bKeepGoing) {
          if (fb.bMap["through_flag"])
            fb.bMap["through_flag"] = false;
          else
            --fb.uMap["through"];
        }

        if (bKeepGoing) {
          fPoint v = fVel;

          for (unsigned i = 0; i < nChain; ++i) {
            smart_pointer<Fireball> pFb = make_smart(new Fireball(
                (*itr)->GetPosition(), GetWedgeAngle(v, 1.F / 6, i, nChain),
                pBc, fb, Chain(), nChain));
            pBc->AddBoth(pFb);
          }
        }

        if (fb.uMap["explode"] > 0) {
          smart_pointer<ChainExplosion> pEx;

          if (!fb.bMap["laser"]) {
            pEx = make_smart(new ChainExplosion(
                AnimationOnce(
                    GetPriority(),
                    pBc->pGl->GetImgSeq("explosion" + GetSizeSuffix(fb)),
                    nFramesInSecond / 10, (*itr)->GetPosition(), true),
                GetExplosionInitialRaduis(fb), GetExplosionExpansionRate(fb),
                pBc, Chain(fb.uMap["explode"] - 1)));
          } else {
            pEx = make_smart(new ChainExplosion(
                AnimationOnce(
                    GetPriority(),
                    pBc->pGl->GetImgSeq("laser_expl" + GetSizeSuffix(fb)),
                    nFramesInSecond / 10, (*itr)->GetPosition(), true),
                GetExplosionInitialRaduis(fb), GetExplosionExpansionRate(fb),
                pBc, Chain(fb.uMap["explode"] - 1)));
          }

          pBc->AddBoth(pEx);

          pBc->pGl->pSnd->PlaySound(pBc->pGl->GetSnd("explosion"));
        }
      }

      bMultiHit = true;
    }
  }

  Critter::Update();
}

void TimedFireballBonus::Update() {
  if (t.Tick())
    bExist = false;
}

void CircularFireball::Update() {
  Fireball::Update();

  if (t.Tick())
    bExist = false;

  fPoint p = fPoint(GetPosition()) - i_pos;
  if (p.Length() < fRadius)
    return;
  fPoint fPen(-p.y, p.x);
  p.Normalize((p.Length() - fRadius) * 2);
  fPen -= p;

  fPen.Normalize(fVel.Length());
  fVel = fPen;

  if (fb.bMap["laser"]) {
    Polar pol(Critter::fVel);
    unsigned n = DiscreetAngle(pol.a, 16);
    Critter::seq = ImageSequence(
        pBc->pGl->GetImgSeq("laser" + GetSizeSuffix(fb)).vImage[n]);
  }
}

FireballBonusAnimation::FireballBonusAnimation(Point p_, unsigned n_,
                                               LevelController *pAd_)
    : Animation(.5F, ImageSequence(), nFramesInSecond / 10, p_, true), n(n_),
      bBlink(false), pAd(pAd_), tm(nBonusOnGroundTime), sUnderText("") {
  seq = pAd->pGl->GetImgSeq(GetBonusImage(n));
  coronaSeq = pAd->pGl->GetImgSeq("corona");
}

void FireballBonusAnimation::Draw(smart_pointer<ScalingDrawer> pDr) {
  pDr->Draw(coronaSeq.GetImage(), GetPosition(), bCenter);
  Animation::Draw(pDr);

  Point p = GetPosition();
  p.y += 13;

#ifdef UNDERLINE_UNIT_TEXT
  if (sUnderText != "")
    pAd->pGl->pNum->DrawWord(sUnderText, p, true);
#endif
}

void FireballBonusAnimation::Update() {
  coronaSeq.Toggle();
  Animation::Update();

  if (tm.Tick())
    bExist = false;

  if (!bBlink && tm.nPeriod && (tm.nPeriod - tm.nTimer) < 7 * nFramesInSecond) {
    bBlink = true;

    ImageSequence img;

    unsigned nSz = seq.vImage.size();

    for (unsigned i = 0; i < nSz; ++i) {
      int nLm = 1;
      if (seq.vIntervals.size() > i)
        nLm = seq.vIntervals[i];
      if (nLm == 0)
        nLm = 1;
      for (int j = 0; j < nLm; ++j) {
        img.Add(seq.vImage[i]);
        img.Add(pAd->pGl->GetImg("empty"));
      }
    }

    seq = img;
  }
}
