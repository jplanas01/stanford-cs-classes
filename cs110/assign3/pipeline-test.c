/**
 * File: pipeline-test.c
 * ---------------------
 * Exercises the pipeline function to verify 
 * basic functionality.
 */

#include "pipeline.h"
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

static void printArgumentVector(char *argv[]) {
    if (argv == NULL || *argv == NULL) {
        printf("<empty>");
        return;
    }

    while (true) {
        printf("%s", *argv);
        argv++;
        if (*argv == NULL) return;
        printf(" ");
    }
}

static void summarizePipeline(char *argv1[], char *argv2[]) {
    printf("Pipeline: ");
    printArgumentVector(argv1);
    printf(" -> ");
    printArgumentVector(argv2);
    printf("\n");
}

static void launchPipedExecutables(char *argv1[], char *argv2[]) {
    summarizePipeline(argv1, argv2);
    pid_t pids[2];
    pipeline(argv1, argv2, pids);
    waitpid(pids[0], NULL, 0);
    waitpid(pids[1], NULL, 0);
}

static void simpleTest() {
    char *argv1[] = {"cat", "/usr/include/tar.h", NULL};
    char *argv2[] = {"wc", NULL};
    launchPipedExecutables(argv1, argv2);
}

/* Another simple test */
static void secondTest() {
    char *argv1[] = {"ls", NULL};
    char *argv2[] = {"wc", "-l", NULL};
    launchPipedExecutables(argv1, argv2);
}

/* Test case when STDIN takes indeterminate amount of time to be closed. Type in
 * gibberish manually, then terminate input with ^D.
 */
/*
static void thirdTest() {
    char *argv1[] = {"cat", NULL};
    char *argv2[] = {"wc", "-l", NULL};
    launchPipedExecutables(argv1, argv2);
}
*/

/* Add some self-reference for fun.
 */
static void fourthTest() {
    char *argv1[] = {"./pipeline-test", NULL};
    char *argv2[] = {"wc", "-l", NULL};
    launchPipedExecutables(argv1, argv2);
}

int main(int argc, char *argv[]) {
    // thirdTest and fourthTest can't be run simultaneously
    // while pipeline-test is waitpid()ing, cat is expecting a ^D, which
    // pipeline-test eats.
    void (*fp[])(void) = {simpleTest, secondTest, fourthTest};
    for (int i = 0; i < sizeof(fp) / sizeof(fp[0]); i++) {
        fp[i]();
    }
    return 0;
}
