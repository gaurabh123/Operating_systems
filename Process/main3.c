#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_ITER 30
#define MAX_SLEEP 10

int main() {
  pid_t child1, child2;

  // Seed the random number generator with the current time
  srandom(time(NULL));

  // Create the first child process
  child1 = fork();
  if (child1 < 0) {
      perror("Fork failed for child 1");
      exit(1);
  }

  if (child1 == 0) {
      // Child 1 process
      int iterations = random() % MAX_ITER + 1;  // Random number of iterations (1 to MAX_ITER)
      int i;
      for (i = 0; i < iterations; i++) {
          printf("Child Pid: %d is going to sleep!\n", getpid());
          int sleep_time = random() % MAX_SLEEP + 1;  // Sleep for random time (1 to MAX_SLEEP seconds)
          sleep(sleep_time);
          printf("Child Pid: %d is awake!\nWhere is my Parent: %d?\n", getpid(), getppid());
      }
      exit(0);  // Child terminates
  }

  // Create the second child process
  child2 = fork();
  if (child2 < 0) {
      perror("Fork failed for child 2");
      exit(1);
  }

  if (child2 == 0) {
      // Child 2 process
      int iterations = random() % MAX_ITER + 1;  // Random number of iterations (1 to MAX_ITER)
      int i;
      for (i = 0; i < iterations; i++) {
          printf("Child Pid: %d is going to sleep!\n", getpid());
          int sleep_time = random() % MAX_SLEEP + 1;  // Sleep for random time (1 to MAX_SLEEP seconds)
          sleep(sleep_time);
          printf("Child Pid: %d is awake!\nWhere is my Parent: %d?\n", getpid(), getppid());
      }
      exit(0);  // Child terminates
  }

  // Parent process waits for both children to terminate
  int status;
  pid_t terminated_pid;

  // Wait for the first child
  terminated_pid = wait(&status);
  printf("Child Pid: %d has completed\n", terminated_pid);

  // Wait for the second child
  terminated_pid = wait(&status);
  printf("Child Pid: %d has completed\n", terminated_pid);

  return 0;
}