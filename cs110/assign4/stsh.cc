/**
 * File: stsh.cc
 * -------------
 * Defines the entry point of the stsh executable.
 */

#include "stsh-parser/stsh-parse.h"
#include "stsh-parser/stsh-readline.h"
#include "stsh-parser/stsh-parse-exception.h"
#include "stsh-signal.h"
#include "stsh-job-list.h"
#include "stsh-job.h"
#include "stsh-process.h"
#include "stsh-parse-utils.h"
#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>  // for fork
#include <signal.h>  // for kill
#include <sys/wait.h>
#include <assert.h>
using namespace std;

static STSHJobList joblist; // the one piece of global data we need so signal handlers can access it

/* Blocks until the current foreground job is no longer in the foreground.
 */
static void waitOnForeground(STSHJob &job) {
    sigset_t additions, existingmask;
    sigemptyset(&additions);
    sigaddset(&additions, SIGCHLD);
    sigprocmask(SIG_BLOCK, &additions, &existingmask);

    while (joblist.hasForegroundJob()) {
        sigsuspend(&existingmask);
    }

    sigprocmask(SIG_UNBLOCK, &additions, NULL);
}

/* Checks if a job has a process in the stopped state.
 */
bool hasStoppedProcess(STSHJob &job) {
    vector<STSHProcess> &procs = job.getProcesses();
    for (const STSHProcess &proc : procs) {
        if (proc.getState() == kStopped) {
            return true;
        }
    }
    return false;
}

/* Handles the foreground and background builtins, bringing a background job to
 * the foreground (continuing if necessary) or continuing a stopped process in
 * the background.
 *
 * Assumes that p is a proper pipeline.
 */
void groundCommand(const pipeline &p, string name, STSHJobState state, bool fg) {
    try { 
        size_t num = parseNumber(p.commands[0].tokens[0], "Usage: " + name + " <jobid>.");
        if (!joblist.containsJob(num)) {
            throw STSHException(name + " " + to_string(num) + ": No such job.");
            return;
        }
        STSHJob &job = joblist.getJob(num);
        job.setState(state);
        if (hasStoppedProcess(job)) {
            kill(-job.getGroupID(), SIGCONT);
        }
        if (fg) {
            waitOnForeground(job);
        }
    } catch (STSHException e) {
        cout << e.what() << endl;
    }
}

/* Counts the number of tokens in a NULL-terminated array.
 */
size_t countTokens(char* const *tokens) {
    int i;
    for (i = 0; tokens[i] != NULL; i++);
    return i;
}

/* Signals the specified process with matching ID, optionally checking that it
 * isn't in a certain state. Does not handle processes not spawned by the
 * program (ie, not in the joblist).
 *
 * pid is the PID of the child process
 * signal is the signal number.
 * checkState indicates whether the state of the program should be checked: if
 * the state matches the one provided, no signal is sent.
 * state is the state to check against. Ignored if checkState is false.
 *
 * Assumes that signal is a valid signal number.
 */
void signalPid(pid_t pid, int signal, bool checkState, STSHProcessState state) {
    if (!joblist.containsProcess(pid)) {
        throw STSHException("No proces with pid " + to_string(pid) + ".");
    }
    if (checkState) {
        STSHJob &job = joblist.getJobWithProcess(pid);
        STSHProcess &proc = job.getProcess(pid);
        if (proc.getState() == state) {
            return;
        }
    }
    kill(pid, signal);
}

/* Signals the process with specified job number and index, optionally checking
 * that it isn't in a certain state.
 *
 * jobnum is the job number.
 * index is the index within that job number.
 * checkState indicates whether the state of the program should be checked: if
 * the state matches the one provided, no signal is sent.
 * state is the state to check against. Ignored if checkState is false.
 *
 * Assumes that signal is a valid signal number.
 */
void signalJob(size_t jobnum, size_t index, int signal, bool checkState, STSHProcessState state) {
    if (!joblist.containsJob(jobnum)) {
        throw STSHException("No job with id of " + to_string(jobnum) + ".");
    }
    STSHJob &job = joblist.getJob(jobnum);
    vector<STSHProcess> &procs = job.getProcesses();
    if (index > procs.size() - 1) {
        throw STSHException("Job " + to_string(jobnum) + " doesn't have a process at index " + to_string(index) + ".");
    }
    if (checkState) {
        if (procs[index].getState() == state) {
            return;
        }
    }
    kill(procs[index].getID(), signal);
}

/* Handles the slay, halt, and cont builtins. Forwards the specified signal to
 * the process with a specific ID or the process with a specified job number and
 * index within that job.
 * If one argument is supplied, it's assumed to be a PID; otherwise, it's
 * assumed to be a job number and index within that job.
 *
 * p is the pipeline that the was created by typing in the builtin.
 * name is a string containing the name of the builtin.
 * signal is the signal to be sent to the command.
 * checkState indicates whether the state of the program should be checked: if
 * the state matches the one provided, no signal is sent.
 * state is the state to check against. Ignored if checkState is false.
 *
 * Assumes that p is a proper pipeline, signal is a valid signal number.
 */
void slayCommand(const pipeline &p, string name, int signal, bool checkState, STSHProcessState state) {
    try {
        size_t num1 = parseNumber(p.commands[0].tokens[0], "Usage: " + name +  " <jobid> <index> | <pid>.");
        size_t num2 = 0;
        if (countTokens(p.commands[0].tokens) == 2) {
            num2 = parseNumber(p.commands[0].tokens[1], "Usage: " + name + " <jobid> <index> | <pid>.");
            signalJob(num1, num2, signal, checkState, state);
            return;
        } else {
            signalPid(num1, signal, checkState, state);
        }
    } catch (STSHException e) {
        cout << e.what() << endl;
    }
}

/**
 * Function: handleBuiltin
 * -----------------------
 * Examines the leading command of the provided pipeline to see if
 * it's a shell builtin, and if so, handles and executes it.  handleBuiltin
 * returns true if the command is a builtin, and false otherwise.
 */
static const string kSupportedBuiltins[] = {"quit", "exit", "fg", "bg", "slay", "halt", "cont", "jobs"};
static const size_t kNumSupportedBuiltins = sizeof(kSupportedBuiltins)/sizeof(kSupportedBuiltins[0]);
static bool handleBuiltin(const pipeline& pipeline) {
    const string& command = pipeline.commands[0].command;
    auto iter = find(kSupportedBuiltins, kSupportedBuiltins + kNumSupportedBuiltins, command);
    if (iter == kSupportedBuiltins + kNumSupportedBuiltins) return false;
    size_t index = iter - kSupportedBuiltins;

    switch (index) {
        case 0:
        case 1: 
            exit(0);
            break;
        case 2:
            groundCommand(pipeline, kSupportedBuiltins[index], kForeground, true);
            break;
        case 3:
            groundCommand(pipeline, kSupportedBuiltins[index], kBackground, false);
            break;
        case 4:
            slayCommand(pipeline, kSupportedBuiltins[index], SIGKILL, false, kRunning);
            break;
        case 5:
            slayCommand(pipeline, kSupportedBuiltins[index], SIGSTOP, true, kStopped);
            break;
        case 6:
            slayCommand(pipeline, kSupportedBuiltins[index], SIGCONT, true, kRunning);
            break;
        case 7: cout << joblist; break;
        default: throw STSHException("Internal Error: Builtin command not supported."); // or not implemented yet
    }

    return true;
}

/* Updates the process in the global job list to the update state. Calls
 * joblist.synchronize() to handle any dead or backgrounded jobs.
 */
static void updateJobList(pid_t pid, STSHProcessState state) {
     if (!joblist.containsProcess(pid)) {
         return;
     }
     STSHJob& job = joblist.getJobWithProcess(pid);
     assert(job.containsProcess(pid));

     STSHProcess& process = job.getProcess(pid);
     process.setState(state);
     joblist.synchronize(job);
}

/* SIGCHLD handler, called whenever a child terminates, stops, or continues.
 * Updates the global job list with the changed status of the child.
 */
static void reapChild(int sig) {
    pid_t pid;
    int status;
    STSHProcessState state = kRunning;
    while (true) {
        pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (pid <= 0) {
            break;
        }
        if (WIFEXITED(status)) {
            state = kTerminated;
        } else if (WIFSTOPPED(status)) {
            state = kStopped;
        } else if (WIFSIGNALED(status)) {
            state = kTerminated;
        } else if (WIFCONTINUED(status)) {
            state = kRunning;
        }
        updateJobList(pid, state);
    }
}

/* SIGINT and SIGTSTP handler. Dispatches the received signal to the foreground
 * process, if any; otherwise does nothing. If the received signal was a ^Z, it
 * puts the foreground process in the background.
 */
static void handleInt(int sig) {
    if (!joblist.hasForegroundJob()) {
        return;
    }
    
    STSHJob &job = joblist.getForegroundJob();
    pid_t gid = job.getGroupID();
    if (sig == SIGTSTP) {
        job.setState(kBackground);
    }
    kill(-gid, sig);
}

/* Populates the given argv pointer array with the details in the provided
 * command structure, NULL terminating the vector.
 *
 * Assumes that argv has enough space to hold all tokens in cmd, an
 * additional NULL, and the program name.
 */
static void makeArgv(const struct command &cmd, char *argv[]) {
    argv[0] = const_cast<char *>(cmd.command);
    size_t i;
    for (i = 0; i <= kMaxArguments && cmd.tokens[i] != NULL; i++) {
        argv[i + 1] = cmd.tokens[i];
    }
    argv[i + 1] = NULL;
}

/**
 * Function: installSignalHandlers
 * -------------------------------
 * Installs user-defined signals handlers for four signals
 * (once you've implemented signal handlers for SIGCHLD, 
 * SIGINT, and SIGTSTP, you'll add more installSignalHandler calls) and 
 * ignores two others.
 */
static void installSignalHandlers() {
    installSignalHandler(SIGQUIT, [](int sig) { exit(0); });
    installSignalHandler(SIGTTIN, SIG_IGN);
    installSignalHandler(SIGTTOU, SIG_IGN);
    installSignalHandler(SIGCHLD, reapChild);
    installSignalHandler(SIGCONT, reapChild);
    installSignalHandler(SIGINT, handleInt);
    installSignalHandler(SIGTSTP, handleInt);
}

/* Adds all STSHProcesses in the vector to the global joblist in a new job that
 * has the specified job state and returns the job number.
 */
static size_t addJob(const vector<STSHProcess>& processes, STSHJobState state) {
    STSHJob& job = joblist.addJob(state);
    for (const STSHProcess& process : processes) job.addProcess(process);
    return job.getNum(); // surface the job number the job was
}

/* Forks off and executes the first command in the pipeline, setting up piping
 * and redirection as necessary. Also sets the group ID of the child process.
 * cmd is the command structure containing the program and arguments. Handles
 * single-command pipelines and multiple-command pipelines.
 *
 * pipes is a pointer to an int array of size 2 that has pipe file descriptors
 * in the order {read_pipe, write_pipe}
 * input is the name of the file to draw input from, or empty if none.
 * output is the name of the file to output to, or empty if none (only used if
 * no pipes are specified in the command line).
 *
 * Assumes that the FDs passed in through pipes are valid pipe() file
 * descriptors and that cmd has been populated correctly.
 *
 * Returns the pid of the child process.
 */
static pid_t forkAndExecFirst(const struct command &cmd, int pipes[], string input, string output) {
    char *argv[kMaxArguments + 2]; // 1 for command name, 1 for NULL   
    makeArgv(cmd, argv);

    int in_fd = -1;
    int out_fd = -1;
    if (input != "") {
        in_fd = open(input.c_str(), O_RDONLY);
        if (in_fd < 0) {
            throw STSHException("Could not open \"" + input + "\".");
        }
    }

    if (output != "") {
        out_fd = creat(output.c_str(), 0644);
        if (out_fd < 0) {
            throw STSHException("Could not open \"" + output + "\".");
        }
    }

    pid_t pid;
    pid = fork();
    if (pid == 0) {
        if (pipes != NULL) {
            dup2(pipes[1], STDOUT_FILENO);
            close(pipes[0]);
            close(pipes[1]);
        } else if (out_fd != -1) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        if (in_fd != -1) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        setpgid(getpid(), getpid());
        execvp(argv[0], argv); 
        throw STSHException(string(argv[0]) + ": Command not found.");
        exit(0);
    }
    setpgid(pid, pid);
    if (pipes != NULL) {
        //close(pipes[0]);
        close(pipes[1]);
    }
    return pid;
}

/* Forks off and executes any non-first and non-last processes in the command
 * line. These must have redirected input and output as they are in the middle
 * of the pipeline. The child process has its group ID set to the specified
 * number.
 *
 * cmd is the command structure containing programs and arguments.
 * group is the group ID to assign to children.
 * in_pipe is the input file descriptor.
 * out_pipe is the output file descriptor.
 *
 * cmd is assumed to be populated correctly from the pipeline parser, group is
 * assumed to be a valid group ID, in_pipe and out_pipe are both assumed to be
 * valid file descriptors.
 *
 * Returns the PID of the forked child.
 */
static pid_t forkAndExecMid(const struct command &cmd, pid_t group, int in_pipe, int out_pipe) {
    char *argv[kMaxArguments + 2]; // 1 for command name, 1 for NULL   
    makeArgv(cmd, argv);


    pid_t pid;
    pid = fork();
    if (pid == 0) {
        dup2(in_pipe, STDIN_FILENO);
        dup2(out_pipe, STDOUT_FILENO);
        close(in_pipe);
        close(out_pipe);

        setpgid(getpid(), group);
        execvp(argv[0], argv); 
        throw STSHException(string(argv[0]) + ": Command not found.");
        exit(0);
    }
    setpgid(pid, group);
    close(in_pipe);
    close(out_pipe);
    return pid;
}

/* Forks off and executes the last process in the pipeline. This process has
 * input pipe and optionally outputs to a file. The child process has its group
 * ID set to the specified number.
 *
 * cmd is the command structure containing programs and arguments.
 * group is the group ID to assign to children.
 * pipes is the input file descriptor from which to read.
 * output is the output file name to redirect to, truncating the file if it
 * exists. Can be blank to indicate no output redirection.
 *
 * Assumes that cmd is valid as generated by the pipeline parser, group is a
 * valid group ID, in_pipe is a valid file descriptor.
 *
 * Returns the PID of the forked child.
 */
static pid_t forkAndExecLast(const struct command &cmd, pid_t group, int in_pipe, string output) {
    char *argv[kMaxArguments + 2]; // 1 for command name, 1 for NULL   
    makeArgv(cmd, argv);

    int out_fd = -1;
    if (output != "") {
        out_fd = creat(output.c_str(), 0644);
        if (out_fd < 0) {
            throw STSHException("Could not open \"" + output + "\".");
        }
    }

    pid_t pid;
    pid = fork();
    if (pid == 0) {
        dup2(in_pipe, STDIN_FILENO);
        close(in_pipe);

        if (out_fd != -1) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        setpgid(getpid(), group);
        execvp(argv[0], argv); 
        throw STSHException(string(argv[0]) + ": Command not found.");
        exit(0);
    }
    setpgid(pid, group);
    close(in_pipe);
    return pid;
}

/**
 * Function: createJob
 * -------------------
 * Creates a new job on behalf of the provided pipeline.
 */
static void createJob(const pipeline& p) {
    vector<STSHProcess> procs;

    size_t npipes = p.commands.size() - 1;
    int pipes[npipes][2];
    for (size_t i = 0; i < npipes; i++) {
        pipe(pipes[i]);
    }

    pid_t gid;
    pid_t pid;

    // Exec first, get pid for group, set up redirect input if necessary
    // This one is guaranteed to exist.
    if (npipes == 0) {
        gid = forkAndExecFirst(p.commands[0], NULL, p.input, p.output);
    } else {
        gid = forkAndExecFirst(p.commands[0], pipes[0], p.input, "");
    }
    procs.push_back(STSHProcess(gid, p.commands[0]));

    if (npipes != 0) {
        // Exec rest but not last
        for (size_t i = 0; i < npipes - 1; i++) {
            pid = forkAndExecMid(p.commands[i+1], gid, pipes[i][0], pipes[i+1][1]);
            procs.push_back(STSHProcess(pid, p.commands[i+1]));
        }

        // Exec last, set up redirect output if necessary
        pid = forkAndExecLast(p.commands[npipes], gid, pipes[npipes - 1][0], p.output);
        procs.push_back(STSHProcess(pid, p.commands[npipes]));
    }

    // Hand over control to the process group
    int err = tcsetpgrp(STDIN_FILENO, gid);
    if (err != 0 && errno != ENOTTY) {
        throw STSHException("Error transferring control of terminal to process.");
    }

    size_t jobnum = addJob(procs, p.background ? kBackground : kForeground); 

    if (!p.background) {
        waitOnForeground(joblist.getJob(jobnum));
    } else {
        cout << "[" << jobnum << "] ";
        size_t i;
        for (i = 0; i < procs.size() - 1; i++) {
            cout << procs[i].getID() << " ";
        }
        cout << procs[i].getID() << endl;
    }

}

/**
 * Function: main
 * --------------
 * Defines the entry point for a process running stsh.
 * The main function is little more than a read-eval-print
 * loop (i.e. a repl).  
 */
int main(int argc, char *argv[]) {
    pid_t stshpid = getpid();
    installSignalHandlers();
    rlinit(argc, argv);
    while (true) {
        string line;
        if (!readline(line)) break;
        if (line.empty()) continue;
        try {
            pipeline p(line);
            bool builtin = handleBuiltin(p);
            if (!builtin) createJob(p);

            // Regain control of the terminal
            tcsetpgrp(STDIN_FILENO, stshpid);
        } catch (const STSHException& e) {
            cerr << e.what() << endl << flush;
            if (getpid() != stshpid) exit(0); // if exception is thrown from child process, kill it
        }
    }

    return 0;
}
