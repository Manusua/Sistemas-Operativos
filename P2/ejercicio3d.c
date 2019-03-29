/*
Fichero: ejercicio3d.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 29/03/2019
Descripción: ejercicio en el que tratamos de capturar SIGKILL
*/

/* Librerías utilizadas*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
/* manejador: rutina de tratamiento de la señal SIGINT. */
void manejador(int sig){
  printf("He conseguido capturar SIGKILL");
  fflush(stdout);
}

int main(void){
  struct sigaction act;
  act.sa_handler = manejador;
  sigemptyset(&(act.sa_mask));
  act.sa_flags =0;
  if(sigaction(SIGKILL,&act,NULL)<0){
    perror("sigaction");
    exit(EXIT_FAILURE);
  }
  while(1){
    printf("En espera de SigKill (PID = %d)\n", getpid());
    sleep(9999);
  }
}
