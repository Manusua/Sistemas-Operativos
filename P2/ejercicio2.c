/*
Fichero: ejercicio2.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 29/03/2019
Descripción: cada hijo imprime un mensaje indicando su pid y comprueba las señales recibidas/enviadas
*/

/* Librerías utilizadas*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define N 4

int main(){
  int i;
  pid_t pid;
  for(i = 0; i < N; ++i){
    pid = fork();
    if(pid < 0){
      printf("Error al mplear e fork\n" );
      exit(EXIT_FAILURE);
    }
    else if( pid == 0){
      printf("Soy el proceso hijo %ld\n", (long) getpid());
      sleep(30);
      printf("Soy el proceso hijo %ld y me toca terminar.\n", (long) getpid());
      exit(EXIT_SUCCESS);
    }
    else{
      sleep(5);
      kill(pid, SIGTERM);
    }
  }
  exit(EXIT_SUCCESS);
}
