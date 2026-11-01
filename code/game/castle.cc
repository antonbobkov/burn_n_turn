#include "castle.h"
#include "critters/royal.h"
#include "dragon.h"
#include "controller/dragon_game_controller.h"
#include "controller/level_controller.h"
#include "dragon_constants.h"
#include "../utils/random_utils.h"
#include <algorithm>
#include <memory>

namespace {

// The tower has but four windows, each a few paces along from the last. These
// paces are counted from the tower's heart, for that is where it stands.
const int kRoyalWindowCount = 4;
const int kFirstWindowX = -7;
const int kWindowSpacingX = 4;
const int kWindowY = -6;

} // namespace

Castle::Castle(Point p, Rectangle rBound_, LevelController *pAv_)
    : Critter(15, p, Point(), rBound_, 3, pAv_->GetGl()->GetImgSeq("castle")),
      pAv(pAv_), pDrag(), bBroken(false) {}

void Castle::AddRoyals(const std::vector<RoyalKind> &royals) {
  vRoyals.insert(vRoyals.end(), royals.begin(), royals.end());
}

void Castle::OnKnight(std::string cWhat) {
  if (pAv->IsCheating())
    return;

  // An empty castle struck by any knight, or any castle struck by a golem, is destroyed.
  // The golem always demolishes regardless of stored royals.
  if (vRoyals.empty() || cWhat == "golem") {
    if (!bBroken) {
      pAv->GetGl()->PlaySound("destroy_castle_sound");
      pAv->StopMusic();
      Critter::seq = pAv->GetGl()->GetImgSeq("destroy_castle");
    }

    // The castle falls! Start the 3-second countdown to game over.
    pAv->StartLoseTimer();

    bBroken = true;
    vRoyals.clear();

    // The dragon must flee a crumbling keep immediately.
    if (pDrag != nullptr) {
      pDrag->TakeOff();
      pDrag = nullptr;
    }

    return;
  }

  if (pDrag != nullptr) {
    // Dragon is perched: only the newest captive flees, leaving their window dark.
    pAv->GetGl()->PlaySound("one_princess");

    RoyalKind fleeing = vRoyals.back();
    vRoyals.pop_back();

    if (cWhat == "knight") {
      fPoint v = fPoint::Normalized(RandomAngle(), fRoyalSpeed * 3.F);

      pAv->AddCritter(MakeRoyal(fleeing, GetPosition(), v, rBound, 0, pAv));
    }
  } else {
    // Dragon is away: all stored royals panic and scatter as live entities.
    pAv->GetGl()->PlaySound("all_princess_escape");

    if (cWhat == "knight") {
      float r = float(rand()) / RAND_MAX * 2 * 3.1415F;

      int nCount = GetRoyalCount();
      for (int i = 0; i < nCount; ++i) {
        fPoint v(sin(r + i * 2 * 3.1415F / nCount),
                 cos(r + i * 2 * 3.1415F / nCount));
        v.Normalize(fRoyalSpeed * 3.F);

        pAv->AddCritter(MakeRoyal(vRoyals[i], GetPosition(), v, rBound, 0, pAv));
      }
    }

    vRoyals.clear();
  }
}

void Castle::Draw(ScalingDrawer *pDr) {
  // First rises the bare stone of the tower, its windows dark and waiting.
  Critter::seq.SetActive(0);

  if (bBroken) {
    Critter::seq.SetActive(pAv->GetLoseTimerFrame());
    if (seq.GetActive() > seq.GetImageCount() - 1)
      seq.SetActive(seq.GetImageCount() - 1);
  }

  Critter::Draw(pDr);

  // Then each captive peers out, left to right, in the order they were taken.
  // Should more than four be held, the latest wait out of sight within.
  const ImageSequence &figures = pAv->GetGl()->GetImgSeq("royal_figures");
  int nShown = std::min(GetRoyalCount(), kRoyalWindowCount);
  for (int i = 0; i < nShown; ++i) {
    Point window(kFirstWindowX + i * kWindowSpacingX, kWindowY);
    pDr->Draw(figures.GetImageAt(static_cast<int>(vRoyals[i])),
              GetPosition() + window, false);
  }
}
