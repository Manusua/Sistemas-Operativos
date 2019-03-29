/*
Fichero: ejercicio6a.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 29/03/2019
Descripción: ejercicio que nos permite comporbar la recepcion de las alarmas
*/

/* Librerías utilizadas*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define N_ITER 5


int main(void){
  pid_t pid;
  sigset_t set1, set2, setaux;
  int counter;
  pid = fork();


  if(pid <0){
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if(pid ==0){
    alarm(40);

    /*Entiendo que justo antes de comenzar cada bloque es lo mimso que al pinicpio de cada bloque*/
    while(1){
      sigemptyset(&set1);
      printf("Bloqueando SIGUSR1\n");
      sigaddset(&set1, SIGUSR1);
      printf("Bloqueando SIGUSR2\n");
      sigaddset(&set1, SIGUSR2);
      printf("Bloqueando SIGALARM\n");
      sigaddset(&set1, SIGALRM);

      if (sigprocmask(SIG_BLOCK, &set1, &setaux) < 0) {
          perror("sigprocmask");
          exit(EXIT_FAILURE);
      }
      for(counter = 0;counter < N_ITER;counter++){
        printf("%d\n", counter);
        sleep(1);
      }
      /*Metemos en set2 las señale sque queremos desbloquear*/
      sigaddset(&set2, SIGUSR1);
      sigaddset(&set2, SIGALRM);

      if (sigprocmask(SIG_UNBLOCK, &set2, &setaux) < 0) {
          perror("sigprocmask");
          exit(EXIT_FAILURE);
      }
      sleep(3);
    }
  }
  while(wait(NULL)>0);
}
