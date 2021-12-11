/**
 * File: thread-pool.h
 * -------------------
 * This class defines the ThreadPool class, which accepts a collection
 * of thunks (which are zero-argument functions that don't return a value)
 * and schedules them in a FIFO manner to be executed by a constant number
 * of child threads that exist solely to invoke previously scheduled thunks.
 */

#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>     // for size_t
#include <functional>  // for the function template used in the schedule signature
#include <thread>      // for thread
#include <vector>      // for vector
#include <queue>
#include <mutex>
#include <condition_variable>
#include "semaphore.h"

enum WorkerState { WorkerIdle, WorkerBusy, WorkerExit, WorkerInexistant };

struct WorkerInfo {
    std::function<void(void)> thunk;
    WorkerState state;
    std::condition_variable cv;
};

class ThreadPool {
 public:

/**
 * Constructs a ThreadPool configured to spawn up to the specified
 * number of threads.
 */
  ThreadPool(size_t numThreads);

/**
 * Schedules the provided thunk (which is something that can
 * be invoked as a zero-argument function without a return value)
 * to be executed by one of the ThreadPool's threads as soon as
 * all previously scheduled thunks have been handled.
 */
  void schedule(const std::function<void(void)>& thunk);

/**
 * Blocks and waits until all previously scheduled thunks
 * have been executed in full.
 */
  void wait();

/**
 * Waits for all previously scheduled thunks to execute, then waits
 * for all threads to be be destroyed, and then otherwise brings
 * down all resources associated with the ThreadPool.
 */
  ~ThreadPool();
  
 private:
  std::thread dt;                // dispatcher thread handle
  std::vector<std::thread> wts;  // worker thread handles

  void dispatch();
  void worker(size_t workerId);
  bool checkBusy();
  void spawnNewWorker();

  std::queue<std::function<void(void)> > thunkQueue;
  std::mutex queueMutex;
  std::mutex workersMutex;
  std::mutex countMutex;
  std::vector<WorkerInfo *> workers;
  std::condition_variable queueReady;
  std::condition_variable busyWorkers; // For some reason, trying to reuse this CV in place of
  std::condition_variable workersDone; // workersDone causes deadlocks sometimes. Any ideas, TA?
  bool destructInitiate = false;
  size_t maxThreadCount = 0;
  size_t availableWorkers = 0;

/**
 * ThreadPools are the type of thing that shouldn't be cloneable, since it's
 * not clear what it means to clone a ThreadPool (should copies of all outstanding
 * functions to be executed be copied?).
 *
 * In order to prevent cloning, we remove the copy constructor and the
 * assignment operator.  By doing so, the compiler will ensure we never clone
 * a ThreadPool.
 */
  ThreadPool(const ThreadPool& original) = delete;
  ThreadPool& operator=(const ThreadPool& rhs) = delete;
};

#endif
