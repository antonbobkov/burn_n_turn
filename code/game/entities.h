#ifndef TOWER_DEFENSE_ENTITIES_H
#define TOWER_DEFENSE_ENTITIES_H

#include "game_utils/draw_utils.h"
#include "game_utils/image_sequence.h"
#include "game_utils/sound_sequence.h"
#include "game_utils/sound_utils.h"
#include "utils/index.h"
#include "utils/smart_pointer.h"
#include "utils/timer.h"
#include "wrappers/color.h"
#include "wrappers/geometry.h"

#include <list>

struct DragonGameController;
struct LevelController;

/** The root of all things in the realm; a flag says if it still exists. */
struct Entity : virtual public SP_Info {
  std::string get_class_name() override { return "Entity"; }
  bool bExist;
  Entity() : bExist(true) {}
  virtual ~Entity() {}
};

/** A thing that can step and update each tick. */
struct EventEntity : virtual public Entity {
  std::string get_class_name() override { return "EventEntity"; }
  virtual void Move() {}
  virtual void Update() {}
};

/** A thing that has a place on the vista (GetPosition). */
struct ScreenEntity : virtual public Entity {
  std::string get_class_name() override { return "ScreenEntity"; }
  virtual Point GetPosition() { return Point(0, 0); }
};

struct VisualEntity : virtual public ScreenEntity {
  std::string get_class_name() override { return "VisualEntity"; }
  virtual void Draw(smart_pointer<ScalingDrawer> pDr) {}
  virtual float GetPriority() { return 0; }
  VisualEntity() = default;
  VisualEntity(const VisualEntity &) = default;
  VisualEntity &operator=(const VisualEntity &) = default;
  VisualEntity &operator=(VisualEntity &&) = delete;
};

/** A sight that paints lines of text at a point on the vista. */
struct TextDrawEntity : virtual public VisualEntity {
  std::string get_class_name() override { return "TextDrawEntity"; }
  float dPriority;
  Point pos;
  bool bCenter;
  std::vector<std::string> vText;
  smart_pointer<NumberDrawer> pNum;

  TextDrawEntity(float dPriority_, Point pos_, bool bCenter_, std::string sText,
                 smart_pointer<NumberDrawer> pNum_)
      : dPriority(dPriority_), pos(pos_), bCenter(bCenter_), pNum(pNum_) {
    SetText(sText);
  }

  void SetText(std::string sText);

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return dPriority; }
};

/** A sight with a sequence of images: draws the current frame; Update steps
 * by timer or when position changes. */
struct SimpleVisualEntity : virtual public EventEntity, public VisualEntity {
  std::string get_class_name() override { return "SimpleVisualEntity"; }
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

/** A thing that plays a sound sequence on a timer; it vanishes when the tune
 * ends. */
struct SimpleSoundEntity : virtual public EventEntity {
  std::string get_class_name() override { return "SimpleSoundEntity"; }
  unsigned nPeriod;
  Timer t;

  SoundSequence seq;
  smart_pointer<SoundInterfaceProxy> pSnd;

  SimpleSoundEntity(const SoundSequence &seq_, unsigned nPeriod_,
                    smart_pointer<SoundInterfaceProxy> pSnd_)
      : seq(seq_), nPeriod(nPeriod_), t(nPeriod * seq_.GetTime()), pSnd(pSnd_) {
  }

  /*virtual*/ void Update();
};

/** A sight that stays in one place and cycles through frames. */
struct Animation : public SimpleVisualEntity {
  std::string get_class_name() override { return "Animation"; }
  Point pos;

  Animation(float dPriority_, const ImageSequence &seq, unsigned nTimeMeasure_,
            Point p, bool bCenter = false)
      : SimpleVisualEntity(dPriority_, seq, bCenter, nTimeMeasure_), pos(p) {}

  /*virtual*/ Point GetPosition() { return pos; }
};

/** An animation that plays once to the end, then vanishes. */
struct AnimationOnce : public SimpleVisualEntity {
  std::string get_class_name() override { return "AnimationOnce"; }
  Point pos;
  bool bOnce;

  AnimationOnce(float dPriority_, const ImageSequence &seq,
                unsigned nTimeMeasure_, Point p, bool bCenter = false)
      : SimpleVisualEntity(dPriority_, seq, bCenter, nTimeMeasure_), pos(p),
        bOnce(true) {}

  /*virtual*/ Point GetPosition() { return pos; }
  /*virtual*/ void Update();
};

/** A sight that shows one image at a fixed point. */
struct StaticImage : public VisualEntity {
  std::string get_class_name() override { return "StaticImage"; }
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

/** A sight that draws a filled rectangle (covers the vista, no single spot). */
struct StaticRectangle : public VisualEntity {
  std::string get_class_name() override { return "StaticRectangle"; }
  float dPriority;
  Rectangle r;
  Color c;

  StaticRectangle(Rectangle r_, Color c_, float dPriority_ = 0)
      : r(r_), c(c_), dPriority(dPriority_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ Point GetPosition() { return Point(0, 0); }

  /*virtual*/ float GetPriority() { return dPriority; }
};

/** A thing on the vista with a radius—so we can tell when it touches another. */
struct PhysicalEntity : virtual public ScreenEntity {
  std::string get_class_name() override { return "PhysicalEntity"; }
  virtual unsigned GetRadius() { return 0; }

  bool HitDetection(smart_pointer<PhysicalEntity> pPh);
  bool HitDetection(PhysicalEntity *pPh);
};

/** A thing that can be struck (OnHit), has a kind (GetType) and an image
 * (GetImage). */
struct ConsumableEntity : virtual public PhysicalEntity {
  std::string get_class_name() override { return "ConsumableEntity"; }
  virtual char GetType() = 0;
  virtual void OnHit(char cWhat) = 0;
  virtual Index GetImage() = 0;
};

/** A creature that moves: place, speed, bounds, and radius; Move() steps it
 * and clamps or removes it when it leaves the realm. */
struct Critter : virtual public PhysicalEntity, public SimpleVisualEntity {
  std::string get_class_name() override { return "Critter"; }
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

/** A creature that steps and flips frames by a timer. */
struct FancyCritter : virtual public PhysicalEntity, public SimpleVisualEntity {
  std::string get_class_name() override { return "FancyCritter"; }
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

/** How we order what is drawn: by priority and height (operator<). */
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

/** A floating "+N" tally at a point; it lingers a moment then fades. */
struct BonusScore : public EventEntity, public VisualEntity {
  std::string get_class_name() override { return "BonusScore"; }
  LevelController *pAc;
  std::string sText;
  unsigned nScore;
  unsigned nScoreSoFar;
  Point p;
  Timer t;
  unsigned nC;
  Color c;

  BonusScore(LevelController *pAc_, Point p_, unsigned nScore_);

  /*virtual*/ void Update();

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return 5; }

  /*virtual*/ Point GetPosition() { return p; }
};

struct SoundControls : public EventEntity {
  std::string get_class_name() override { return "SoundControls"; }
  BackgroundMusicPlayer &plr;
  int nTheme;

  SoundControls(BackgroundMusicPlayer &plr_, int nTheme_)
      : plr(plr_), nTheme(nTheme_) {}

  /*virtual*/ void Update();
};

/** Paints the finest tally yet within a rectangle on the vista. */
struct HighScoreShower : public VisualEntity {
  std::string get_class_name() override { return "HighScoreShower"; }
  DragonGameController *pGl;
  Rectangle rBound;

  HighScoreShower(DragonGameController *pGl_, Rectangle rBound_)
      : pGl(pGl_), rBound(rBound_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

struct IntroTextShower : public VisualEntity {
  std::string get_class_name() override { return "IntroTextShower"; }
  DragonGameController *pGl;
  Rectangle rBound;

  IntroTextShower(DragonGameController *pGl_, Rectangle rBound_)
      : pGl(pGl_), rBound(rBound_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
};

/** Clear from the list all that have left the realm (bExist is false). */
template <class T> void CleanUp(std::list<T> &ar) {
  for (typename std::list<T>::iterator itr = ar.begin(), etr = ar.end();
       itr != etr;) {
    if (!(*itr)->bExist)
      ar.erase(itr++);
    else
      ++itr;
  }
}

#endif
