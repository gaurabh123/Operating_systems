#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>

/**
 * Executes the command "cat scores | grep Lakers".  In this quick-and-dirty
 * implementation the parent doesn't wait for the child to finish and
 * so the command prompt may reappear before the child terminates.
 *
 */

int main(int argc, char** argv) {
    int pipe1[2], pipe2[2];
    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", "28", NULL};
    char *sort_args[] = {"sort", NULL};

    if(pipe(pipe1) == -1) {
        perror("Pipe failed");
        exit(1);
    }

    if(pipe(pipe2) == -1) {
        perror("Pipe failed");
        exit(1);
    }
    int real_out = dup(1);

    if (fork() == 0) { // Cat
        close(pipe1[0]);
        dup2(pipe1[1], 1); // Cat will write to pipe1's write buffer.
        execvp("cat", cat_args);
        exit(0);
    }
    if (fork() == 0) { // Grep
        sleep(1);
        close(pipe1[1]); // This doesn't need to write to pipe1's write buffer.
        dup2(pipe1[0], 0); // It does need to read frm pipe1's read buffer
        dup2(pipe2[1], 1); // It needs to write to pipe2's write buffer

        execvp("grep", grep_args);
        exit(0);
    }
    if (fork() == 0) { // Sort
        sleep(1);
        close(pipe2[1]); // It doesn't need to write to pipe2's write buffer.
        dup2(pipe2[0], 0); // It does need to read from pipe2's read buffer.
        dup2(real_out, 1);
        close(pipe1[0]);
        close(pipe1[1]);
        execvp("sort", sort_args);
        exit(0);
    }
    sleep(2);
    close(pipe2[0]);
    exit(0);
}
