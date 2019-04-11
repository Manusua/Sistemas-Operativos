/*
Fichero: ejercicio3_consumidor.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 05/04/2019
Descripción: parte del consumidor en el problema consumidor-productor
*/

/* Librerías utilizadas*/
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
#define SHM_NAME "/mem_info"
#define MAXIM 10
#define NOMBRE_ARCHIVO "fich.txt"

/*Para que cuando salgamos de una ejeccion con Ctrl+C se elimine le semaforo*/
void manejador_SIGINT(int sig){
  printf("recibida SIGINT\n");
  sem_unlink(LETRAS);
  sem_unlink(ESPACIOS);
  shm_unlink(SHM_NAME);
  exit(EXIT_FAILURE);
}

/*TODO Me falta implementar que sea una cola circular*/
typedef struct {
  char cadena[MAXIM];
} cadenaCaraterers;

int main(){
  sem_t *sem_letras, *sem_espacios;
  int fd_shm, error;
  cadenaCaraterers *cad;
  char auxic;
  FILE*f;
  int i = 0, aux;

  if((sem_letras = sem_open(LETRAS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  if((sem_espacios = sem_open(ESPACIOS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 10)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }


/*Creamos el segmento de memoria comaprtida*/
  fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

  if(fd_shm == -1) {
  	fprintf (stderr, "Error creando el segmento de memoria compartida \n");
    sem_unlink(LETRAS);
    sem_unlink(ESPACIOS);
  	return EXIT_FAILURE;
  }

  error = ftruncate(fd_shm, sizeof(cadenaCaraterers));

  if(error == -1) {
    fprintf (stderr, "Error redimensionando el segmento de memoria compartida \n");
    shm_unlink(SHM_NAME);
    sem_unlink(LETRAS);
    sem_unlink(ESPACIOS);
    return EXIT_FAILURE;
  }

  cad = (cadenaCaraterers *)mmap(NULL, sizeof(*cad), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);

  if(cad == MAP_FAILED){
    fprintf (stderr, "Error mapeando el egmento de memoria compartida \n");
    shm_unlink(SHM_NAME);
    sem_unlink(LETRAS);
    sem_unlink(ESPACIOS);
    return EXIT_FAILURE;
  }


  f = fopen(NOMBRE_ARCHIVO, "r");
  /*Hace falta un control de que se meten mas de 10 letras? Que pasa entonces?*/
  while((auxic = fgetc(f)) != EOF){
    if(sem_post(sem_letras) == -1){
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
    /*Comprobamos cuantos espacios quedan disponibles*/
    sem_getvalue(sem_espacios, &aux);
    while(aux != 0){
      sleep(1);
      sem_getvalue(sem_espacios, &aux);
    }

    if(sem_wait(sem_espacios) == -1){
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
    cad->cadena[i] = auxic;
    i++;
  }
  cad->cadena[i] = '\0';



  shm_unlink(SHM_NAME);
  sem_close(sem_letras);
  sem_unlink(LETRAS);
  sem_close(sem_espacios);
  sem_unlink(ESPACIOS);
  exit(EXIT_SUCCESS);
}
