#pragma once

#include <string>
#include <vector>

/**
 * @file string.hpp
 * @brief Contains common functions related to manipulating strings
 */

namespace jaffarPlus
{

// Function to split a vector into n mostly fair chunks
template <typename T>
std::vector<T> splitVector(const T size, const T n)
{
  std::vector<T> subSizes(n);

  T length = size / n;
  T remain = size % n;

  for (T i = 0; i < n; i++)
    subSizes[i] = i < remain ? length + 1 : length;

  return subSizes;
}

// Function to split a string into a sub-strings delimited by a character
// Taken from stack overflow answer to https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
// By Evan Teran

template <typename Out>
inline void split(const std::string &s, char delim, Out result)
{
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim))
  {
    *result++ = item;
  }
}

static inline std::vector<std::string> split(const std::string &s, char delim)
{
 std::string newString = s;
 std::replace(newString.begin(), newString.end(), '\n', ' ');
 std::vector<std::string> elems;
 split(newString, delim, std::back_inserter(elems));
 return elems;
}

} // namespace jaffarPlus
