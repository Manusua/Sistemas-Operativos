/*
Fichero: ejercicio3_consumidor.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 05/04/2019
Descripción: parte del consumidor en el problema consumidor-productor
*/

/* Librerías utilizadas*/
#ifndef EJE_PROD
#define EJE_PROD
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
#define NOMBRE_ARCHIVO "fich.txt"

/*Para que cuando salgamos de una ejeccion con Ctrl+C se elimine le semaforo*/
void manejador_SIGINT(int sig){
  printf("\nrecibida SIGINT\n");
  shm_unlink(SHM_NAME);
  sem_unlink(LETRAS);
  sem_unlink(ESPACIOS);
  sem_unlink(GENERAL);
  exit(EXIT_FAILURE);
}

int main(){
  sem_t *sem_letras, *sem_espacios, *sem_general;
  int fd_shm, error;
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

/*En primer lugar el productor crea la memoria compartida y los semaforos pertinentes*/

  if((sem_letras = sem_open(LETRAS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  if((sem_espacios = sem_open(ESPACIOS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, MAXIM)) == SEM_FAILED){
    perror("sem_open");
    sem_close(sem_letras);
    sem_unlink(LETRAS);
    exit(EXIT_FAILURE);
  }

  if((sem_general = sem_open(GENERAL, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED){
    perror("sem_open");
    sem_close(sem_letras);
    sem_unlink(LETRAS);
    sem_close(sem_espacios);
    sem_unlink(ESPACIOS);
    exit(EXIT_FAILURE);
  }

/*Creamos el segmento de memoria comaprtida*/
  fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

  if(fd_shm == -1) {
  	fprintf (stderr, "Error creando el segmento de memoria compartida \n");
    sem_close(sem_letras);
    sem_unlink(LETRAS);
    sem_close(sem_espacios);
    sem_unlink(ESPACIOS);
    sem_close(sem_general);
    sem_unlink(GENERAL);
  	return EXIT_FAILURE;
  }

  error = ftruncate(fd_shm, sizeof(Cola));

  if(error == -1) {
    fprintf (stderr, "Error redimensionando el segmento de memoria compartida \n");
    shm_unlink(SHM_NAME);
    sem_close(sem_letras);
    sem_unlink(LETRAS);
    sem_close(sem_espacios);
    sem_unlink(ESPACIOS);
    sem_close(sem_general);
    sem_unlink(GENERAL);
    return EXIT_FAILURE;
  }

  cad = (Cola *)mmap(NULL, sizeof(Cola), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);

  if(cad == MAP_FAILED){
    fprintf (stderr, "Error mapeando el egmento de memoria compartida \n");
    shm_unlink(SHM_NAME);
    sem_close(sem_letras);
    sem_unlink(LETRAS);
    sem_close(sem_espacios);
    sem_unlink(ESPACIOS);
    sem_close(sem_general);
    sem_unlink(GENERAL);
    return EXIT_FAILURE;
  }

  scanf("%c", &auxic);

  while(auxic != EOF){

    sem_wait(sem_espacios);
    sem_wait(sem_general);

    if(insert(cad, auxic) == -1){
      perror("insert");

      shm_unlink(SHM_NAME);
      munmap(cad, sizeof(Cola));
      sem_close(sem_letras);
      sem_unlink(LETRAS);
      sem_close(sem_espacios);
      sem_unlink(ESPACIOS);
      sem_close(sem_general);
      sem_unlink(GENERAL);

      return EXIT_FAILURE;
    }

    sem_post(sem_general);
    sem_post(sem_letras);

    scanf("%c", &auxic);
  }
  sem_wait(sem_espacios);
  sem_wait(sem_general);

  if(insert(cad, '\0') == -1){
    perror("insert");

    shm_unlink(SHM_NAME);
    munmap(cad, sizeof(Cola));
    sem_close(sem_letras);
    sem_unlink(LETRAS);
    sem_close(sem_espacios);
    sem_unlink(ESPACIOS);
    sem_close(sem_general);
    sem_unlink(GENERAL);

    return EXIT_FAILURE;
  }
  else{
    sem_post(sem_general);
    sem_post(sem_letras);

    shm_unlink(SHM_NAME);
    munmap(cad, sizeof(Cola));
    sem_close(sem_letras);
    sem_unlink(LETRAS);
    sem_close(sem_espacios);
    sem_unlink(ESPACIOS);
    sem_close(sem_general);
    sem_unlink(GENERAL);

    return EXIT_FAILURE;
  }
}

#endif
