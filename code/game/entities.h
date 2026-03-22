#ifndef TOWER_DEFENSE_ENTITIES_H
#define TOWER_DEFENSE_ENTITIES_H

#include "../game_utils/draw_utils.h"
#include "../game_utils/image_sequence.h"
#include "../game_utils/sound_sequence.h"
#include "../game_utils/sound_utils.h"
#include "../utils/index.h"
#include "../utils/timer.h"
#include "../wrappers/color.h"
#include "../wrappers/geometry.h"

#include <list>

struct DragonGameController;
struct LevelController;

/** The root of all things in the realm; a flag says if it still exists.
 * Combines movement (Move/Update), screen presence (GetPosition), visuals
 * (Draw/GetPriority), and collision (GetRadius/HitDetection). */
struct Entity {
  Entity() : bExist(true) {}
  Entity(const Entity &) = default;
  Entity &operator=(const Entity &) = default;
  Entity &operator=(Entity &&) = delete;
  virtual ~Entity() {}
  virtual std::string get_class_name() { return "Entity"; }
  virtual void Move() {}
  virtual void Update() {}
  virtual Point GetPosition() { return Point(0, 0); }
  virtual void Draw(ScalingDrawer * /*pDr*/) {}
  virtual float GetPriority() { return 0; }
  virtual int GetRadius() { return 0; }
  virtual bool ShouldDraw() { return false; }
  bool HitDetection(Entity *pPh);

  bool IsAlive() const { return bExist; }
  void Kill() { bExist = false; }

private:
  bool bExist;
};

/** A sight that paints lines of text at a point on the vista. */
struct TextDrawEntity : public Entity {
  std::string get_class_name() override { return "TextDrawEntity"; }
  bool ShouldDraw() override { return true; }

  TextDrawEntity(float dPriority_, Point pos_, bool bCenter_, std::string sText,
                 NumberDrawer *pNum_)
      : dPriority(dPriority_), pos(pos_), bCenter(bCenter_), pNum(pNum_) {
    SetText(sText);
  }

  void SetText(std::string sText);

  void Draw(ScalingDrawer *pDr) override;

  float GetPriority() override { return dPriority; }

private:
  float dPriority;
  Point pos;
  bool bCenter;
  std::vector<std::string> vText;
  NumberDrawer *pNum;
};

/** A sight with a sequence of images: draws the current frame; Update steps
 * by timer or when position changes. */
struct SimpleVisualEntity : virtual public Entity {
  std::string get_class_name() override { return "SimpleVisualEntity"; }
  bool ShouldDraw() override { return true; }

  SimpleVisualEntity(float dPriority_, const ImageSequence &seq_, bool bCenter_,
                     int nPeriod_)
      : dPriority(dPriority_), nPeriod(nPeriod_), t(nPeriod_ * seq_.GetTime()),
        bTimer(true), bStep(false), bCenter(bCenter_), seq(seq_),
        bImageToggle(false) {}

  SimpleVisualEntity(float dPriority_, const ImageSequence &seq_, bool bCenter_,
                     bool bStep_ = false)
      : dPriority(dPriority_), nPeriod(1), bTimer(false), bStep(bStep_),
        bCenter(bCenter_), seq(seq_), bImageToggle(false) {}

  void Draw(ScalingDrawer *pDr) override;

  float GetPriority() override { return dPriority; }

  void SetPriority(float d) { dPriority = d; }
  void SetSeq(const ImageSequence &s) { seq = s; }

  void Update() override;

protected:
  float dPriority;

  int nPeriod;
  Timer t;

  bool bTimer, bStep, bCenter;

  Point pPrev;

  ImageSequence seq;

  bool bImageToggle;
};

/** A thing that plays a sound sequence on a timer; it vanishes when the tune
 * ends. */
struct SimpleSoundEntity : public Entity {
  std::string get_class_name() override { return "SimpleSoundEntity"; }

  SimpleSoundEntity(const SoundSequence &seq_, int nPeriod_,
                    SoundInterfaceProxy *pSnd_)
      : nPeriod(nPeriod_), t(nPeriod * seq_.GetTime()), seq(seq_), pSnd(pSnd_) {
  }

  void Update() override;

private:
  int nPeriod;
  Timer t;

  SoundSequence seq;
  SoundInterfaceProxy *pSnd;
};

/** A sight that stays in one place and cycles through frames. */
struct Animation : public SimpleVisualEntity {
  std::string get_class_name() override { return "Animation"; }

  Animation(float dPriority_, const ImageSequence &seq, int nTimeMeasure_,
            Point p, bool bCenter = false)
      : SimpleVisualEntity(dPriority_, seq, bCenter, nTimeMeasure_), pos(p) {}
  Animation(const Animation &) = default;
  Animation &operator=(const Animation &) = default;

  Point GetPosition() override { return pos; }

  void SetPos(Point p_) { pos = p_; }

private:
  Point pos;
};

/** An animation that plays once to the end, then vanishes. */
struct AnimationOnce : public SimpleVisualEntity {
  std::string get_class_name() override { return "AnimationOnce"; }

  AnimationOnce(float dPriority_, const ImageSequence &seq, int nTimeMeasure_,
                Point p, bool bCenter = false)
      : SimpleVisualEntity(dPriority_, seq, bCenter, nTimeMeasure_), pos(p),
        bOnce(true) {}

  Point GetPosition() override { return pos; }
  void Update() override;

private:
  Point pos;
  bool bOnce;
};

/** A sight that shows one image at a fixed point. */
struct StaticImage : public Entity {
  std::string get_class_name() override { return "StaticImage"; }
  bool ShouldDraw() override { return true; }

  StaticImage(Index img_, Point p_ = Point(0, 0), bool bCentered_ = false,
              float dPriority_ = 0)
      : img(img_), dPriority(dPriority_), p(p_), bCentered(bCentered_) {}
  StaticImage(const StaticImage &) = default;
  StaticImage &operator=(const StaticImage &) = default;

  void Draw(ScalingDrawer *pDr) override;

  Point GetPosition() override { return p; }

  float GetPriority() override { return dPriority; }

private:
  Index img;
  float dPriority;
  Point p;
  bool bCentered;
};

/** A sight that draws a filled rectangle (covers the vista, no single spot). */
struct StaticRectangle : public Entity {
  std::string get_class_name() override { return "StaticRectangle"; }
  bool ShouldDraw() override { return true; }

  StaticRectangle(Rectangle r_, Color c_, float dPriority_ = 0)
      : dPriority(dPriority_), r(r_), c(c_) {}

  void Draw(ScalingDrawer *pDr) override;

  Point GetPosition() override { return Point(0, 0); }

  float GetPriority() override { return dPriority; }

private:
  float dPriority;
  Rectangle r;
  Color c;
};

/** A thing that can be struck (OnHit), has a kind (GetType) and an image
 * (GetImage). */
struct ConsumableEntity : virtual public Entity {
  std::string get_class_name() override { return "ConsumableEntity"; }
  virtual char GetType() = 0;
  virtual void OnHit(char cWhat) = 0;
  virtual Index GetImage() = 0;
};

/** A creature that moves: place, speed, bounds, and radius; Move() steps it
 * and clamps or removes it when it leaves the realm. */
struct Critter : public SimpleVisualEntity {
  std::string get_class_name() override { return "Critter"; }

  int GetRadius() override { return nRadius; }
  Point GetPosition() override { return fPos.ToPnt(); }
  void Move() override;

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
struct FancyCritter : public SimpleVisualEntity {
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
struct BonusScore : public Entity {
  std::string get_class_name() override { return "BonusScore"; }
  bool ShouldDraw() override { return true; }

  BonusScore(LevelController *pAc_, Point p_, int nScore_);

  void Update() override;

  void Draw(ScalingDrawer *pDr) override;

  float GetPriority() override { return 5; }

  Point GetPosition() override { return p; }

private:
  LevelController *pAc;
  std::string sText;
  int nScore;
  int nScoreSoFar;
  Point p;
  Timer t;
  int nC;
  Color c;
};

struct SoundControls : public Entity {
  std::string get_class_name() override { return "SoundControls"; }

  SoundControls(BackgroundMusicPlayer &plr_, int nTheme_)
      : plr(plr_), nTheme(nTheme_) {}
  SoundControls(const SoundControls &) = default;
  SoundControls &operator=(const SoundControls &) = delete;

  void SetTheme(int n) { nTheme = n; }

  void Update() override;

private:
  BackgroundMusicPlayer &plr;
  int nTheme;
};

/** Paints the finest tally yet within a rectangle on the vista. */
struct HighScoreShower : public Entity {
  std::string get_class_name() override { return "HighScoreShower"; }
  bool ShouldDraw() override { return true; }

  HighScoreShower(DragonGameController *pGl_, Rectangle rBound_)
      : pGl(pGl_), rBound(rBound_) {}

  void Draw(ScalingDrawer *pDr) override;

private:
  DragonGameController *pGl;
  Rectangle rBound;
};

struct IntroTextShower : public Entity {
  std::string get_class_name() override { return "IntroTextShower"; }
  bool ShouldDraw() override { return true; }

  IntroTextShower(DragonGameController *pGl_, Rectangle rBound_)
      : pGl(pGl_), rBound(rBound_) {}

  void Draw(ScalingDrawer *pDr) override;

private:
  DragonGameController *pGl;
  Rectangle rBound;
};

/** Clear from the list all that have left the realm (bExist is false). */
template <class T> void CleanUp(std::list<T> &ar) {
  for (typename std::list<T>::iterator itr = ar.begin(), etr = ar.end();
       itr != etr;) {
    if (!(*itr)->IsAlive())
      ar.erase(itr++);
    else
      ++itr;
  }
}

#endif
