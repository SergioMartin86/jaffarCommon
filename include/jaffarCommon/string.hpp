#pragma once

/**
 * @file string.hpp
 * @brief Contains common functions related to manipulating strings
 */

#include <algorithm>
#include <sstream>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

namespace jaffarCommon
{

namespace string
{

/**
 * [Internal] Function to split a string into a sub-strings collection delimited by a character
 *
 * @note Taken from stack overflow answer to https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string By Evan Teran
 *
 * @param[in] s The input string
 * @param[in] delim The separator that divides the substrings
 * @param[out] result The storage where the substring collection wil be stored
 */
template <typename Out>
__JAFFAR_COMMON_INLINE__ void split(const std::string& s, char delim, Out result)
{
  std::istringstream iss(s);
  std::string        item;
  while (std::getline(iss, item, delim)) { *result++ = item; }
}

/**
 * Function to split a string into a vector of sub-strings delimited by a character
 *
 * @param[in] s The input string
 * @param[in] delim The separator that divides the substrings
 * @return A vector containing all the substrings
 */
__JAFFAR_COMMON_INLINE__ std::vector<std::string> split(const std::string& s, char delim)
{
  std::string newString = s;
  std::replace(newString.begin(), newString.end(), '\n', delim);
  std::vector<std::string> elems;
  split(newString, delim, std::back_inserter(elems));
  return elems;
}

/**
 * Converts a C-formatted string into a C++ string, taking the arguments as a va_list
 *
 * @param[in] format The format string
 * @param[in] ap The arguments to the format string (not consumed: the caller still owns and ends it)
 * @return The C++ string produced
 */
__JAFFAR_COMMON_INLINE__ std::string vformatString(const char* format, va_list ap)
{
  va_list sizing;
  va_copy(sizing, ap);
  const int size = vsnprintf(nullptr, 0, format, sizing);
  va_end(sizing);
  if (size < 0) return "";

  std::string outString((size_t)size + 1, '\0');
  va_list     writing;
  va_copy(writing, ap);
  const int ret = vsnprintf(outString.data(), outString.size(), format, writing);
  va_end(writing);
  if (ret < 0) return "";

  outString.resize((size_t)size);
  return outString;
}

/**
 * Converts a C-formatted string into a C++ string
 *
 * @param[in] format The format string
 * @param[in] ... The arguments to the format string
 * @return The C++ string produced
 */
__attribute__((format(printf, 1, 2))) __JAFFAR_COMMON_INLINE__ std::string formatString(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  auto outString = vformatString(format, ap);
  va_end(ap);
  return outString;
}

/**
 * Converts a binary input into a readable hex string
 *
 * @param[in] data Binary data to print
 * @param[in] size Number of bytes in the input data
 * @return The C++ string produced
 */
__JAFFAR_COMMON_INLINE__ std::string dumpBinary(const void* data, const size_t size)
{
  auto        input = (uint8_t*)data;
  std::string output;
  for (size_t i = 0; i < size; i++)
  {
    char substr[16];
    snprintf(substr, sizeof(substr), "%02X", input[i]);
    output += std::string(substr);
  }

  return output;
}

} // namespace string

} // namespace jaffarCommon
