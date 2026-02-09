#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include <string>

/* Call to turn on debug logging. Until then, DebugLog does nothing. */
void EnableDebugLog();

/* If EnableDebugLog() was called, write message to stdout. Otherwise no-op. */
void DebugLog(std::string message);

#endif
