#ifndef TOWER_DEFENSE_ENTITIES_H
#define TOWER_DEFENSE_ENTITIES_H

#include "core.h"
#include "smart_pointer.h"

/** Root entity; bExist flag, virtual dtor. */
struct Entity : virtual public SP_Info {
  bool bExist;
  Entity() : bExist(true) {}
  virtual ~Entity() {}
};

/** Entity that can Move and Update each frame. */
struct EventEntity : virtual public Entity {
  virtual void Move() {}
  virtual void Update() {}
};

/** Entity with a screen position (GetPosition). */
struct ScreenEntity : virtual public Entity {
  virtual Point GetPosition() { return Point(0, 0); }
};

struct VisualEntity : virtual public ScreenEntity {
  virtual void Draw(smart_pointer<ScalingDrawer> pDr) {}
  virtual float GetPriority() { return 0; }
  VisualEntity() = default;
  VisualEntity(const VisualEntity &) = default;
  VisualEntity &operator=(const VisualEntity &) = default;
  VisualEntity &operator=(VisualEntity &&) = delete;
};

/** VisualEntity that draws multi-line text via NumberDrawer at a position. */
struct TextDrawEntity : virtual public VisualEntity {
  float dPriority;
  Point pos;
  bool bCenter;
  std::vector<std::string> vText;
  SSP<NumberDrawer> pNum;

  TextDrawEntity(float dPriority_, Point pos_, bool bCenter_, std::string sText,
                 smart_pointer<NumberDrawer> pNum_)
      : dPriority(dPriority_), pos(pos_), bCenter(bCenter_), pNum(this, pNum_) {
    SetText(sText);
  }

  void SetText(std::string sText) { vText = BreakUpString(sText); }

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return dPriority; }
};

/** VisualEntity with an ImageSequence: draws current frame, Update toggles by
 * timer or on position change. */
struct SimpleVisualEntity : virtual public EventEntity, public VisualEntity {
  float dPriority;

  unsigned nPeriod;
  Timer t;

  bool bTimer, bStep, bCenter;

  Point pPrev;

  ImageSequence seq;

  SimpleVisualEntity(float dPriority_, const ImageSequence &seq_, bool bCenter_,
                     unsigned nPeriod_)
      : dPriority(dPriority_), seq(seq_), nPeriod(nPeriod_),
        t(nPeriod_ * seq_.GetTime()), bCenter(bCenter_), bTimer(true),
        bStep(false), bImageToggle(false) {}

  SimpleVisualEntity(float dPriority_, const ImageSequence &seq_, bool bCenter_,
                     bool bStep_ = false)
      : dPriority(dPriority_), seq(seq_), bCenter(bCenter_), bTimer(false),
        bStep(bStep_), nPeriod(1), bImageToggle(false) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return dPriority; }

  bool bImageToggle;

  /*virtual*/ void Update();
};

/** EventEntity that plays a SoundSequence on a timer; sets bExist false when
 * sequence ends. */
struct SimpleSoundEntity : virtual public EventEntity {
  unsigned nPeriod;
  Timer t;

  SoundSequence seq;
  SSP<SoundInterfaceProxy> pSnd;

  SimpleSoundEntity(const SoundSequence &seq_, unsigned nPeriod_,
                    smart_pointer<SoundInterfaceProxy> pSnd_)
      : seq(seq_), nPeriod(nPeriod_), t(nPeriod * seq_.GetTime()),
        pSnd(this, pSnd_) {}

  /*virtual*/ void Update();
};

/** SimpleVisualEntity with fixed position (no movement). */
struct Animation : public SimpleVisualEntity {
  Point pos;

  Animation(float dPriority_, const ImageSequence &seq, unsigned nTimeMeasure_,
            Point p, bool bCenter = false)
      : SimpleVisualEntity(dPriority_, seq, bCenter, nTimeMeasure_), pos(p) {}

  /*virtual*/ Point GetPosition() { return pos; }
};

/** Animation that runs once then sets bExist false (seq plays to end). */
struct AnimationOnce : public SimpleVisualEntity {
  Point pos;
  bool bOnce;

  AnimationOnce(float dPriority_, const ImageSequence &seq,
                unsigned nTimeMeasure_, Point p, bool bCenter = false)
      : SimpleVisualEntity(dPriority_, seq, bCenter, nTimeMeasure_), pos(p),
        bOnce(true) {}

  /*virtual*/ Point GetPosition() { return pos; }
  /*virtual*/ void Update();
};

/** VisualEntity that draws a single image at a fixed point. */
struct StaticImage : public VisualEntity {
  Index img;
  float dPriority;
  Point p;
  bool bCentered;

  StaticImage(Index img_, Point p_ = Point(0, 0), bool bCentered_ = false,
              float dPriority_ = 0)
      : img(img_), dPriority(dPriority_), p(p_), bCentered(bCentered_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ Point GetPosition() { return p; }

  /*virtual*/ float GetPriority() { return dPriority; }
};

/** VisualEntity that draws a filled rectangle (no position). */
struct StaticRectangle : public VisualEntity {
  float dPriority;
  Rectangle r;
  Color c;

  StaticRectangle(Rectangle r_, Color c_, float dPriority_ = 0)
      : r(r_), c(c_), dPriority(dPriority_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ Point GetPosition() { return Point(0, 0); }

  /*virtual*/ float GetPriority() { return dPriority; }
};

/** ScreenEntity with radius for hit detection (HitDetection). */
struct PhysicalEntity : virtual public ScreenEntity {
  virtual unsigned GetRadius() { return 0; }

  bool HitDetection(smart_pointer<PhysicalEntity> pPh);
};

/** PhysicalEntity that can be hit (OnHit), has type (GetType) and image
 * (GetImage). */
struct ConsumableEntity : virtual public PhysicalEntity {
  virtual char GetType() = 0;
  virtual void OnHit(char cWhat) = 0;
  virtual Index GetImage() = 0;
};

/** Moving unit: position, velocity, bounds, radius; Move() steps and clamps or
 * kills on exit. */
struct Critter : virtual public PhysicalEntity, public SimpleVisualEntity {
  unsigned nRadius;
  fPoint fPos;
  fPoint fVel;

  Rectangle rBound;
  bool bDieOnExit;

  std::string sUnderText;

  /*virtual*/ unsigned int GetRadius() { return nRadius; }
  /*virtual*/ Point GetPosition() { return fPos.ToPnt(); }
  /*virtual*/ void Move();

  Critter(unsigned nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
          float dPriority, const ImageSequence &seq, unsigned nPeriod)
      : SimpleVisualEntity(dPriority, seq, true, nPeriod), rBound(rBound_),
        nRadius(nRadius_), fPos(fPos_), fVel(fVel_), bDieOnExit(true),
        sUnderText("") {}

  Critter(unsigned nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
          float dPriority, const ImageSequence &seq, bool bStep = false)
      : SimpleVisualEntity(dPriority, seq, true, true), rBound(rBound_),
        nRadius(nRadius_), fPos(fPos_), fVel(fVel_), bDieOnExit(true),
        sUnderText("") {}
};

/** Critter that advances position and toggles frame on a timer (tm). */
struct FancyCritter : virtual public PhysicalEntity, public SimpleVisualEntity {
  unsigned nRadius;
  fPoint fPos;
  fPoint fVel;

  Rectangle rBound;
  bool bDieOnExit;

  Timer tm;

  /*virtual*/ unsigned int GetRadius() { return nRadius; }
  /*virtual*/ Point GetPosition() { return fPos.ToPnt(); }
  /*virtual*/ void Move();

  FancyCritter(unsigned nRadius_, fPoint fPos_, fPoint fVel_, Rectangle rBound_,
               float dPriority, const ImageSequence &seq, unsigned nPeriod)
      : SimpleVisualEntity(dPriority, seq, true, false), rBound(rBound_),
        nRadius(nRadius_), fPos(fPos_), fVel(fVel_), bDieOnExit(true),
        tm(nPeriod) {}
};

/** Sort key for draw order: priority and height (operator<). */
struct ScreenPos {
  float fPriority;
  int nHeight;

  ScreenPos(float fPriority_, Point p) : fPriority(fPriority_), nHeight(p.y) {}

  bool operator<(const ScreenPos &sp) const {
    if (fPriority == sp.fPriority)
      return nHeight < sp.nHeight;
    return fPriority < sp.fPriority;
  }
};

#endif
