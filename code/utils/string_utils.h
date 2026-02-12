/*
 * string_utils.h - Gui namespace: S(T) convert value to string.
 */

#ifndef GUI_STRING_UTILS_HEADER_ALREADY_DEFINED
#define GUI_STRING_UTILS_HEADER_ALREADY_DEFINED

#include <sstream>
#include <string>

namespace Gui {

/* Convert a value to a string using stream output. */
template <class T> std::string S(T t) {
  std::string s;
  std::ostringstream ostr(s);
  ostr << t;
  return ostr.str();
}

} // namespace Gui

#endif // GUI_STRING_UTILS_HEADER_ALREADY_DEFINED
