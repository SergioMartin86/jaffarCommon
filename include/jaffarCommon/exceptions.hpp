#pragma once

/**
 * @file exceptions.hpp
 * @brief Contains common functions for exception throwing
 */

#include "string.hpp"
#include <stdarg.h>
#include <stdexcept>
#include <stdio.h>

namespace jaffarCommon
{

namespace exceptions
{

/**
 * Macro for throwing runtime exceptions. It includes the file, line and a C-formatted string
 */
#define JAFFAR_THROW_RUNTIME(...) jaffarCommon::exceptions::throwException("Runtime", __FILE__, __LINE__, __VA_ARGS__)

/**
 * Macro for throwing logic exceptions. It includes the file, line and a C-formatted string
 */
#define JAFFAR_THROW_LOGIC(...) jaffarCommon::exceptions::throwException("Logic", __FILE__, __LINE__, __VA_ARGS__)

/**
 * Common function for throwing exceptions.
 *
 * @param[in] exceptionType The type of exception to launch (e.g., "Logic", "Runtime")
 * @param[in] fileName The file in which the exception was originall thrown
 * @param[in] lineNumber The line inside the file where the exception was originall thrown
 * @param[in] format A constant formatted string
 * @param[in] ... parameters for the formatted string
 */
__JAFFAR_COMMON__INLINE__ void throwException [[noreturn]] (const char* exceptionType, const char* fileName, const int lineNumber, const char* format, ...)
{
  char*   outstr = 0;
  va_list ap;
  va_start(ap, format);
  int ret = vsnprintf(nullptr, 0, format, ap);
  va_end(ap);
  if (ret < 0) throw std::invalid_argument("Failed processing exception reason");

  outstr = (char*)malloc(ret + 1);
  if (outstr == nullptr) throw std::invalid_argument("Failed processing exception reason");

  va_start(ap, format);
  ret = vsnprintf(outstr, ret + 1, format, ap);
  va_end(ap);
  if (ret < 0)
  {
    free(outstr);
    throw std::invalid_argument("Failed processing exception reason");
  }

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