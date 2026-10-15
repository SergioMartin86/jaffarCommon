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
 * The clock all time points come from: a monotonic one, so intervals are never negative (in libstdc++
 * the high resolution clock is the system clock, which jumps when the time of day is adjusted)
 */
typedef std::chrono::steady_clock clock;

/**
 * Abstract definition of a time point
 */
typedef clock::time_point timePoint;

/**
 * Gets the current time point as per a monotonic clock
 *
 * @return The current time point
 */
__JAFFAR_COMMON_INLINE__ timePoint now() { return clock::now(); };

/**
 * Calculates the difference in seconds between two given time points (tf - t0), using the monotonic clock
 *
 * @param[in] end The end time point (tf)
 * @param[in] start The start time point (t0)
 * @return A 64-bit precision floating point number with the difference in seconds between the two time points
 */
__JAFFAR_COMMON_INLINE__ double timeDeltaSeconds(const timePoint end, const timePoint start)
{
  return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() * 1.0e-9;
};

/**
 * Calculates the difference in nanoseconds between two given time points (tf - t0), using the monotonic clock
 *
 * @param[in] end The end time point (tf)
 * @param[in] start The start time point (t0)
 * @return A 64-bit unsigned integer with the nanoseconds difference between the time points (0 if end precedes start)
 */
__JAFFAR_COMMON_INLINE__ size_t timeDeltaNanoseconds(const timePoint end, const timePoint start)
{
  const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  return delta > 0 ? (size_t)delta : 0;
};

/**
 * Calculates the difference in microseconds between two given time points (tf - t0), using the monotonic clock
 *
 * @param[in] end The end time point (tf)
 * @param[in] start The start time point (t0)
 * @return A 64-bit unsigned integer with the microseconds difference between the time points (0 if end precedes start)
 */
__JAFFAR_COMMON_INLINE__ size_t timeDeltaMicroseconds(const timePoint end, const timePoint start)
{
  const auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
  return delta > 0 ? (size_t)delta : 0;
};

} // namespace timing

} // namespace jaffarCommon
