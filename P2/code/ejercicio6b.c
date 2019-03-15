#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define N_ITER 5

void manejador_SIGTERM(int sig){
  /*Pongo %d en vez de %ld para que no se queje el compilador*/
  printf("Soy %d y he recibido la señal SIGTERM\n", getpid());
  exit(EXIT_SUCCESS);
  return;
}

int main (void){
  pid_t pid;
  int counter;
  struct sigaction act;


  pid = fork();
  if(pid < 0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if(pid == 0){
    while(1){
      act.sa_handler = manejador_SIGTERM;
      act.sa_flags = 0;
      sigemptyset(&(act.sa_mask));
      if(sigaction(SIGTERM, &act, NULL) < 0){
        perror("Sigaction");
        exit(EXIT_FAILURE);
      }
      for(counter = 0; counter < N_ITER; counter++){
        printf("%d\n", counter);
        sleep(1);
      }
      sleep(3);
    }
  }
  else{
    sleep(4);
    kill(pid,SIGTERM);
    wait(NULL);
  }
}

//Cuando el hijo haya finalizado su ejecucion el apdre terminara
