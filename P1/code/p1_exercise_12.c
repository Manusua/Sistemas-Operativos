/*
Fichero: p1_exercise_12.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado Puerta: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 21/2/2019
Descripción: Ejercicio 12: Creación de un programa que cree N hilos para realizar una operación
y además el padre debe esperar a que todos terminen e imprimir los resultados por pantalla
*/

/* Librerías utilizadas*/

#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_HILOS 5

/* Estructura: Hilo
x : número hilo que coincide con el elemento al que hay que elevar a 2
sol : resultado de la operación
*/

typedef struct _hilo{
    int x;
    long sol;
}hilo;

/* Función exponencial:

parámetros de entrada: puntero a void (hilo)
parámetros de salida: no tiene

actualiza la información del hilo tras la operación.
*/

void *exp (void *hilo){
  long sol = 1;
  int i;

  for(i = 0; i< ((hilo*)hilo)->x; ++i){
    sol *= 2;
  }
  ((hilo*)hilo)->sol = sol;
}


int main(int argc, char *argv[]){
  hilo hilos[NUM_HILOS];
  int i;
  pthread_t vec_hilos[NUM_HILOS];

  for(i = 1; i <= NUM_HILOS; ++i){
    hilos[i-1].x = i;
    pthread_create(&vec_hilos[i-1], NULL, exp, &hilos[i-1]);
  }

  for(i = 0; i < NUM_HILOS; ++i){
    pthread_join(vec_hilos[i], NULL);
    printf("Resultado del hilo número %d: %ld\n", i+1, hilos[i].sol);
  }
  exit(EXIT_SUCCESS);
}
