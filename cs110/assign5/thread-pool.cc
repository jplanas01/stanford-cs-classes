/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
#include <iostream>
#include "ostreamlock.h"
using namespace std;

/* Simply creates the dispatcher thread, which stands at the ready to process
 * any thunks that are scheduled.
 */
ThreadPool::ThreadPool(size_t numThreads) : wts(0), workers(0) {
    maxThreadCount = numThreads;
    dt = thread([this]() {
            dispatch();
        });

}

/* Spawns a new worker thread and adds information for it to the appropriate
 * status vectors.
 */
void ThreadPool::spawnNewWorker() {
    workersMutex.lock();
    size_t id = workers.size();
    WorkerInfo *wi = new WorkerInfo {};
    workers.push_back(wi);

    wts.push_back(thread([this](size_t id) {
                      worker(id);
                  }, id));
    workers[id]->state = WorkerIdle;
    workersMutex.unlock();

    countMutex.lock();
    availableWorkers++;
    countMutex.unlock();
}

/* Dispatches queued thunks to worker threads as they become available. If a
 * nullptr is enqueued, it is taken as a sentinel value meaning that the
 * ThreadPool is exiting, and that the dispatcher should exit.
 * Worker threads are lazily spawned, up to a maximum value specified at
 * initialization.
 */
void ThreadPool::dispatch() {
    while (true) {
        unique_lock<mutex> lk(queueMutex);
        queueReady.wait(lk, [this]{return thunkQueue.size() > 0;});

        if (availableWorkers == 0 && workers.size() <= maxThreadCount) {
            spawnNewWorker();
        } else {
            unique_lock<mutex> lk2(countMutex);
            workersDone.wait(lk2, [this] {
                    return availableWorkers > 0;});
        }

        // Guaranteed to have a process available at this point,
        // no chance of losing thunk.
        function<void(void)> thunk = thunkQueue.front();
        thunkQueue.pop();
        if (thunk == nullptr) {
            return;
        }

        for (size_t i = 0; i < workers.size(); i++) {
            if (workers[i]->state == WorkerIdle) {

                workersMutex.lock();
                workers[i]->state = WorkerBusy;
                workers[i]->thunk = thunk;
                workersMutex.unlock();

                countMutex.lock();
                availableWorkers--;
                countMutex.unlock();

                busyWorkers.notify_all();
                workers[i]->cv.notify_all();
                break;
            }
        }
    }
}

/* Worker thread process. Constantly executes thunks that are assigned to it
 * until the ThreadPool is destroyed.
 */
void ThreadPool::worker(size_t id) {
    while (true) {
        unique_lock<mutex> lk(workersMutex);
        workers[id]->cv.wait(lk, [this, id]{return workers[id]->state != WorkerIdle;});
        lk.unlock();
        if (workers[id]->state == WorkerExit) {
            return;
        }

        workers[id]->thunk();

        lk.lock();
        workers[id]->state = WorkerIdle;
        workers[id]->thunk = nullptr;
        lk.unlock();

        countMutex.lock();
        availableWorkers++;
        countMutex.unlock();
        busyWorkers.notify_all();
        workersDone.notify_all();
    }
}

/* Adds a thunk to the processing queue and notifies the dispatcher that it is
 * available.
 */
void ThreadPool::schedule(const function<void(void)>& thunk) {
    unique_lock<mutex> lk(queueMutex);
    thunkQueue.push(thunk);
    queueReady.notify_all();
}

/* Ensures that no more works is to be done.
 * Checks the thunk queue to see if it's empty and runs through all the workers
 * to ensure that they're not currently working.
 */
bool ThreadPool::checkBusy() {
    if (thunkQueue.size() > 0) {
        return false;
    }
    for (size_t id = 0; id < workers.size(); id++) {
        if (workers[id]->state == WorkerBusy) {
            return false;
        }
    }
    return true;
}

/* Blocks until all scheduled chunks have been processed.
 */
void ThreadPool::wait() {
    unique_lock<mutex> lk(workersMutex);
    busyWorkers.wait(lk, [this] {return checkBusy(); });
}

/* Clean up for the ThreadPool. Ensures all work has been finished, triggers the
 * dispatcher and notifies it that it's time to quit, notifies all workers that
 * it's time to quit, and frees up used memory.
 */
ThreadPool::~ThreadPool() {
    wait();
    // Signal dispatcher and worker threads to exit
    availableWorkers = 1; // Ensure that dispatcher thinks worker is available
                          // So it processes sentinel nullptr. Nothing else
                          // should be modifying this value since all workers
                          // are done and nothing else will be enqueued.
    schedule(nullptr);
    for (size_t id = 0; id < workers.size(); id++) {
        workers[id]->state = WorkerExit;
    }

    for (size_t id = 0; id < workers.size(); id++) {
        workers[id]->cv.notify_all();
    }
    
    // Join all threads
    for (thread &t : wts) {
        t.join();
    }

    // Free all memory allocated.
    for (WorkerInfo *wi : workers) {
        delete wi;
    }
    dt.join();
}
