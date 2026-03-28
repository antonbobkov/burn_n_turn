#ifndef TOWER_DEFENSE_CRITTERS_CRITTERS_H
#define TOWER_DEFENSE_CRITTERS_CRITTERS_H

#include "../entities.h"
#include <string>

/** A creature that moves: place, speed, bounds, and radius; Move() steps it
 * and clamps or removes it when it leaves the realm.
 * May also be struck (OnHit), have a kind (GetType), and show an icon
 * (GetImage) — default no-ops for critters that are not consumable. */
class Critter : public SimpleVisualEntity {
public:
  std::string get_class_name() override { return "Critter"; }

  int GetRadius() override { return nRadius; }
  Point GetPosition() override { return fPos.ToPnt(); }
  void Move() override;

  virtual std::string GetType() { return ""; }
  virtual void OnHit(char /*cWhat*/) {}
  virtual Index GetImage() { return Index(); }

  fPoint GetFPos() const { return fPos; }
  fPoint GetVel() const { return fVel; }
  void SetVel(fPoint v) { fVel = v; }
  void SetUnderText(std::string s) { sUnderText = s; }

  Critter(int nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
          float dPriority, const ImageSequence &seq, int nPeriod)
      : SimpleVisualEntity(dPriority, seq, true, nPeriod), nRadius(nRadius_),
        fPos(fPos_), fVel(fVel_), rBound(rBound_), bDieOnExit(true),
        sUnderText("") {}

  Critter(int nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
          float dPriority, const ImageSequence &seq, bool /*bStep*/ = false)
      : SimpleVisualEntity(dPriority, seq, true, true), nRadius(nRadius_),
        fPos(fPos_), fVel(fVel_), rBound(rBound_), bDieOnExit(true),
        sUnderText("") {}

protected:
  int nRadius;
  fPoint fPos;
  fPoint fVel;

  Rectangle rBound;
  bool bDieOnExit;

  std::string sUnderText;
};

/** A creature that steps and flips frames by a timer. */
class FancyCritter : public SimpleVisualEntity {
public:
  std::string get_class_name() override { return "FancyCritter"; }

  int GetRadius() override { return nRadius; }
  Point GetPosition() override { return fPos.ToPnt(); }
  void Move() override;

  FancyCritter(int nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
               float dPriority, const ImageSequence &seq, int nPeriod)
      : SimpleVisualEntity(dPriority, seq, true, false), nRadius(nRadius_),
        fPos(fPos_), fVel(fVel_), rBound(rBound_), bDieOnExit(true),
        tm(nPeriod) {}

private:
  int nRadius;
  fPoint fPos;
  fPoint fVel;

  Rectangle rBound;
  bool bDieOnExit;

  Timer tm;
};

#endif
