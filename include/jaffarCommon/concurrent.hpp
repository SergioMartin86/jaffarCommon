#pragma once

/**
 * @file concurrent.hpp
 * @brief Containers designed for fast parallel, mutual exclusive access
 */

#include <atomic>
#include <atomic_queue/include/atomic_queue/atomic_queue.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
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
 * A fixed-capacity buffer specialized for a fill-once / drain-from-both-ends lifecycle, with a
 * lock-free concurrent drain phase.
 *
 * It is built for the pattern where one phase fills the buffer single-threaded (no contention),
 * a barrier follows, and then many threads concurrently consume the elements -- most pulling
 * batches from the front, a few pulling single elements from the back -- until it is empty. There
 * is never a push concurrent with a pop. (This is exactly how a best-first search step works: the
 * ordered set of states for the step is laid down once, then worker threads drain it.)
 *
 * Versus a mutex-guarded std::deque this wins three ways: the fill is a plain sequential store with
 * no per-element allocation; the storage is contiguous, so the concurrent drain is cache- and
 * prefetcher-friendly; and front/back claiming is lock-free (a single CAS) instead of taking a
 * mutex. Both ends are claimed by advancing counters packed into one 64-bit atomic, so a front and
 * a back claim can never alias the same slot. Indices only grow within a step and are reset
 * (clear()) only while the buffer is quiescent, so there is no ABA hazard.
 *
 * @note Capacity is fixed at reserve() time; the fill phase must not exceed it. Capacity is limited
 *       to UINT32_MAX elements (the claim counters are 32-bit halves).
 */
template <class T>
class DrainBuffer
{
public:
  DrainBuffer() = default;
  ~DrainBuffer()
  {
    if (_buffer != nullptr) free(_buffer);
  }

  /**
   * Allocates the backing storage. Must be called once before use.
   * @param[in] capacity Maximum number of elements the buffer will ever hold in a single fill phase
   */
  __JAFFAR_COMMON_INLINE__ void reserve(const size_t capacity)
  {
    _capacity = capacity;
    _buffer   = (T*)malloc(capacity * sizeof(T));
  }

  /**
   * Resets the buffer to empty for a new fill phase. Must be called while quiescent (no concurrent
   * drain in flight), e.g. right before the single-threaded fill of the next step.
   */
  __JAFFAR_COMMON_INLINE__ void clear()
  {
    _count = 0;
    _claim.store(0, std::memory_order_relaxed);
  }

  /**
   * Appends an element during the (single-threaded) fill phase.
   *
   * @note Not thread safe -- intended to be called by a single filler thread between clear() and
   *       the start of the concurrent drain.
   */
  __JAFFAR_COMMON_INLINE__ void push_back_no_lock(T element) { _buffer[_count++] = element; }

  /**
   * Claims up to maxCount elements from the front in a single lock-free step, copying them into the
   * provided buffer in front-to-back order.
   *
   * @param[out] elements Destination buffer; room for at least maxCount elements
   * @param[in] maxCount Maximum number of elements to claim
   * @return The number of elements actually claimed (0 if empty)
   */
  __JAFFAR_COMMON_INLINE__ size_t pop_front_get_batch(T* elements, const size_t maxCount)
  {
    uint64_t observed = _claim.load(std::memory_order_acquire);
    uint64_t desired;
    uint32_t front;
    size_t   take;
    do {
      front              = (uint32_t)(observed & 0xFFFFFFFFULL);
      const uint32_t back = (uint32_t)(observed >> 32);
      if ((size_t)front + (size_t)back >= _count) return 0; // empty
      const size_t available = _count - (size_t)front - (size_t)back;
      take                    = maxCount < available ? maxCount : available;
      desired                 = (observed & 0xFFFFFFFF00000000ULL) | (uint64_t)(front + (uint32_t)take);
    } while (_claim.compare_exchange_weak(observed, desired, std::memory_order_acq_rel, std::memory_order_acquire) == false);

    memcpy(elements, &_buffer[front], take * sizeof(T));
    return take;
  }

  /**
   * Claims a single element from the front. Lock-free.
   * @param[out] element Storage for the claimed element
   * @return True if an element was claimed; false if empty
   */
  __JAFFAR_COMMON_INLINE__ bool pop_front_get(T& element) { return pop_front_get_batch(&element, 1) == 1; }

  /**
   * Claims a single element from the back. Lock-free.
   * @param[out] element Storage for the claimed element
   * @return True if an element was claimed; false if empty
   */
  __JAFFAR_COMMON_INLINE__ bool pop_back_get(T& element)
  {
    uint64_t observed = _claim.load(std::memory_order_acquire);
    uint64_t desired;
    uint32_t back;
    do {
      const uint32_t front = (uint32_t)(observed & 0xFFFFFFFFULL);
      back                 = (uint32_t)(observed >> 32);
      if ((size_t)front + (size_t)back >= _count) return false; // empty
      desired = (observed & 0x00000000FFFFFFFFULL) | ((uint64_t)(back + 1) << 32);
    } while (_claim.compare_exchange_weak(observed, desired, std::memory_order_acq_rel, std::memory_order_acquire) == false);

    element = _buffer[_count - 1 - (size_t)back];
    return true;
  }

  /**
   * Number of elements not yet claimed, at the time of checking. Safe to call concurrently.
   */
  __JAFFAR_COMMON_INLINE__ size_t wasSize() const
  {
    const uint64_t observed = _claim.load(std::memory_order_relaxed);
    const size_t   claimed  = (size_t)(observed & 0xFFFFFFFFULL) + (size_t)(observed >> 32);
    return claimed >= _count ? 0 : _count - claimed;
  }

private:
  /**
   * Contiguous backing storage, allocated once by reserve()
   */
  T* _buffer = nullptr;

  /**
   * Allocated capacity (elements)
   */
  size_t _capacity = 0;

  /**
   * Number of elements filled in the current phase (written single-threaded during fill)
   */
  size_t _count = 0;

  /**
   * Packed claim counters: low 32 bits = elements claimed from the front, high 32 bits = elements
   * claimed from the back. A single atomic so front/back claims can't race onto the same slot.
   */
  std::atomic<uint64_t> _claim{0};
};

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
   * Pops (removes) up to maxCount elements from the front of the Deque under a single lock
   * acquisition, copying them into the provided buffer in front-to-back order.
   *
   * @note This is a thread safe operation.
   *
   * Batching amortizes the mutex cost across many elements. Under heavy multi-consumer contention
   * (dozens of threads each pulling one element at a time, as the Jaffar engine does when the
   * per-element work is cheap) the lock/unlock pair -- not the pop itself -- dominates wall time.
   * Grabbing a run of elements per lock cuts lock-acquisition traffic by up to maxCount.
   *
   * @param[out] elements Destination buffer; must have room for at least maxCount elements
   * @param[in] maxCount Maximum number of elements to pop
   * @return The number of elements actually popped (0 if the Deque was empty)
   */
  __JAFFAR_COMMON_INLINE__ size_t pop_front_get_batch(T* elements, const size_t maxCount)
  {
    _mutex.lock();

    size_t count = 0;
    while (count < maxCount && _internalDeque.empty() == false)
    {
      elements[count++] = _internalDeque.front();
      _internalDeque.pop_front();
    }
    if (count > 0) _size.fetch_sub(count, std::memory_order_relaxed);

    _mutex.unlock();
    return count;
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
