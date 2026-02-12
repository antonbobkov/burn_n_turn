#include "debug_log.h"

#include <iostream>

namespace {

bool g_debug_enabled = false;

} // namespace

void EnableDebugLog() { g_debug_enabled = true; }

void DebugLog(std::string message) {
  if (g_debug_enabled)
    std::cout << message << "\n";
}
