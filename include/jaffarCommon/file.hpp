#pragma once

/**
 * @file file.hpp
 * @brief Contains common functions related to file manipulation
 */

#include <fstream>
#include <sstream>
#include <string>

namespace jaffarCommon
{

namespace file
{

// Taken from https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c/116220#116220
static __INLINE__ std::string slurp(std::ifstream &in)
{
  std::ostringstream sstr;
  sstr << in.rdbuf();
  return sstr.str();
}

static __INLINE__ bool loadStringFromFile(std::string &dst, const std::string &fileName)
{
  std::ifstream fi(fileName);

  // If file not found or open, return false
  if (fi.good() == false) return false;

  // Reading entire file
  dst = slurp(fi);

  // Closing file
  fi.close();

  return true;
}

// Save string to a file
static __INLINE__ bool saveStringToFile(const std::string &src, const std::string &fileName)
{
  FILE *fid = fopen(fileName.c_str(), "w");
  if (fid != NULL)
  {
    fwrite(src.c_str(), 1, src.size(), fid);
    fclose(fid);
    return true;
  }
  return false;
}

} // namespace file

} // namespace jaffarCommon
