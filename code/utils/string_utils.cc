/*
 * string_utils.cc - BreakUpString implementation.
 */

#include "string_utils.h"

std::vector<std::string> BreakUpString(std::string s) {
  s += '\n';

  std::vector<std::string> vRet;
  std::string sCurr;

  for (int i = 0; i < (int)s.size(); ++i) {
    if (s[i] == '\n') {
      vRet.push_back(sCurr);
      sCurr = "";
    } else
      sCurr += s[i];
  }

  return vRet;
}
