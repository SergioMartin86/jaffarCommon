#pragma once

/**
 * @file timing.hpp
 * @brief Contains common functions related to time measurement
 */

#include <chrono>
#include <type_traits>

namespace jaffarCommon
{

inline std::chrono::time_point<std::chrono::high_resolution_clock> now() { return std::chrono::high_resolution_clock::now(); };


inline double timeDeltaSeconds(
   const std::chrono::time_point<std::chrono::high_resolution_clock> end,
   const std::chrono::time_point<std::chrono::high_resolution_clock> start) 
   { return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() * 1.0e-9; };

inline size_t timeDeltaNanoseconds(
   const std::chrono::time_point<std::chrono::high_resolution_clock> end,
   const std::chrono::time_point<std::chrono::high_resolution_clock> start) 
   { return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(); };

} // namespace jaffarCommon
