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
struct _Cola{
  char *last;
  char *first;
  char cola[BUFFER_SIZE];
};

Cola* init(){
  Cola* queue;
  queue = (Cola*)malloc(sizeof(Cola));
  if(!queue){
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  queue->first = queue->cola;
  queue->last = queue->cola;;
  return queue;
}

void destroy(Cola* queue){
  free(queue);
}

int queue_size(Cola *queue){
    int aux;

    if(!queue)
        return -1;

    if(is_empty(queue))
        return 0;

    if(queue->last > queue->first)
        aux = queue->last - queue->first;

    else if(queue->first > queue->last) {
        aux = queue->first - queue->last;
        aux = BUFFER_SIZE - aux;
    }

    return aux;
}

bool insert(Cola* queue, char letra){
  if(!queue)
    return FALSE;
  else if(is_full(queue))
    return FALSE;

  *queue->last = letra;

  if(queue->last == queue->cola + BUFFER_SIZE + 1){
    queue->last = queue->cola;
  }
  else
    queue->last++;

  return TRUE;
}

bool is_full(Cola* queue){
  char aux;
  if(!queue){
    return TRUE;
  }
  /*TODO esto es asi ?¿*/
  else if(queue->last == queue->cola + BUFFER_SIZE - 1){
    aux = *(queue->cola);
  }
  else{
    aux = *(queue->last + 1);
  }

  if(aux == *(queue->first)){
    return TRUE;
  }

  return FALSE;
}

char delete(Cola* queue){
  char aux;
  if(!queue)
    return FALSE;
  else if(is_empty(queue))
    return FALSE;

  aux = *(queue->first);

  if(queue->first == queue->cola + BUFFER_SIZE - 1){
    queue->first = queue->cola;
  }
  else
    queue->first++;

  return aux;
}

bool is_empty(Cola* queue){
  if(!queue){
    return FALSE;
  }

  if(queue->last == queue->first){
    return TRUE;
  }

  return FALSE;
}
