/*
Fichero: queue.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 10/04/2019
Ficher que implemena las funciones de la cola circular
*/

/* Librerías utilizadas*/
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

/*Consideramos la creacion de un array estático en vez de uno dinamico pues tenemos
definido previamente le numero maximo del mismo (10)*/
struct _Queue{
  float *first;
  float *last;
  char cola[BUFFER_SIZE];
}

Queue* init(){
  Queue* queue;
  q = (Queue*)malloc(sizeof(Queue));
  if(q == NULL){
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  queue->first = queue->cola;
  queue->last = queue->cola;;
  return queue;
}

void destroy(Queue* queue){
  free(queue);
}

void insert(Queue* queue, char letra){
  char aux;
  if()
}

int is_full(Queue* queue){
  if()
}
