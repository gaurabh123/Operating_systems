#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>


void  ClientProcess(int []);

int  main(int  argc, char *argv[])
{
     int    ShmID;
     int    *ShmPTR;
     pid_t  pid;
     int    status;

     ShmID = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Server has received a shared memory of 2 integers...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     printf("Server has attached the shared memory...\n");

     ShmPTR[0] = 0;
     ShmPTR[1] = 0;

     printf("Server is about to fork a child process...\n");
     pid = fork();
     if (pid < 0) {
          printf("*** fork error (server) ***\n");
          exit(1);
     }
     else if (pid == 0) {
          ClientProcess(ShmPTR);
          exit(0);
     }

    // Exit when Child has exited
    
     int i = 0;
     int account, balance;
     int sleep_time;
     srand(42);
     while (waitpid(pid, &status, WNOHANG) == 0) { // Child is still running...
      while (i <25) {
        
        sleep_time = rand() % 6; // 0 - 5
        sleep(sleep_time);
        while (ShmPTR[1] != 0);
        account = ShmPTR[0];
        if (account <= 100) {
          balance = rand() % 101; // 0 - 100

          if (balance % 2 == 0) {
            account += balance;
            printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
          } else {
            printf("Dear old Dad: Doesn't have any money to give\n");
          }
          ShmPTR[0] = account;
        }
        ShmPTR[1] = 1;
        i++;
      }
     }
     printf("Parent has detected the completion of its child...\n");
     shmdt((void *) ShmPTR);
     printf("Parent has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Parent has removed its shared memory...\n");
     printf("Parent exits...\n");
     exit(0);
}

void  ClientProcess(int  SharedMem[])
{
     srand(54);
     int i = 0;
     int account, balance;
     int sleep_time;
     while (i < 25) {
      sleep_time = rand() % 6;
      sleep(sleep_time);
      while (SharedMem[1] != 1);
      balance = rand() % 51;
      account = SharedMem[0];
      printf("Poor Student needs $%d\n", balance);
      if (balance <= account) {
        account -= balance;
        printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
      } else {
        printf("Poor Student: Not Enough Cash ($%d)\n", account );
      }
      SharedMem[0] = account;
      SharedMem[1] = 0;
      i++;
     }
     printf("Child Exits\n");
}