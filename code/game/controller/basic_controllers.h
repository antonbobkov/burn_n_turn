#ifndef TOWER_DEFENSE_BASIC_CONTROLLERS_H
#define TOWER_DEFENSE_BASIC_CONTROLLERS_H

/* Include after GameController and TwrGlobalController are defined (e.g. from
 * game.h). */
#include "entities.h"
#include "smart_pointer.h"

/** Controller that draws a single full-screen image and advances on key. */
struct SimpleController : public GameController {
  Index nImage;

  SimpleController(smart_pointer<TwrGlobalController> pGraph,
                   std::string strFileName);
  ~SimpleController();

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ std::string GetControllerName() const { return "basic"; }
};

/** Controller that draws background + text that flashes every second. */
struct FlashingController : public GameController {
  Index nImage, nText;
  unsigned nTimer;
  bool bShow;

  FlashingController(smart_pointer<TwrGlobalController> pGraph,
                     std::string strFileName, std::string strTextName);
  ~FlashingController();

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ std::string GetControllerName() const { return "basic"; }
};

struct MenuDisplay;

typedef void (MenuDisplay::*EvntPntr)();

struct MenuController;

/** One menu item: size, label, callback (EvntPntr), disabled flag. */
struct MenuEntry : virtual public SP_Info {
  Size szSize;
  std::string sText;
  EvntPntr pTriggerEvent;
  bool bDisabled;

  MenuEntry(Size szSize_, std::string sText_, EvntPntr pTriggerEvent_,
            bool bDisabled_ = false)
      : szSize(szSize_), sText(sText_), pTriggerEvent(pTriggerEvent_),
        bDisabled(bDisabled_) {}
};

/** Return "on" or "off" for menu toggles. */
std::string OnOffString(bool b);

inline std::string SoundString() { return "sound: "; }
inline std::string MusicString() { return "music: "; }
inline std::string TutorialString() { return "tutorial: "; }
inline std::string FullTextString() { return "full screen: "; }

/** Holds menu entries and current selection index (nMenuPosition). */
struct MenuEntryManager {
  std::vector<MenuEntry> vEntries;
  int nMenuPosition;

  MenuEntryManager() : nMenuPosition(0) {}
};

/** In-game menu: draws entries, caret, handles mouse/key; submenus and option
 * toggles. */
struct MenuDisplay : virtual public EventEntity, public VisualEntity {
  MenuEntryManager *pCurr;

  MenuEntryManager memMain;
  MenuEntryManager memLoadChapter;
  MenuEntryManager memOptions;
  std::vector<std::string> vOptionText;

  int nMusic, nSound, nTutorial, nFullScreen, nCheats;
  bool bCheatsUnlocked;

  Point pLeftTop;
  SSP<NumberDrawer> pNum;

  SSP<Animation> pMenuCaret;

  SSP<MenuController> pMenuController;

  MenuDisplay(Point pLeftTop_, smart_pointer<NumberDrawer> pNum_,
              smart_pointer<Animation> pMenuCaret_,
              smart_pointer<MenuController> pMenuController_,
              bool bCheatsUnlocked_);

  /** Draws the menu entries and the caret at the current selection. */
  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return 0; }

  /*virtual*/ void Update() { pCurr->vEntries.at(0).bDisabled = false; }

  /** Updates which menu item is highlighted from the mouse position. */
  void OnMouseMove(Point pMouse);

  void PositionIncrement(bool bUp);

  void Boop();

  void Restart();
  void Continue();
  void MusicToggle();
  void SoundToggle();
  void TutorialToggle();
  void FullScreenToggle();
  void CheatsToggle();
  void Exit();

  void Escape();
  void LoadChapterSubmenu();
  void OptionsSubmenu();
  void UpdateMenuEntries();

  void Chapter1();
  void Chapter2();
  void Chapter3();
};

/** VisualEntity that draws a countdown number and sets bExist false when it
 * reaches 0. */
struct Countdown : public VisualEntity, public EventEntity {
  SSP<NumberDrawer> pNum;
  unsigned nTime, nCount;

  Countdown(smart_pointer<NumberDrawer> pNum_, unsigned nTime_)
      : pNum(this, pNum_), nTime(nTime_), nCount(0) {}

  /** Decrements the countdown every second; removes this when it reaches zero.
   */
  /*virtual*/ void Update();

  /** Draws the remaining countdown at a fixed screen position. */
  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ Point GetPosition() { return Point(0, 0); }
};

/** GameController with draw/update/consumable lists; Update runs Move, Update,
 * then draws by priority. */
struct EntityListController : public GameController {
  std::list<ASSP<VisualEntity>> lsDraw;
  std::list<ASSP<EventEntity>> lsUpdate;
  std::list<ASSP<ConsumableEntity>> lsPpl;

  /** Add visual entity to lsDraw. */
  void AddV(smart_pointer<VisualEntity> pVs);
  /** Adds an event entity to the update list. */
  void AddE(smart_pointer<EventEntity> pEv);

  template <class T> void AddBoth(T &t) {
    lsDraw.push_back(ASSP<VisualEntity>(this, t));
    lsUpdate.push_back(ASSP<EventEntity>(this, t));
  }

  /** Add scaled fullscreen StaticRectangle of color c to lsDraw. */
  void AddBackground(Color c);

  EntityListController(const EntityListController &b);
  bool bNoRefresh;
  EntityListController(smart_pointer<TwrGlobalController> pGl_, Rectangle rBound,
                  Color c);

  /**
   * Each frame: remove dead things from the lists, move everything that can
   * move, then update everyone. Draw everything in order from back to front.
   * Refresh the screen unless refresh is disabled.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "basic"; }
};

/** Cursor image and position; Draw/Update for rendering and click state. */
struct MouseCursor {
  bool bPressed;
  ImageSequence imgCursor;
  Point pCursorPos;
  TwrGlobalController *pGl;

  MouseCursor(ImageSequence imgCursor_, Point pCursorPos_,
              TwrGlobalController *pGl_)
      : imgCursor(imgCursor_), pCursorPos(pCursorPos_), pGl(pGl_),
        bPressed(false) {}

  void DrawCursor();
  void SetCursorPos(Point pPos);
};

/** Controller for pause/main menu: MenuDisplay, resume position. */
struct MenuController : public EntityListController {
  int nResumePosition;
  SSP<MenuDisplay> pMenuDisplay;

  MouseCursor mc;

  SSP<TextDrawEntity> pHintText;
  SSP<TextDrawEntity> pOptionText;

  MenuController(smart_pointer<TwrGlobalController> pGl_, Rectangle rBound,
                 Color c, int nResumePosition_);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void OnMouse(Point pPos);
  /*virtual*/ void OnMouseDown(Point pPos);

  /*virtual*/ void Update();
  /*virtual*/ std::string GetControllerName() const { return "menu"; }
};

struct StartScreenController : public EntityListController {
  StartScreenController(smart_pointer<TwrGlobalController> pGl_,
                        Rectangle rBound, Color c)
      : EntityListController(pGl_, rBound, c) {}

  /** Advance to next screen and play start_game sound. */
  void Next();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos) { Next(); }
  /*virtual*/ std::string GetControllerName() const { return "start"; }
};

struct BuyNowController;

struct SlimeUpdater : public VisualEntity {
  BuyNowController *pBuy;

  SlimeUpdater(BuyNowController *pBuy_) : pBuy(pBuy_) {}

  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);
  /*virtual*/ float GetPriority() { return 0; }
};

/** Controller for buy-now screen: slime animations and timer. */
struct BuyNowController : public EntityListController {
  int t;
  std::vector<smart_pointer<Animation>> mSlimes;
  std::vector<fPoint> mSlimeVel;
  std::vector<fPoint> mSlimePos;
  int nSlimeCount;

  Timer tVel;

  BuyNowController(smart_pointer<TwrGlobalController> pGl_, Rectangle rBound,
                   Color c);

  /** Set fVel to random direction, sometimes toward center; scale by speed. */
  void RandomizeVelocity(fPoint &fVel, fPoint pPos);

  /** Draw all slime animations. */
  void DrawSlimes();

  /**
   * Runs the normal controller update. Every so often, some slimes get a new
   * random direction. All slimes move and animate. Countdown runs. Screen
   * refreshes.
   */
  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "buy"; }
};

struct Cutscene : public EntityListController {
  SSP<FancyCritter> pCrRun;
  SSP<FancyCritter> pCrFollow;

  Timer tm;
  bool Beepy;

  bool bRelease;

  /**
   * Sets up the chase: black screen, one character running and one waiting.
   * Runner starts left or right depending on flip; when the runner reaches the
   * middle, the chaser is released. Beep/boop timer for sound.
   */
  Cutscene(smart_pointer<TwrGlobalController> pGl_, Rectangle rBound_,
           std::string sRun, std::string sChase, bool bFlip = false);

  /*virtual*/ void Update();
  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void OnMouseDown(Point pPos) {}
  /*virtual*/ std::string GetControllerName() const { return "cutscene"; }
};

/** Controller that shows dragon score and exits on click or timer. */
struct DragonScoreController : public EntityListController {
  Timer t;
  bool bClickToExit;

  DragonScoreController(smart_pointer<TwrGlobalController> pGl_,
                        Rectangle rBound, Color c, bool bScoreShow);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void Update();
  /*virtual*/ void DoubleClick();

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "score"; }
};

/** EntityListController that advances (Next) when only background is left or
 * on input. */
struct AutoAdvanceController : public EntityListController {
  AutoAdvanceController(const EntityListController &b)
      : EntityListController(b) {}

  /*virtual*/ void Update();

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);

  /*virtual*/ void OnMouseDown(Point pPos);
  /*virtual*/ std::string GetControllerName() const { return "logo"; }
};

#endif
