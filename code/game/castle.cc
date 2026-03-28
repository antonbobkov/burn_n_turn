#include "castle.h"
#include "critters/princess.h"
#include "dragon.h"
#include "controller/dragon_game_controller.h"
#include "controller/level_controller.h"
#include "dragon_constants.h"
#include "../utils/random_utils.h"
#include <memory>

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
