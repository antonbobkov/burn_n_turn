#ifndef TOWER_DEFENSE_MENU_CONTROLLER_H
#define TOWER_DEFENSE_MENU_CONTROLLER_H

/* The great pause hall: choices on the wall, the caret that points, options
 * and side chambers; the menu keeper ties display and cursor to the list.
 */

#include "wrappers/color.h"
#include "game/controller/basic_controllers.h"
#include "game_utils/draw_utils.h"
#include "game/entities.h"
#include "wrappers/gui_key_type.h"
#include "game_utils/mouse_utils.h"

struct DragonGameSettings;
struct MenuDisplay;
struct MenuController;

/** A spell the menu keeper can cast when a choice is chosen. */
typedef void (MenuDisplay::*EvntPntr)();

/** One choice on the wall: size, label, the spell to cast, and whether it is
 * dimmed. */
struct MenuEntry : virtual public SP_Info {
  std::string get_class_name() override { return "MenuEntry"; }
  Size szSize;
  std::string sText;
  EvntPntr pTriggerEvent;
  bool bDisabled;

  MenuEntry(Size szSize_, std::string sText_, EvntPntr pTriggerEvent_,
            bool bDisabled_ = false)
      : szSize(szSize_), sText(sText_), pTriggerEvent(pTriggerEvent_),
        bDisabled(bDisabled_) {}
};

/** Returns "on" or "off" for the realm's toggles. */
std::string OnOffString(bool b);

/** Labels for the options chamber. */
std::string SoundString();
std::string MusicString();
std::string TutorialString();
std::string FullTextString();

/** The list of choices on the wall and which one the caret points to. */
struct MenuEntryManager {
  std::vector<MenuEntry> vEntries;
  int nMenuPosition;

  MenuEntryManager() : nMenuPosition(0) {}
};

/** The pause hall's face: draws choices and caret, answers mouse and key;
 * side chambers and option toggles. */
struct MenuDisplay : virtual public EventEntity, public VisualEntity {
  std::string get_class_name() override { return "MenuDisplay"; }
  MenuEntryManager *pCurr;

  MenuEntryManager memMain;
  MenuEntryManager memLoadChapter;
  MenuEntryManager memOptions;
  std::vector<std::string> vOptionText;

  int nMusic, nSound, nTutorial, nFullScreen, nCheats;
  bool bCheatsUnlocked;

  Point pLeftTop;
  smart_pointer<NumberDrawer> pNum;

  smart_pointer<Animation> pMenuCaret;

  /** The menu keeper who owns this hall (we only point). */
  MenuController *pMenuController;

  MenuDisplay(Point pLeftTop_, smart_pointer<NumberDrawer> pNum_,
              smart_pointer<Animation> pMenuCaret_,
              smart_pointer<MenuController> pMenuController_,
              bool bCheatsUnlocked_);

  /** Draw the choices on the wall and the caret at the chosen line. */
  /*virtual*/ void Draw(smart_pointer<ScalingDrawer> pDr);

  /*virtual*/ float GetPriority() { return 0; }

  /*virtual*/ void Update() { pCurr->vEntries.at(0).bDisabled = false; }

  /** Move the caret to the choice under the mouse. */
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

/** The keeper of the pause hall: the display and options from the scrolls. */
struct MenuController : public EntityListController {
  std::string get_class_name() override { return "MenuController"; }
  smart_pointer<MenuDisplay> pMenuDisplay;

  MouseCursor mc;

  smart_pointer<TextDrawEntity> pHintText;
  smart_pointer<TextDrawEntity> pOptionText;

  DragonGameSettings *settings;

  MenuController(DragonGameController *pGl_, DragonGameSettings *settings_,
                 Rectangle rBound, Color c);

  /*virtual*/ void OnKey(GuiKeyType c, bool bUp);
  /*virtual*/ void OnMouse(Point pPos);
  /*virtual*/ void OnMouseDown(Point pPos);

  /*virtual*/ void Update();
  /*virtual*/ std::string GetControllerName() const { return "menu"; }
};

#endif
