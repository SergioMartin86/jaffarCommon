#pragma once

/**
 * @file timing.hpp
 * @brief Contains common functions related to time measurement
 */

#include <chrono>
#include <stddef.h>
#include <type_traits>

namespace jaffarCommon
{

namespace timing
{

/**
 * Abstract definition of a time point
 */
typedef std::chrono::high_resolution_clock::time_point timePoint;

/**
 * Gets the current time point as per a high resolution clock
 *
 * @return The current high resolution time point
 */
__JAFFAR_COMMON_INLINE__ timePoint now() { return std::chrono::high_resolution_clock::now(); };

/**
 * Calculates the difference in seconds between two given time points (tf - t0), using a high resolution clock
 *
 * @param[in] end The end time point (tf)
 * @param[in] start The start time point (t0)
 * @return A 64-bit precision floating point number with the difference in seconds between the two time points
 */
__JAFFAR_COMMON_INLINE__ double timeDeltaSeconds(const std::chrono::time_point<std::chrono::high_resolution_clock> end,
                                                  const std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() * 1.0e-9;
};

/**
 * Calculates the difference in nanoseconds between two given time points (tf - t0), using a high resolution clock
 *
 * @param[in] end The end time point (tf)
 * @param[in] start The start time point (t0)
 * @return A 64-bit unsigned integer with the nanoseconds difference between the time points
 */
__JAFFAR_COMMON_INLINE__ size_t timeDeltaNanoseconds(const std::chrono::time_point<std::chrono::high_resolution_clock> end,
                                                      const std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
};

/**
 * Calculates the difference in microseconds between two given time points (tf - t0), using a high resolution clock
 *
 * @param[in] end The end time point (tf)
 * @param[in] start The start time point (t0)
 * @return A 64-bit unsigned integer with the microseconds difference between the time points
 */
__JAFFAR_COMMON_INLINE__ size_t timeDeltaMicroseconds(const std::chrono::time_point<std::chrono::high_resolution_clock> end,
                                                       const std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
};

} // namespace timing

} // namespace jaffarCommon
