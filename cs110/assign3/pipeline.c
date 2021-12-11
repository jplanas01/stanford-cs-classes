/**
 * File: pipeline.c
 * ----------------
 * Presents the implementation of the pipeline routine.
 */

#include "pipeline.h"
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>

/* Takes the output of one process and feeds it into the input of another.
 * argv1 and argv2 are null-terminated vectors specifying the processes to be
 * run, pids is a 2-element array: pids[0] is the sender, pids[1] is the
 * receiver of the data.
 */
void pipeline(char *argv1[], char *argv2[], pid_t pids[]) {
    int pipe_fd[2];
    pipe(pipe_fd);

    pids[0] = fork();
    if (pids[0] == 0) {
        // Child (sender)
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[0]);
        execvp(argv1[0], argv1);
        exit(0);
    }

    pids[1] = fork();
    if (pids[1] == 0) {
        // Second child (receiver)
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[1]);
        execvp(argv2[0], argv2);
        exit(0);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);
}
