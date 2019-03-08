#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define N_ITER 5

int main(void){
  pid_t pid;
  int counter;
  pid = fork();

  if(pid <0){
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if(pid ==0){
    while(1){
      for(counter =0;counter < N_ITER;counter++){
        printf("%d\n", counter);
        sleep(1);
      }
      sleep(3);
    }
  }
  while(wait(NULL)>0);
}
