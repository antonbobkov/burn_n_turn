#include "entities.h"

void TextDrawEntity::Draw(SP<ScalingDrawer> pDr) {
  Point p = pos;
  for (unsigned i = 0; i < vText.size(); ++i) {
    pNum->DrawWord(vText[i], p, bCenter);
    p.y += 7;
  }
}

void SimpleVisualEntity::Update() {
  if (bTimer) {
    if (t.Tick()) {
      seq.Toggle();
      t = Timer(nPeriod * seq.GetTime());
    }
  } else if (bStep) {
    Point p = GetPosition();
    if (p != pPrev) {
      bImageToggle = !bImageToggle;

      if (bImageToggle)
        seq.Toggle();
    }
    pPrev = p;
  }
}

void SimpleVisualEntity::Draw(SP<ScalingDrawer> pDr) {
  pDr->Draw(seq.GetImage(), GetPosition(), bCenter);
}

void StaticImage::Draw(SP<ScalingDrawer> pDr) {
  pDr->Draw(img, GetPosition(), bCentered);
}

void StaticRectangle::Draw(SP<ScalingDrawer> pDr) {
  pDr->pGr->DrawRectangle(r, c, false);
}

void SimpleSoundEntity::Update() {
  if (t.Tick()) {
    pSnd->PlaySound(seq.GetSound());

    if (seq.nActive == seq.vSounds.size() - 1) {
      bExist = false;
      return;
    }

    seq.Toggle();
    t = Timer(nPeriod * seq.GetTime());
  }
}

void AnimationOnce::Update() {
  SimpleVisualEntity::Update();

  if (SimpleVisualEntity::seq.nActive == 0) {
    if (!bOnce)
      bExist = false;
  } else {
    bOnce = false;
  }
}

bool PhysicalEntity::HitDetection(SP<PhysicalEntity> pPh) {
  Point d = GetPosition() - pPh->GetPosition();
  unsigned r1 = GetRadius(), r2 = pPh->GetRadius();
  return unsigned(d.x * d.x + d.y * d.y) < (r1 * r1 + r2 * r2);
}

void Critter::Move() {
  fPos += fVel;
  if (!InsideRectangle(rBound, fPos.ToPnt())) {
    if (bDieOnExit)
      bExist = false;
    else {
      if (InsideRectangle(rBound, (fPos - fPoint(0, fVel.y)).ToPnt()))
        fPos.y -= fVel.y;
      else if (InsideRectangle(rBound, (fPos - fPoint(fVel.x, 0)).ToPnt()))
        fPos.x -= fVel.x;
      else
        fPos -= fVel;
    }
  }
}

void FancyCritter::Move() {
  if (tm.Tick()) {
    fPos += fVel;
    seq.Toggle();
  }

  if (!InsideRectangle(rBound, fPos.ToPnt())) {
    if (bDieOnExit)
      bExist = false;
    else {
      if (InsideRectangle(rBound, (fPos - fPoint(0, fVel.y)).ToPnt()))
        fPos.y -= fVel.y;
      else if (InsideRectangle(rBound, (fPos - fPoint(fVel.x, 0)).ToPnt()))
        fPos.x -= fVel.x;
      else
        fPos -= fVel;
    }
  }
}
