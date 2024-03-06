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

typedef _uint128_t hash_t;

__INLINE__ std::string getSHA1String(const std::string &string) { return sha1::SHA1::GetHash((const uint8_t *)string.data(), string.size()); }

__INLINE__ hash_t calculateMetroHash(const void *data, size_t size)
{
  MetroHash128 hash;
  hash.Update(data, size);
  hash_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}

__INLINE__ std::string hashToString(const hash_t hash)
{
  // Creating hash string
  char hashStringBuffer[256];
  sprintf(hashStringBuffer, "0x%016lX%016lX", hash.first, hash.second);
  return std::string(hashStringBuffer);
}

__INLINE__ hash_t hashString(const std::string &string) { return calculateMetroHash(string.data(), string.size()); }

} // namespace hash

} // namespace jaffarCommon