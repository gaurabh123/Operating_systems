/* hello_signal.c */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

time_t start, stop;

void handler1(int signum)
{ //signal handler for SIGALRM
  printf("Hello World!\n");
}

void handler2(int signum)
{ //signal handler for SIGINT
  stop = time(NULL);
  int difference = stop - start;
  printf("\nTime of execution: %d seconds.\n", difference);
  exit(1);
}

int main(int argc, char * argv[])
{
  start = time(NULL);
  signal(SIGALRM,handler1); 
  signal(SIGINT,handler2);
  while(1){
    alarm(1);
    sleep(1);
    printf("Turing was right!\n");
  }
  return 0; 
}