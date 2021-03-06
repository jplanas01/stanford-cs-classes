#include <cassert>
#include <ctime>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include "subprocess.h"

using namespace std;

struct worker {
    worker() {}
    worker(char *argv[]) : sp(subprocess(argv, true, false)), available(false) {}
    subprocess_t sp;
    bool available;
};

static const size_t kNumCPUs = sysconf(_SC_NPROCESSORS_ONLN);
// restore static keyword once you start using these, commented out to suppress compiler warning
static vector<worker> workers(kNumCPUs);
static size_t numWorkersAvailable = 0;

/* Signal handler: Once a worker stops, a SIGCHLD is sent to this process and
 * the status of that worker is marked as available.
 */
static void markWorkerAsAvailable(int sig) {
    pid_t pid;
    int status;
    while (true) {
        pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
        if (pid <= 0) {
            break;
        }

        for (size_t i = 0; i < workers.size(); i++) {
            if (workers[i].sp.pid == pid) {
                // Found match
                workers[i].available = true;
                numWorkersAvailable++;
                //cout << "Marked " << i << " available." << endl;
                break;
            }
        }

    }
}

static const char *kWorkerArguments[] = {"./factor.py", "--self-halting", NULL};
/* Starts a subprocess for every CPU core and adds information about each worker
 * to the workers vector.
 */
static void spawnAllWorkers() {
    cout << "There are this many CPUs: " << kNumCPUs << ", numbered 0 through " << kNumCPUs - 1 << "." << endl;

    cpu_set_t cpus;
    for (size_t i = 0; i < kNumCPUs; i++) {
        CPU_ZERO(&cpus);
        CPU_SET(i, &cpus);
        try {
            struct worker w(const_cast<char **>(kWorkerArguments));
            workers[i] = w;
            cout << "Worker " << w.sp.pid << " is set to run on CPU " << i << "." << endl;
            sched_setaffinity(w.sp.pid, sizeof(cpu_set_t), &cpus);
        } catch (SubprocessException e) {
            cout << "Error creating worker: " << e.what() << endl;
        }

    }
}

/* Gets the index of the lowest-numbered available worker in the workers vector.
 * If none are available, the function blocks until one becomes available.
 */
static size_t getAvailableWorker() {
    sigset_t additions, existingmask;
    sigemptyset(&additions);
    sigaddset(&additions, SIGCHLD);
    sigprocmask(SIG_BLOCK, &additions, &existingmask);

    while (numWorkersAvailable == 0) {
        sigsuspend(&existingmask);
    }
    for (size_t i = 0; i < workers.size(); i++) {
        if (workers[i].available) {
            sigprocmask(SIG_UNBLOCK, &additions, NULL);
            return i;
        }
    }

    sigprocmask(SIG_UNBLOCK, &additions, NULL);
    return 0;
}

/* Reads in a number from standard in and passes it to the next available
 * worker. If none are available, the function blocks until one is available and
 * then sends the number.
 * If an invalid number is entered, a message to this effect is displayed and
 * the program exits.
 */
static void broadcastNumbersToWorkers() {
    while (true) {
        string line;
        getline(cin, line);
        if (cin.fail()) break;
        size_t endpos;
        try {
            long long num = stoll(line, &endpos);
            if (endpos != line.size()) break;
            size_t work_num = getAvailableWorker();

            //cout << "Sending " << num << " to worker #" << work_num << endl;
            workers[work_num].available = false;
            numWorkersAvailable--;
            kill(workers[work_num].sp.pid, SIGCONT);
            dprintf(workers[work_num].sp.supplyfd, "%lld\n", num);
        } catch (invalid_argument e) {
            cout << "Invalid number entered." << endl;
            break;
        }
    }
}

/* Waits for all workers to finish their current workload, harvests the dead
 * processes.
 */
static void closeAllWorkers() {
    signal(SIGCHLD, SIG_DFL);
    for (size_t i = 0; i < workers.size(); i++) {
        if (!workers[i].available) {
            waitpid(workers[i].sp.pid, NULL, WUNTRACED);
        }
        kill(workers[i].sp.pid, SIGCONT);
        close(workers[i].sp.supplyfd);
    }

    for (size_t i = 0; i < workers.size(); i++) {
        waitpid(workers[i].sp.pid, NULL, 0);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, markWorkerAsAvailable);
    spawnAllWorkers();
    broadcastNumbersToWorkers();
    closeAllWorkers();
    return 0;
}
