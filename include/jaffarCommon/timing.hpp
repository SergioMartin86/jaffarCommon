#pragma once

/**
 * @file timing.hpp
 * @brief Contains common functions related to time measurement
 */

#include <chrono>
#include <type_traits>

namespace jaffarCommon
{

namespace timing
{

__INLINE__ auto now() { return std::chrono::high_resolution_clock::now(); };

__INLINE__ double timeDeltaSeconds(
  const std::chrono::time_point<std::chrono::high_resolution_clock> end,
  const std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() * 1.0e-9;
};

__INLINE__ size_t timeDeltaNanoseconds(
  const std::chrono::time_point<std::chrono::high_resolution_clock> end,
  const std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
};

} // namespace timing

} // namespace jaffarCommon
