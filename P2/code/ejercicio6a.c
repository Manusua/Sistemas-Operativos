#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define N_ITER 5

void manejador_SIGALARM(int sig){
  printf("Esta es una alarma\n" );
  return;
}


int main(void){
  pid_t pid;
  struct sigaction act;
  int counter;
  pid = fork();


  if(pid <0){
    perror("fork");
    exit(EXIT_FAILURE);
  } 

  if(pid ==0){
    sigemptyset(&(act.sa_mask));
    act.sa_handler = manejador_SIGALARM;
    alarm(40);
    if(sigaction(SIGALRM, &act, NULL) < 0){
      perror("Sigaction");
      exit(EXIT_FAILURE);
    }
    /*Entiendo que justo antes de comenzar cada bloque es lo mimso que al pinicpio de cada bloque*/
    while(1){
      sigaddset(&(act.sa_mask), SIGUSR1);
      sigaddset(&(act.sa_mask), SIGUSR2);
      sigaddset(&(act.sa_mask), SIGALRM);
      for(counter =0;counter < N_ITER;counter++){
        printf("%d\n", counter);
        sleep(1);
      }

      sigdelset(&(act.sa_mask), SIGUSR1);
      sigdelset(&(act.sa_mask), SIGALRM);
      sleep(3);
    }
  }
  while(wait(NULL)>0);
}
