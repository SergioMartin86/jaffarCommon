#pragma once

/**
 * @file dethreader.hpp
 * @brief Contains the classes for serializing threading-parallel applications
 */

#include <libco/libco.h>
#include <functional>
#include <memory>
#include <queue>

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
    * Runs or resume an already created thread
    */
    __INLINE__ void run() const { co_switch(_coroutine); }

    /**
     * Yields execution to the runtime
     */
    __INLINE__ void yield()
    {
      __runtime->setThreadSuspended();
      __runtime->yieldToRuntime();
    }

private:

    /**
     * Internal wrapper for the execution of the coroutine
     */
    __INLINE__ static void coroutineWrapper()
    {
      auto currentThread = __runtime->getCurrentThread();
      currentThread->_fc();
      __runtime->setThreadFinished();
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
        if (_threadFinished == false) _threadQueue.push(std::move(thread));
      }
  }

  /**
   * Function to indicate the current thread is now suspended
   */
  __INLINE__ void setThreadSuspended() { _threadFinished = false; }

  /**
   * Function to indicate the current thread is now finished
   */
  __INLINE__ void setThreadFinished() { _threadFinished = true; }

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

  private:

  /** 
  * Indicates that the currently scheduled thread has finished
  */
  bool _threadFinished;

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
