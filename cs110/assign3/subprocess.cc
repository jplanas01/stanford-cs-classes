/**
 * File: subprocess.cc
 * -------------------
 * Presents the implementation of the subprocess routine.
 */

#include "subprocess.h"
using namespace std;

/* Does the necessary redirection of file descriptors as specified by the
 * original function call in the soon-to-be-executed process.
 * argv is a NULL-terminated argument vector, pipes[0] is the pipe from which
 * the child will read, pipes[1] is to where the child will write. options[0]
 * specifies whether the parent process will supply child input, options[1]
 * specifies if the parent process will read from the child.
 *
 * Assumes all arguments are valid (argv is NULL-terminated argument vector, the
 * pipes have been initialized, and pipes and options both contain 2 elements.
 *
 * Throws SubprocessException if any system calls fail.
 */
static void child_func(char *argv[], int pipes[], bool options[]) {
    if (options[0]) {
        int err = dup2(pipes[0], STDIN_FILENO);
        if (err == -1) {
            throw SubprocessException("subprocess: dup2() call failed");
        }
    }

    if (options[1]) {
        int err = dup2(pipes[1], STDOUT_FILENO);
        if (err == -1) {
            throw SubprocessException("subprocess: dup2() call failed");
        }
    }

    // stdin and stdout store these pipes, don't need them
    close(pipes[0]);
    close(pipes[1]);

    execvp(argv[0], argv);
    throw SubprocessException("subprocess: execvp() failed");
}


/* Runs a command and optionally creates pipes allowing the caller to feed data
 * in or read data from an arbitrary process. Returns a struct with information
 * about any open pipes and the PID of the child. The parent is responsible for
 * closing these pipes when finished.
 *
 * argv is a NULL-terminated vector of arguments, the boolean names are
 * self-explanatory.
 *
 * Throws SubprocessException if any system calls fail.
 */
subprocess_t subprocess(char *argv[], bool supplyChildInput, bool ingestChildOutput) throw (SubprocessException) {
    subprocess_t result;

    /* Named from the parent's perspective. The parent will read from
     * read_pipes[0] and write to write_pipes[1], the child will read and write
     * from the remaining file descriptors.
     */
    int read_pipes[2];
    int write_pipes[2];

    if (pipe(read_pipes) == -1) {
        throw SubprocessException("subprocess: call to pipe() failed");   
    }
    if (pipe(write_pipes) == -1) {
        throw SubprocessException("subprocess: call to pipe() failed");   
    }

    pid_t child = fork();
    if (child == -1) {
        throw SubprocessException("subprocess: fork() failed");
    }

    if (child == 0) {
        bool opts[] = {supplyChildInput, ingestChildOutput};
        int pipes[] = {write_pipes[0], read_pipes[1]};
        
        // These pipes used by parent but not by child
        close(read_pipes[0]);
        close(write_pipes[1]);

        child_func(argv, pipes, opts);
        exit(1);
    }
    result.pid = child;

    if (supplyChildInput) {
        result.supplyfd = write_pipes[1];
    } else {
        result.supplyfd = kNotInUse;
        close(write_pipes[1]);
    }

    if (ingestChildOutput) {
        result.ingestfd = read_pipes[0];
    } else {
        result.ingestfd = kNotInUse;
        close(read_pipes[0]);
    }

    // Used by the child but not the parent
    close(write_pipes[0]);
    close(read_pipes[1]);

    return result;
}
