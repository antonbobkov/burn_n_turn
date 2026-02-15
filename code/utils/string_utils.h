/*
 * string_utils.h - String helpers: S(T), BreakUpString.
 */

#ifndef GUI_STRING_UTILS_HEADER_ALREADY_DEFINED
#define GUI_STRING_UTILS_HEADER_ALREADY_DEFINED

#include <sstream>
#include <string>
#include <vector>

/* Convert a value to a string using stream output. */
template <class T> std::string S(T t) {
  std::string s;
  std::ostringstream ostr(s);
  ostr << t;
  return ostr.str();
}

/* Split string into lines (by newline). */
std::vector<std::string> BreakUpString(std::string s);

#endif // GUI_STRING_UTILS_HEADER_ALREADY_DEFINED
