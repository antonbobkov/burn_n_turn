#ifndef TOWER_DEFENSE_MENU_CONTROLLER_H
#define TOWER_DEFENSE_MENU_CONTROLLER_H

/* In-game pause/main menu: menu entries, display, caret, options and
 * submenus; MenuController ties MenuDisplay and cursor to EntityListController.
 */

#include "controller/basic_controllers.h"
#include "entities.h"
#include "gui_key_type.h"
#include "mouse_utils.h"

struct MenuDisplay;
struct MenuController;

/** Pointer to a MenuDisplay member function used as menu item callback. */
typedef void (MenuDisplay::*EvntPntr)();

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

/** Returns "on" or "off" for menu toggles. */
std::string OnOffString(bool b);

/** Labels for options submenu entries. */
std::string SoundString();
std::string MusicString();
std::string TutorialString();
std::string FullTextString();

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

  /** Non-owning pointer; menu is owned by MenuController. */
  MenuController *pMenuController;

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

#endif
