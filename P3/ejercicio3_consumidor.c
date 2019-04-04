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

/*TODO ver como podemos meterlo en otro fichero*/
#define ESPACIOS "/sem_espacios"
#define LETRAS "/sem_letras"
#define SHM_NAME "/mem_info"
#define MAXIM 10
#define NOMBRE_ARCHIVO "fich_sal.txt"

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
  int fd_shm;
  cadenaCaraterers *cad;
  FILE*f;
  int i = 0, aux;

  if((sem_letras = sem_open(LETRAS, 0)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  if((sem_espacios = sem_open(ESPACIOS, 10)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }



  /*Tenemso qeu sacar fd_shm en algun lado, qeu lo haga el productor el un txt?¿
  se puede meter en un semaforo? creo que si */
  cad = (cadenaCaraterers *)mmap(NULL, sizeof(* cad), PROT_READ | PROT_WRITE,
   MAP_SHARED, fd_shm, 0);

  if(cad == MAP_FAILED){
    fprintf (stderr, "Error mapeando el egmento de memoria compartida \n");
    shm_unlink(SHM_NAME);
    sem_unlink(LETRAS);
    sem_unlink(ESPACIOS);
    return EXIT_FAILURE;
  }


  f = fopen(NOMBRE_ARCHIVO, "w");
  /*Hace falta un control de que se meten mas de 10 letras? Que pasa entonces?*/
  while(cad->cadena[i] != '\0'){
    if(sem_post(sem_espacios) == -1){
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
    /*Comprobamos cuantos letras quedan disponibles*/
    sem_getvalue(sem_letras, &aux);
    while(aux != 0){
      sleep(1);
      sem_getvalue(sem_letras, &aux);
    }

    if(sem_wait(sem_letras) == -1){
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
    fprintf(f, "%c\n", cad->cadena[i]);
    i++;
  }
  cad->cadena[i] = '\0';

  exit(EXIT_SUCCESS);
}
