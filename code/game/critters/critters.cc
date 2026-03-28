#include "critters.h"

void Critter::Move() {
  fPos += fVel;
  if (!InsideRectangle(rBound, fPos.ToPnt())) {
    if (bDieOnExit)
      this->Destroy();
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
      this->Destroy();
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
