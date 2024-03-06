#pragma once

/**
 * @file loggers/stdio.hpp
 * @brief Contains common functions related to output and logging using stdio
 */

#include "../string.hpp"
#include <cstdarg>
#include <cstdio>
#include <stdexcept>
#include <unistd.h>

namespace jaffarCommon
{

namespace logger
{

template <typename... Args>
__INLINE__ void log(const char *f, Args... args)
{
  auto string = jaffarCommon::string::formatString(f, args...);
  printf("%s", string.c_str());
}

__INLINE__ int waitForKeyPress() { return getchar(); }
__INLINE__ int getKeyPress() { return 0; };
__INLINE__ void initializeTerminal() {}
__INLINE__ void clearTerminal() {}
__INLINE__ void finalizeTerminal() {}
__INLINE__ void refreshTerminal() {}

} // namespace logger

} // namespace jaffarCommon