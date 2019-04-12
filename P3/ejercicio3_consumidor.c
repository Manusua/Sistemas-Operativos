/*
Fichero: ejercicio3_consumidor.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 05/04/2019
Descripción: parte del consumidor en el problema consumidor-productor
*/

/* Librerías utilizadas*/
#ifndef EJE_CONS
#define EJE_CONS
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include "queue.h"

/*TODO ver como podemos meterlo en otro fichero*/
#define ESPACIOS "/sem_espacios"
#define LETRAS "/sem_letras"
#define GENERAL "/sem_general"
#define SHM_NAME "/mem_info"
#define MAXIM 10

/*Para que cuando salgamos de una ejeccion con Ctrl+C se elimine le semaforo*/
void manejador_SIGINT(int sig){
  printf("\nrecibida SIGINT\n");
  exit(EXIT_FAILURE);
}

int main(){
  sem_t *sem_letras, *sem_espacios, *sem_general;
  int fd_shm;
  Cola *cad;
  char auxic;
  struct sigaction act;

  sigemptyset(&(act.sa_mask));
  act.sa_flags = 0;
  act.sa_handler = manejador_SIGINT;

  if (sigaction(SIGINT, &act, NULL) < 0) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  if((sem_letras = sem_open(LETRAS, S_IWUSR | S_IRUSR, 0)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  if((sem_espacios = sem_open(ESPACIOS, S_IWUSR | S_IRUSR, MAXIM)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  if((sem_general = sem_open(LETRAS, S_IWUSR | S_IRUSR, 1)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  fd_shm = shm_open(SHM_NAME, O_RDWR, S_IRUSR | S_IWUSR);

  if(fd_shm == -1) {
  	fprintf (stderr, "Error creando el segmento de memoria compartida \n");
  	return EXIT_FAILURE;
  }

  cad = (Cola *)mmap(NULL, sizeof(Cola), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);

  if(cad == MAP_FAILED){
    fprintf (stderr, "Error mapeando el egmento de memoria compartida \n");
    return EXIT_FAILURE;
  }

  while(1){
    sem_wait(sem_letras);
    sem_wait(sem_general);

    if((auxic = delete(cad)) == -1){
      perror("delete");
      munmap(cad, sizeof(Cola));
      exit(EXIT_FAILURE);
    }
    else if(auxic != '\0'){
      printf("%c", auxic);
      sem_post(sem_general);
      sem_post(sem_espacios);
    }
    else{
      printf("\nEncontrado el final del fichero\n");
      sem_close(sem_general);
      sem_close(sem_letras);
      sem_close(sem_espacios);

      exit(EXIT_SUCCESS);
    }
  }
}

#endif
