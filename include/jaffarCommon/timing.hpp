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

/**
 * Gets the current time point as per a high resolution clock
 *
 * @return The current high resolution time point
 */
__INLINE__ auto now() { return std::chrono::high_resolution_clock::now(); };

/**
 * Calculates the difference in seconds between two given time points (tf - t0), using a high resolution clock
 *
 * @param[in] end The end time point (tf)
 * @param[in] start The start time point (t0)
 * @return A 64-bit precision floating point number with the difference in seconds between the two time points
 */
__INLINE__ double timeDeltaSeconds(const std::chrono::time_point<std::chrono::high_resolution_clock> end, const std::chrono::time_point<std::chrono::high_resolution_clock> start)
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
__INLINE__ size_t timeDeltaNanoseconds(const std::chrono::time_point<std::chrono::high_resolution_clock> end,
                                       const std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
};

} // namespace timing

} // namespace jaffarCommon
