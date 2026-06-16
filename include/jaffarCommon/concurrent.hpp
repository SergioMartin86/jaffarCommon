#pragma once

/**
 * @file concurrent.hpp
 * @brief Containers designed for fast parallel, mutual exclusive access
 */

#include <atomic>
#include <atomic_queue/include/atomic_queue/atomic_queue.h>
#include <deque>
#include <mutex>
#include <oneapi/tbb/concurrent_map.h>
#include <phmap/parallel_hashmap/phmap.h>
#include <stddef.h>

namespace jaffarCommon
{

namespace concurrent
{

/**
 * Definition for an atomic queue. It enables lock-free concurrent push and pop operations.
 */
template <class T>
using atomicQueue_t = atomic_queue::AtomicQueueB<T>;

/**
 * Definition for a parallel hash set. It enables concurrent inserts and queries
 */
template <class V>
using HashSet_t = phmap::parallel_flat_hash_set<V, phmap::priv::hash_default_hash<V>, phmap::priv::hash_default_eq<V>, std::allocator<V>, 8, std::mutex>;

/**
 * Definition for a parallel hash map. It enables concurrent inserts and queries
 */
template <class K, class V>
using HashMap_t = phmap::parallel_flat_hash_map<K, V, phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, std::allocator<std::pair<const K, V>>, 8, std::mutex>;

/**
 * Definition for a concurrent multimap. It enables concurrent inserts and queries
 */
template <class K, class V, class C = std::greater<K>>
using concurrentMultimap_t = oneapi::tbb::concurrent_multimap<K, V, C>;

/**
 * This implementation of a concurrent doble-ended queue class was created specifically for Jaffar's engine
 * It allows for lock-free front and back push, pop, and pop_get operations
 * It uses a single mutex to coordinate access. This could theoretically be improved, but for the time being seems to suffice
 */
template <class T>
class Deque
{
public:
  Deque()  = default;
  ~Deque() = default;

  /**
   * Gets access to the internal Deque storage
   * @return A reference to the internal Deque storage
   */
  __JAFFAR_COMMON_INLINE__ auto& getInternalStorage() { return _internalDeque; }

  /**
   * Pushes an element to the back of the deque without any locking protection
   *
   * @note This is not a thread safe operation
   *
   * @param[in] element The input element to push
   */
  __JAFFAR_COMMON_INLINE__ void push_back_no_lock(T element) { _internalDeque.push_back(element); _size.fetch_add(1, std::memory_order_relaxed); }

  /**
   * Pushes an element to the back of the deque with locking protection
   *
   * @note This is a thread safe operation
   *
   * @param[in] element The input element to push
   */
  __JAFFAR_COMMON_INLINE__ void push_back(T element)
  {
    _mutex.lock();
    _internalDeque.push_back(element);
    _size.fetch_add(1, std::memory_order_relaxed);
    _mutex.unlock();
  }

  /**
   * Pushes an element to the front of the deque without any locking protection
   *
   * @note This is not a thread safe operation
   *
   * @param[in] element The input element to push
   */
  __JAFFAR_COMMON_INLINE__ void push_front_no_lock(T element) { _internalDeque.push_front(element); _size.fetch_add(1, std::memory_order_relaxed); }

  /**
   * Pushes an element to the front of the deque with locking protection
   *
   * @note This is a thread safe operation
   *
   * @param[in] element The input element to push
   */
  __JAFFAR_COMMON_INLINE__ void push_front(T element)
  {
    _mutex.lock();
    _internalDeque.push_front(element);
    _size.fetch_add(1, std::memory_order_relaxed);
    _mutex.unlock();
  }

  /**
   * Gets the element at the front of the Deque
   *
   * @note This is not a thread safe operation
   * @note This operation does not check for an empty container and might produce unexpected behaviour if ran with an empty container
   *
   * @return The element at the front of the Deque
   */
  __JAFFAR_COMMON_INLINE__ T front() const { return _internalDeque.front(); }

  /**
   * Gets the element at the back of the Deque
   *
   * @note This is not a thread safe operation
   * @note This operation does not check for an empty container and might produce unexpected behaviour if ran with an empty container
   *
   * @return The element at the back of the Deque
   */
  __JAFFAR_COMMON_INLINE__ T back() const { return _internalDeque.back(); }

  /**
   * Pops (removes) the element at the front of the Deque
   *
   * @note This is a thread safe operation
   * @note This operation does not check for an empty container and might produce unexpected behaviour if ran with an empty container
   */
  __JAFFAR_COMMON_INLINE__ void pop_front()
  {
    _mutex.lock();
    _internalDeque.pop_front();
    _size.fetch_sub(1, std::memory_order_relaxed);
    _mutex.unlock();
  }

  /**
   * Pops (removes) the element at the back of the Deque
   *
   * @note This is a thread safe operation
   * @note This operation does not check for an empty container and might produce unexpected behaviour if ran with an empty container
   */
  __JAFFAR_COMMON_INLINE__ void pop_back()
  {
    _mutex.lock();
    _internalDeque.pop_back();
    _size.fetch_sub(1, std::memory_order_relaxed);
    _mutex.unlock();
  }

  /**
   * Pops (removes) the element at the back of the Deque and retrieves it
   *
   * @note This is a thread safe operation
   * @param[out] element A reference to the storage to save the element into
   * @return True, if the operation was successful; false, if the Deque was empty
   */
  __JAFFAR_COMMON_INLINE__ bool pop_back_get(T& element)
  {
    _mutex.lock();

    if (_internalDeque.empty())
    {
      _mutex.unlock();
      return false;
    }

    element = _internalDeque.back();
    _internalDeque.pop_back();
    _size.fetch_sub(1, std::memory_order_relaxed);

    _mutex.unlock();
    return true;
  }

  /**
   * Pops (removes) the element at the front of the Deque and retrieves it
   *
   * @note This is a thread safe operation
   * @param[out] element A reference to the storage to save the element into
   * @return True, if the operation was successful; false, if the Deque was empty
   */
  __JAFFAR_COMMON_INLINE__ bool pop_front_get(T& element)
  {
    _mutex.lock();

    if (_internalDeque.empty())
    {
      _mutex.unlock();
      return false;
    }

    element = _internalDeque.front();
    _internalDeque.pop_front();
    _size.fetch_sub(1, std::memory_order_relaxed);

    _mutex.unlock();
    return true;
  }

  /**
   * Retrieves the size of the container at the time of checking
   *
   * @note Reads an atomic counter rather than std::deque::size(), so it is safe to call
   *       concurrently with pushes/pops (the size may be momentarily stale, but it will not
   *       crash by walking deque internals that another thread is mutating).
   * @return The current size of the Deque at the time of checking
   */
  __JAFFAR_COMMON_INLINE__ size_t wasSize() const { return _size.load(std::memory_order_relaxed); }

private:
  /**
   * Internal mutual exclusion mechanism
   */
  std::mutex _mutex;

  /**
   * Element count, maintained atomically alongside every push/pop so that wasSize() can be read
   * concurrently without touching the (non-thread-safe) std::deque internals.
   */
  std::atomic<size_t> _size{0};

  /**
   * Internal storage for the Deque
   */
  std::deque<T> _internalDeque;
};

} // namespace concurrent

} // namespace jaffarCommon
