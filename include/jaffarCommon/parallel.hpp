#pragma once

/**
 * @file parallel.hpp
 * @brief Definitions and utilities for parallel execution
 */

#include <stddef.h>
#include <stdint.h>
#include <omp.h>

namespace jaffarCommon
{

namespace parallel
{

/// Macro to initiate a parallel basic block. Uses OpenMP for this
#define JAFFAR_PARALLEL _Pragma("omp parallel")

/// Macro to initiate a parallel for basic block. Uses OpenMP for this
#define JAFFAR_PARALLEL_FOR _Pragma("omp parallel for")

/// Type definition for thread identifier
typedef uint32_t threadId_t;

/**
 * Gets the id of the currently running thread
 *
 * @return The id of the currently running thread
 */
__INLINE__ threadId_t getThreadId() { return (threadId_t)omp_get_thread_num(); }

/**
 * Gets the number of currently running threads
 *
 * @return The number of currently running threads
 */
__INLINE__ size_t getThreadCount() { return (threadId_t)omp_get_num_threads(); }

/**
 * Gets the number of maximum possible threads
 *
 * @return The number of maximum possible threads
 */
__INLINE__ size_t getMaxThreadCount() { return (threadId_t)omp_get_max_threads(); }

} // namespace parallel

} // namespace jaffarCommon
