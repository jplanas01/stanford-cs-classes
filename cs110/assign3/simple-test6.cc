/**
 * File: simple-test1.cc
 * ---------------------
 * Presents the implementation of a short nonsense program that makes a small number of
 * system calls.  The program can be run standalone, but it's really designed to be fed as
 * an argument to the trace executable, as with:
 * 
 *    > ./trace ./simple-test1
 */
#include <unistd.h>
#include <signal.h>
int main(int argc, char *argv[]) {
    int *i = (int *)0x0;
    //*i = 7;
    raise(SIGKILL);
    fork();
  return 0;
}
