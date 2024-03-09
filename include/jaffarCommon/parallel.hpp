#pragma once

/**
 * @file parallel.hpp
 * @brief Definitions and utilities for parallel execution
 */

#include <cstddef>
#include <omp.h>

namespace jaffarCommon
{

namespace parallel
{

#define JAFFAR_PARALLEL _Pragma("omp parallel")
#define JAFFAR_PARALLEL_FOR _Pragma("omp parallel for")

typedef uint32_t threadId_t;

__INLINE__ threadId_t getThreadId() { return (threadId_t)omp_get_thread_num(); }
__INLINE__ size_t getThreadCount() { return (threadId_t)omp_get_num_threads(); }
__INLINE__ size_t getMaxThreadCount() { return (threadId_t)omp_get_max_threads(); }

} // namespace parallel

} // namespace jaffarCommon
