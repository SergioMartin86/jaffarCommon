#pragma once

/**
 * @file string.hpp
 * @brief Contains common functions related to manipulating strings
 */

#include <cstdarg>
#include <sstream>
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
__INLINE__ void split(const std::string &s, char delim, Out result)
{
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim))
  {
    *result++ = item;
  }
}

/**
 * Function to split a string into a vector of sub-strings delimited by a character
 *
 * @param[in] s The input string
 * @param[in] delim The separator that divides the substrings
 * @return A vector containing all the substrings
 */
__INLINE__ std::vector<std::string> split(const std::string &s, char delim)
{
  std::string newString = s;
  std::replace(newString.begin(), newString.end(), '\n', ' ');
  std::vector<std::string> elems;
  split(newString, delim, std::back_inserter(elems));
  return elems;
}

/**
 * Converts a C-formatted string into a C++ string
 *
 * @param[in] format The format string
 * @param[in] ... The arguments to the format string
 * @return The C++ string produced
 */
__INLINE__ std::string formatString(const char *format, ...)
{
  char *outstr = 0;
  va_list ap;
  va_start(ap, format);
  int ret = vasprintf(&outstr, format, ap);
  if (ret == 0) return "";

  std::string outString = outstr;
  free(outstr);
  return outString;
}

} // namespace string

} // namespace jaffarCommon
