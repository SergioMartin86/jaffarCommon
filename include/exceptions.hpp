#pragma once

/**
 * @file exceptions.hpp
 * @brief Contains common functions for exception throwing
 */

#include <cstdarg>
#include <stdexcept>
#include <cstdio>
#include <unistd.h>
#include "string.hpp"

namespace jaffarCommon
{

namespace exceptions
{

#define JAFFAR_THROW_RUNTIME(...) jaffarCommon::exceptions::throwRuntime(__FILE__, __LINE__, __VA_ARGS__)
__INLINE__ void throwRuntime [[noreturn]] (const char *fileName, const int lineNumber, const char *format, ...)
{
  char *outstr = 0;
  va_list ap;
  va_start(ap, format);
  int ret = vasprintf(&outstr, format, ap);
  if (ret < 0) exit(-1);

  std::string outString = outstr;
  free(outstr);

  char info[1024];

  snprintf(info, sizeof(info) - 1, " + From %s:%d\n", fileName, lineNumber);
  outString += info;

  throw std::runtime_error(outString.c_str());
}

#define JAFFAR_THROW_LOGIC(...) jaffarCommon::exceptions::throwLogic(__FILE__, __LINE__, __VA_ARGS__)
__INLINE__ void throwLogic [[noreturn]] (const char *fileName, const int lineNumber, const char *format, ...)
{
  char *outstr = 0;
  va_list ap;
  va_start(ap, format);
  int ret = vasprintf(&outstr, format, ap);
  if (ret < 0) exit(-1);

  std::string outString = outstr;
  free(outstr);

  char info[1024];

  snprintf(info, sizeof(info) - 1, " + From %s:%d\n", fileName, lineNumber);
  outString += info;

  throw std::logic_error(outString.c_str());
}

} // namespace exceptions

} // namespace jaffarCommon