#pragma once

/**
 * @file hash.hpp
 * @brief Contains common function related to hashing
 */

#include <cstdio>
#include <metrohash128/metrohash128.h>
#include <sha1/sha1.hpp>
#include <string>

namespace jaffarCommon
{

namespace hash
{

/**
 * Definition for the standard 128-bit hash value in Jaffar.
*/
typedef _uint128_t hash_t;

/**
 * Calculates the SHA1 sum of a given input string and returns it as a stylized (string)
 * 
 * @param[in] string The input string
 * @return The SHA1 string of the input string
*/
__INLINE__ std::string getSHA1String(const std::string &string) { return sha1::SHA1::GetHash((const uint8_t *)string.data(), string.size()); }

/**
 * Calculates the 128 bit Metrohash of a given buffer
 * 
 * @param[in] data The input buffer to hash
 * @param[in] size The size of the buffer to hash
 * @return The calculated 128-bit metro hash
*/
__INLINE__ hash_t calculateMetroHash(const void *data, size_t size)
{
  MetroHash128 hash;
  hash.Update(data, size);
  hash_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}

/**
 * Produces an output string given a 128-bit hash
 * 
 * @param[in] hash The hash value to stringify
 * @return The string containing the stringified hash, formatted as upper case hexadecimal
*/
__INLINE__ std::string hashToString(const hash_t hash)
{
  // Creating hash string
  char hashStringBuffer[256];
  sprintf(hashStringBuffer, "0x%016lX%016lX", hash.first, hash.second);
  return std::string(hashStringBuffer);
}

/**
 * Calculates the 128-bit metro hash of a given string
 * 
 * @param[in] string The string to calculate the hash for
 * @return The hash value of the provided string
*/
__INLINE__ hash_t hashString(const std::string &string) { return calculateMetroHash(string.data(), string.size()); }

} // namespace hash

} // namespace jaffarCommon