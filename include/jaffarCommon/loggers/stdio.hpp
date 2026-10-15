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

/**
 * Prints the specified formatted string to the terminal
 *
 * @param[in] f The formatted string (checked against the arguments by the compiler)
 * @param[in] ... The arguments to the formatted string
 */
__attribute__((format(printf, 1, 2))) __JAFFAR_COMMON_INLINE__ void log(const char* f, ...)
{
  va_list ap;
  va_start(ap, f);
  vprintf(f, ap);
  va_end(ap);
}

__JAFFAR_COMMON_INLINE__ int  waitForKeyPress() { return getchar(); }
__JAFFAR_COMMON_INLINE__ int  getKeyPress() { return 0; };
__JAFFAR_COMMON_INLINE__ void initializeTerminal() {}
__JAFFAR_COMMON_INLINE__ void clearTerminal() {}
__JAFFAR_COMMON_INLINE__ void finalizeTerminal() {}
__JAFFAR_COMMON_INLINE__ void refreshTerminal() { fflush(stdout); }

} // namespace logger

} // namespace jaffarCommon
