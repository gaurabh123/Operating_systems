/* hello_signal.c */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handler(int signum)
{
  printf("Hello World!\n");
}

int main(int argc, char * argv[])
{
  signal(SIGALRM,handler); 
  while(1){
    alarm(1);
    sleep(1);
    printf("Turing was right!\n");
  }
  return 0;
}