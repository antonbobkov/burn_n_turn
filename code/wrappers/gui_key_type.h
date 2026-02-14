/*
 * gui_key_type.h - Gui namespace: GuiKeyType enum for keyboard key ids.
 */

#ifndef GUI_KEY_TYPE_HEADER_ALREADY_DEFINED
#define GUI_KEY_TYPE_HEADER_ALREADY_DEFINED

/* Keyboard key identifiers: ASCII-like codes plus platform-mapped keys
 * (F-keys, arrows, modifiers, etc.). */
enum GuiKeyType {
  /* Usually consistent across systems (ASCII). */
  GUI_BACKSPACE = 8,
  GUI_TAB = 9,
  GUI_RETURN = 13,
  GUI_ESCAPE = 27,

  /* Inconsistent keys - need to be mapped manually. */
  GUI_DUMMY = 300,

  GUI_F1,
  GUI_F2,
  GUI_F3,
  GUI_F4,
  GUI_F5,
  GUI_F6,
  GUI_F7,
  GUI_F8,
  GUI_F9,
  GUI_F10,
  GUI_F11,
  GUI_F12,

  GUI_UP,
  GUI_DOWN,
  GUI_LEFT,
  GUI_RIGHT,

  GUI_INSERT,
  GUI_HOME,
  GUI_END,
  GUI_PGUP,
  GUI_PGDOWN,

  GUI_NUMLOCK,
  GUI_CAPSLOCK,
  GUI_SCRLOCK,

  GUI_SHIFT,
  GUI_CTRL,

  GUI_DELETE
};

#endif // GUI_KEY_TYPE_HEADER_ALREADY_DEFINED
