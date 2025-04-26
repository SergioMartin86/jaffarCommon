#pragma once

/**
 * @file concurrent.hpp
 * @brief Containers designed for fast parallel, mutual exclusive access
 */

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
using HashSet_t = phmap::parallel_flat_hash_set<V, phmap::priv::hash_default_hash<V>, phmap::priv::hash_default_eq<V>, std::allocator<V>, 4, std::mutex>;

/**
 * Definition for a parallel hash map. It enables concurrent inserts and queries
 */
template <class K, class V>
using HashMap_t = phmap::parallel_flat_hash_map<K, V, phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, std::allocator<std::pair<const K, V>>, 4, std::mutex>;

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
  __INLINE__ auto& getInternalStorage() { return _internalDeque; }

  /**
   * Pushes an element to the back of the deque without any locking protection
   *
   * @note This is not a thread safe operation
   *
   * @param[in] element The input element to push
   */
  __INLINE__ void push_back_no_lock(T element) { _internalDeque.push_back(element); }

  /**
   * Pushes an element to the back of the deque with locking protection
   *
   * @note This is a thread safe operation
   *
   * @param[in] element The input element to push
   */
  __INLINE__ void push_back(T element)
  {
    _mutex.lock();
    _internalDeque.push_back(element);
    _mutex.unlock();
  }

  /**
   * Pushes an element to the front of the deque without any locking protection
   *
   * @note This is not a thread safe operation
   *
   * @param[in] element The input element to push
   */
  __INLINE__ void push_front_no_lock(T element) { _internalDeque.push_front(element); }

  /**
   * Pushes an element to the front of the deque with locking protection
   *
   * @note This is a thread safe operation
   *
   * @param[in] element The input element to push
   */
  __INLINE__ void push_front(T element)
  {
    _mutex.lock();
    _internalDeque.push_front(element);
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
  __INLINE__ T front() const { return _internalDeque.front(); }

  /**
   * Gets the element at the back of the Deque
   *
   * @note This is not a thread safe operation
   * @note This operation does not check for an empty container and might produce unexpected behaviour if ran with an empty container
   *
   * @return The element at the back of the Deque
   */
  __INLINE__ T back() const { return _internalDeque.back(); }

  /**
   * Pops (removes) the element at the front of the Deque
   *
   * @note This is a thread safe operation
   * @note This operation does not check for an empty container and might produce unexpected behaviour if ran with an empty container
   */
  __INLINE__ void pop_front()
  {
    _mutex.lock();
    _internalDeque.pop_front();
    _mutex.unlock();
  }

  /**
   * Pops (removes) the element at the back of the Deque
   *
   * @note This is a thread safe operation
   * @note This operation does not check for an empty container and might produce unexpected behaviour if ran with an empty container
   */
  __INLINE__ void pop_back()
  {
    _mutex.lock();
    _internalDeque.pop_back();
    _mutex.unlock();
  }

  /**
   * Pops (removes) the element at the back of the Deque and retrieves it
   *
   * @note This is a thread safe operation
   * @param[out] element A reference to the storage to save the element into
   * @return True, if the operation was successful; false, if the Deque was empty
   */
  __INLINE__ bool pop_back_get(T& element)
  {
    _mutex.lock();

    if (_internalDeque.empty())
    {
      _mutex.unlock();
      return false;
    }

    element = _internalDeque.back();
    _internalDeque.pop_back();

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
  __INLINE__ bool pop_front_get(T& element)
  {
    _mutex.lock();

    if (_internalDeque.empty())
    {
      _mutex.unlock();
      return false;
    }

    element = _internalDeque.front();
    _internalDeque.pop_front();

    _mutex.unlock();
    return true;
  }

  /**
   * Retrieves the size of the container at the time of checking
   *
   * @note This is not a thread safe operation
   * @return The current size of the Deque at the time of checking
   */
  __INLINE__ size_t wasSize() const { return _internalDeque.size(); }

private:
  /**
   * Internal mutual exclusion mechanism
   */
  std::mutex _mutex;

  /**
   * Internal storage for the Deque
   */
  std::deque<T> _internalDeque;
};

} // namespace concurrent

} // namespace jaffarCommon
