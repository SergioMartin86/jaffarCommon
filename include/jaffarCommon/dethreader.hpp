#pragma once

/**
 * @file dethreader.hpp
 * @brief Contains the classes for serializing threading-parallel applications
 */

#include "exceptions.hpp"
#include "timing.hpp"
#include <functional>
#include <libco/libco.h>
#include <memory>
#include <queue>
#include <set>

/**
 *  This macro needs to be inserted in any .cpp file to define the singleton
 */
#define __JAFFAR_COMMON_DETHREADER_STATE                                                                                                                                           \
  namespace jaffarCommon                                                                                                                                                           \
  {                                                                                                                                                                                \
  namespace dethreader                                                                                                                                                             \
  {                                                                                                                                                                                \
  Runtime* __runtime = nullptr;                                                                                                                                                    \
  }                                                                                                                                                                                \
  }

/**
 * Size of each thread's stack
 */
#define __JAFFAR_COMMON_DETHREADER_STACK_SIZE 4 * 1024 * 1024

namespace jaffarCommon
{

namespace dethreader
{

class Runtime;

/**
 * Singleton pointer to the running runtime
 */
extern Runtime* __runtime;

/**
 * This library accepts only argument-less labmbda functions. Any required parameters need to be passed by capture
 */
typedef std::function<void()> threadFc_t;

/**
 * Identification type for a thread
 */
typedef uint64_t threadId_t;

/**
 * Represents a simulated kernel-level thread runtime scheduler
 * It uses user-level threads (coroutines) to carry their state and a Round Robin scheduling strategy to fairly distribute the CPU to them
 * The newly created threads execute non-preemptively so the user needs to insert yields in the code to volunteerly give up execution
 */
class Runtime
{
public:
  /**
   * Represents a simulated execution thread
   */
  class Thread
  {
  public:
    /**
     * An enumeration of the reasons why a thread might return
     */
    enum returnReason_t
    {
      /**
       * If the thread just yields execution without reason
       */
      none,

      /**
       * If the thread has finished
       */
      finished,

      /**
       * If the thread is set to sleep
       */
      sleeping,

      /**
       * If the thread is waiting
       */
      waiting
    };

    friend class Runtime;

    Thread()                      = delete;
    Thread(const Thread&)         = delete;
    void operator=(const Thread&) = delete;

    /**
     * Constructor
     *
     * @param[in] fc The (lambda) function to be executed
     * @param[in] id The unique identifier for the thread
     *
     * It creates the thread coroutine on construction
     */
    Thread(const threadFc_t fc, const threadId_t id) : _id(id), _fc(fc)
    {
      constexpr size_t stackSize = __JAFFAR_COMMON_DETHREADER_STACK_SIZE;
      _coroutine                 = co_create(stackSize, Thread::coroutineWrapper);
    }

    /**
     * This destructor frees up the coroutine stack memory
     */
    ~Thread() { co_delete(_coroutine); }

    /**
     * Runs or resume an already created thread.
     *
     * That is, unless the thread is sleeping
     */
    __JAFFARCOMMON__INLINE__ void run() const
    {
      // If the thread is sleep, checking if it has finished
      if (_returnReason == returnReason_t::sleeping)
      {
        auto timeDelta = timing::timeDeltaMicroseconds(timing::now(), _sleepStartTime);

        // If not, return now without continuing
        if (timeDelta < _sleepDuration) return;
      }

      // If the thread is waiting for another to finish, check it now
      if (_returnReason == returnReason_t::waiting)
        if (__runtime->getFinishedThreads().contains(_threadWaitedFor) == false) return;

      // Starting or continuing execution
      co_switch(_coroutine);
    }

    /**
     * Yields execution to the runtime
     */
    __JAFFARCOMMON__INLINE__ void yield()
    {
      _returnReason = returnReason_t::none;
      __runtime->yieldToRuntime();
    }

    /**
     * Returns the reason why the thread has returned
     *
     * @return The return reason
     */
    __JAFFARCOMMON__INLINE__ returnReason_t getReturnReason() const { return _returnReason; }

    /**
     * Function to send the thread to sleep
     *
     * @param[in] sleepDuration The number of microseconds to sleep for
     */
    __JAFFARCOMMON__INLINE__ void sleep(const size_t sleepDuration)
    {
      _sleepDuration  = sleepDuration;
      _returnReason   = returnReason_t::sleeping;
      _sleepStartTime = timing::now();
      __runtime->yieldToRuntime();
    }

    /**
     * States whether the thread is joinable
     *
     * @return Always true
     */
    __JAFFARCOMMON__INLINE__ bool joinable() { return true; }

    /**
     * Function to wait for a thread completion
     *
     * @param[in] threadId Identifier of the thread to wait for
     */
    __JAFFARCOMMON__INLINE__ void join(const threadId_t threadId)
    {
      _threadWaitedFor = threadId;
      _returnReason    = returnReason_t::waiting;
      __runtime->yieldToRuntime();
    }

    /**
     * Function to get the thread's id
     *
     * @return The unique id of the thread
     */
    __JAFFARCOMMON__INLINE__ threadId_t getThreadId() const { return _id; }

    /**
     * The thread this thread is waiting for
     */
    threadId_t _threadWaitedFor;

  private:
    /**
     * Sets the thread's return reason
     *
     * @param[in] returnReason The return reason
     */
    __JAFFARCOMMON__INLINE__ void setReturnReason(const returnReason_t returnReason) { _returnReason = returnReason; }

    /**
     * Internal wrapper for the execution of the coroutine
     */
    __JAFFARCOMMON__INLINE__ static void coroutineWrapper()
    {
      auto currentThread = __runtime->getCurrentThread();
      currentThread->_fc();
      currentThread->setReturnReason(returnReason_t::finished);
      __runtime->yieldToRuntime();
    }

    /**
     * Unique thread identifier
     */
    const threadId_t _id;

    /**
     * Copy of the thread function to execute
     */
    const threadFc_t _fc;

    /**
     * The internal coroutine (state) of the thread
     */
    cothread_t _coroutine;

    /**
     * Establishes the reason why a thread comes back to the runtime
     */
    returnReason_t _returnReason = none;

    /**
     * Number of microseconds to sleep for
     */
    size_t _sleepDuration;

    /**
     * Time point of sleep start
     */
    timing::timePoint _sleepStartTime;
  };

  Runtime() = default;

  /**
   * Creates a new thread and adds it to the thread queue
   *
   * @param[in] fc The function for the thread to execute
   * @return The identifier of the thread to wait for
   */
  __JAFFARCOMMON__INLINE__ threadId_t createThread(const threadFc_t fc)
  {
    const auto threadId = _uniqueThreadIdCounter;
    _threadQueue.push(std::make_unique<Thread>(fc, threadId));
    _uniqueThreadIdCounter++;
    return threadId;
  }

  /**
   * Initializes the runtime
   */
  __JAFFARCOMMON__INLINE__ void initialize()
  {
    // Setting singleton
    jaffarCommon::dethreader::__runtime = this;
  }

  /**
   * Finalizes the runtime
   */
  __JAFFARCOMMON__INLINE__ void finalize()
  {
    // unsetting singleton
    jaffarCommon::dethreader::__runtime = nullptr;
  }

  /**
   * Starts running the scheduler. It won't return until all previously created threads have fully finished executing
   */
  __JAFFARCOMMON__INLINE__ void run()
  {
    // Getting main coroutine
    _coroutine = co_active();

    // Starting to run threads until they are all finished
    while (_threadQueue.empty() == false)
    {
      // Obtaining next thread to run
      auto thread = std::move(_threadQueue.front());

      // Removing thread from the front
      _threadQueue.pop();

      // Setting current thread for execution
      setCurrentThread(thread.get());

      // Running thread
      thread->run();

      // If thread not finished, re-add to the back of the queue
      if (thread->getReturnReason() != Thread::returnReason_t::finished)
        _threadQueue.push(std::move(thread));
      else // Otherwise add it to the set of finished threads
        _finishedThreads.insert(thread->getThreadId());
    }
  }

  /**
   * Function to set the currently scheduled thread
   *
   * @param[in] thread The thread to set as current one
   */
  __JAFFARCOMMON__INLINE__ void setCurrentThread(Thread* const thread) { _currentThread = thread; }

  /**
   * Gets the current thread being scheduled
   *
   * @return The currently scheduled thread
   */
  __JAFFARCOMMON__INLINE__ Thread* getCurrentThread() const { return _currentThread; }

  /**
   * A function for the thread to yield back to the runtime system
   */
  __JAFFARCOMMON__INLINE__ void yieldToRuntime() { co_switch(_coroutine); }

  /**
   * Gets the list of finished threads
   *
   * @return The list of finished threads
   */
  __JAFFARCOMMON__INLINE__ const std::set<threadId_t>& getFinishedThreads() const { return _finishedThreads; }

private:
  /**
   * Unique thread Id counter
   */
  threadId_t _uniqueThreadIdCounter = 0;

  /**
   * A set containing all finished threads
   */
  std::set<threadId_t> _finishedThreads;

  /**
   * A pointer to the current thread
   */
  Thread* _currentThread = nullptr;

  /**
   * A queue of threads waiting to execute
   */
  std::queue<std::unique_ptr<Thread>> _threadQueue;

  /**
   * A pointer to the calling executing state
   */
  cothread_t _coroutine;
};

/**
 * Gets the current thread being scheduled
 *
 * @return The currently scheduled thread
 */
__JAFFARCOMMON__INLINE__ Runtime::Thread* getCurrentThread() { return __runtime->getCurrentThread(); }

/**
 * Publicly available Creates a new thread and adds it to the thread queue
 *
 * @param[in] fc The function for the thread to execute
 * @return The thread identifier of the new  thread
 */
__JAFFARCOMMON__INLINE__ threadId_t createThread(const threadFc_t fc)
{
  if (__runtime == nullptr) JAFFAR_THROW_LOGIC("Trying to use dethreader runtime before it is initialized");
  return __runtime->createThread(fc);
}

/**
 * Publicly avialable function to yield back to the runtime
 */
__JAFFARCOMMON__INLINE__ void yield()
{
  if (__runtime == nullptr) JAFFAR_THROW_LOGIC("Trying to use dethreader runtime before it is initialized");
  __runtime->getCurrentThread()->yield();
}

/**
 * Publicly available function to send the thread to sleep
 *
 * @param[in] sleepDuration The number of microseconds to sleep for
 */
__JAFFARCOMMON__INLINE__ void sleep(const size_t sleepDuration)
{
  if (__runtime == nullptr) JAFFAR_THROW_LOGIC("Trying to use dethreader runtime before it is initialized");
  __runtime->getCurrentThread()->sleep(sleepDuration);
}

/**
 * Function to wait for a thread completion
 *
 * @param[in] threadId Identifier of the thread to wait for
 */
__JAFFARCOMMON__INLINE__ void join(const threadId_t threadId)
{
  if (__runtime == nullptr) JAFFAR_THROW_LOGIC("Trying to use dethreader runtime before it is initialized");
  __runtime->getCurrentThread()->join(threadId);
}

} // namespace dethreader

} // namespace jaffarCommon
