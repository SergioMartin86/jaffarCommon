#pragma once

/**
 * @file loggers/stdio.hpp
 * @brief Contains common functions related to output and logging using stdio
 */

#include "../string.hpp"
#include <stdarg.h>
#include <stdexcept>
#include <stdio.h>

namespace jaffarCommon
{

namespace logger
{

template <typename... Args>
__JAFFAR_COMMON__INLINE__ void log(const char* f, Args... args)
{
  auto string = jaffarCommon::string::formatString(f, args...);
  printf("%s", string.c_str());
}

__JAFFAR_COMMON__INLINE__ int  waitForKeyPress() { return getchar(); }
__JAFFAR_COMMON__INLINE__ int  getKeyPress() { return 0; };
__JAFFAR_COMMON__INLINE__ void initializeTerminal() {}
__JAFFAR_COMMON__INLINE__ void clearTerminal() {}
__JAFFAR_COMMON__INLINE__ void finalizeTerminal() {}
__JAFFAR_COMMON__INLINE__ void refreshTerminal() { fflush(stdout); }

} // namespace logger

} // namespace jaffarCommon
