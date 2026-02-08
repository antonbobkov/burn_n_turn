#ifndef GENERAL_HEADER_ALREADY_DEFINED_08_25
#define GENERAL_HEADER_ALREADY_DEFINED_08_25

/*
 * General.h - Gui namespace utilities and helpers.
 * Provides: math min/max/abs (Gmin, Gmax, Gabs); string conversion S(T);
 * GuiKeyType enum.
 */

#include <list>
#include <map>
#include <set>
#include <vector>

#include <sstream>
#include <string>

namespace Gui {

/* Return the smaller of two values. */
template <class T> inline T Gmin(T a, T b) { return a < b ? a : b; }

/* Return the smallest of three values. */
template <class T> inline T Gmin(T a, T b, T c) { return Gmin(a, Gmin(b, c)); }

/* Return the larger of two values. */
template <class T> inline T Gmax(T a, T b) { return a > b ? a : b; }

/* Return the largest of three values. */
template <class T> inline T Gmax(T a, T b, T c) { return Gmax(a, Gmax(b, c)); }

/* Return the absolute value of a. */
template <class T> inline T Gabs(T a) { return a >= 0 ? a : -a; }

/* Convert a value to a string using stream output. */
template <class T> std::string S(T t) {
  std::string s;
  std::ostringstream ostr(s);
  ostr << t;
  return ostr.str();
}

/* Keyboard key identifiers: ASCII-like codes plus platform-mapped keys
 * (F-keys, arrows, modifiers, etc.). */
enum GuiKeyType {
  /* Usually consistent across systems (ASCII). */

  GUI_BACKSPACE = 8,
  GUI_TAB = 9,
  GUI_RETURN = 13,
  GUI_ESCAPE = 27,

  // inconsistent keys - need to be mapped manually

  GUI_DUMMY = 300, // to start enumeration (hope it works)

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

} // namespace Gui

#endif // GENERAL_HEADER_ALREADY_DEFINED_08_25