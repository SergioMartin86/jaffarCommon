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

#define JAFFAR_THROW_RUNTIME(...) jaffarCommon::exceptions::throwException("Runtime", __FILE__, __LINE__, __VA_ARGS__)
#define JAFFAR_THROW_LOGIC(...) jaffarCommon::exceptions::throwException("Logic", __FILE__, __LINE__, __VA_ARGS__)

__INLINE__ void throwException [[noreturn]] (const char* exceptionType, const char *fileName, const int lineNumber, const char *format, ...)
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

  if (std::string(exceptionType) == "Logic") throw std::logic_error(outString.c_str());
  if (std::string(exceptionType) == "Runtime") throw std::runtime_error(outString.c_str());
  throw std::invalid_argument("Wrong exception type provided: " + std::string(exceptionType) + std::string(" Original error: ") + outString);
}

} // namespace exceptions

} // namespace jaffarCommon