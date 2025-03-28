#pragma once

/**
 * @file dethreader.hpp
 * @brief Contains the classes for serializing threading-parallel applications
 */

#include <libco/libco.h>
#include <functional>
#include <memory>
#include <queue>
#include "timing.hpp"

/**
 *  This macro needs to be inserted in any .cpp file to define the singleton
 */
#define __JAFFAR_COMMON_DETHREADER_STATE                                                                                                                                           \
  namespace jaffarCommon                                                                                                                                                           \
  {                                                                                                                                                                                \
  namespace dethreader                                                                                                                                                             \
  {                                                                                                                                                                                \
  Runtime *__runtime;                                                                                                                                                              \
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
extern Runtime *__runtime;

/**
 * This library accepts only argument-less labmbda functions. Any required parameters need to be passed by capture
 */
typedef std::function<void()> threadFc_t;

/**
 * Represents a simulated kernel-level thread runtime scheduler
 * It uses user-level threads (coroutines) to carry their state and a Round Robin scheduling strategy to fairly distribute the CPU to them
 * The newly created threads execute non-preemptively so the user needs to insert yields in the code to volunteerly give up execution
 */
class Runtime
{
  private:

  /**
   * Represents a simulated execution thread
   */
  class Thread
  {
   public:

    enum returnReason_t
    {
       none, 

       finished,

       yielding,

       sleeping
    };

    friend class Runtime;

    Thread()                       = delete;
    Thread(const Thread &)         = delete;
    void operator=(const Thread &) = delete;

    /**
     * Constructor
     * 
     * @param[in] fc The (lambda) function to be executed
     * 
     * It creates the thread coroutine on construction
     */
    Thread(const threadFc_t fc)
      : _fc(fc)
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
    __INLINE__ void run() const
    {
      // If the thread is sleep, checking if it has finished
      if (_returnReason == returnReason_t::sleeping)
      {
        auto timeDelta = timing::timeDeltaMicroseconds(timing::now(), _sleepStartTime);

        // If not, return now without continuing
        if (timeDelta < _sleepDuration) return;
      }

      // Starting or continuing execution
      co_switch(_coroutine);
    }

    /**
     * Yields execution to the runtime
     */
    __INLINE__ void yield()
    {
      _returnReason = returnReason_t::yielding;
      __runtime->yieldToRuntime();
    }

    /**
     * Returns the reason why the thread has returned
     * 
     * @return The return reason
     */
    __INLINE__ returnReason_t getReturnReason() const { return _returnReason; }

    /**
     * Function to send the thread to sleep
     * 
     * @param[in] sleepDuration The number of microseconds to sleep for
     */
    __INLINE__ void sleep(const size_t sleepDuration)
    {
      _sleepDuration = sleepDuration;
      _returnReason = returnReason_t::sleeping;
      _sleepStartTime = timing::now();
      __runtime->yieldToRuntime();
    }
  
private:

    /**
     * Sets the thread's return reason
     * 
     * @param[in] returnReason The return reason
     */
    __INLINE__ void setReturnReason(const returnReason_t returnReason) { _returnReason = returnReason; }

    /**
     * Internal wrapper for the execution of the coroutine
     */
    __INLINE__ static void coroutineWrapper()
    {
      auto currentThread = __runtime->getCurrentThread();
      currentThread->_fc();
      currentThread->setReturnReason(returnReason_t::finished);
      __runtime->yieldToRuntime();
    }

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

  public:

  Runtime() = default;

  /**
   * Creates a new thread and adds it to the thread queue
   * 
   * @param[in] fc The function for the thread to execute
   */
  __INLINE__ void createThread(const threadFc_t fc) { _threadQueue.push(std::make_unique<Thread>(fc)); }

  /**
   * Starts running the scheduler. It won't return until all previously created threads have fully finished executing
   */
  __INLINE__ void run()
  {
    // Setting singleton
    jaffarCommon::dethreader::__runtime = this;

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
        if (thread->getReturnReason() != Thread::returnReason_t::finished) _threadQueue.push(std::move(thread));
      }
  }

  /**
   * Function to set the currently scheduled thread
   * 
   * @param[in] thread The thread to set as current one
   */
  __INLINE__ void setCurrentThread(Thread *const thread) { _currentThread = thread; }

  /**
   * Gets the current thread being scheduled
   * 
   * @return The currently scheduled thread
   */
  __INLINE__ Thread *getCurrentThread() const { return _currentThread; }

  /**
   * A function for the thread to yield back to the runtime system
   */
  __INLINE__ void yieldToRuntime() { co_switch(_coroutine); }

  /**
   * Publicly avialable function to yield back to the runtime
   */
  __INLINE__ static void yield() { __runtime->getCurrentThread()->yield(); }

   /**
   * Publicly avialable function to send the thread to sleep
   * 
   * @param[in] milliseconds The number of microseconds to sleep for
   */
  __INLINE__ static void sleep(const size_t sleepDuration) { __runtime->getCurrentThread()->sleep(sleepDuration); }

  private:

  /**
   * A pointer to the current thread
   */
  Thread *_currentThread = nullptr;

  /**
   * A queue of threads waiting to execute
   */
  std::queue<std::unique_ptr<Thread>> _threadQueue;

  /**
   * A pointer to the calling executing state
   */
  cothread_t _coroutine;
};

} // namespace dethreader

} // namespace jaffarCommon
