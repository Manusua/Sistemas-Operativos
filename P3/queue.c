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


Cola* init(){
  Cola* queue;
  queue = (Cola*)malloc(sizeof(Cola));
  if(!queue){
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  queue->first = 0;
  queue->last = 0;
  return queue;
}

void destroy(Cola* queue){
  free(queue);
}

int size(Cola *queue){
    if(!queue)
        return -1;

    if(is_empty(queue) == 0)
        return 0;

    if(queue->first < queue->last)
        return (queue->last - queue->first);

    else
        return (BUFFER_SIZE - queue->first + queue->last);
}

int insert(Cola* queue, char letra){
  if(!queue)
    return -1;
  else if(is_full(queue) == 0){
    return -1;
  }
  queue->cola[queue->last] = letra;
  if(queue->last == BUFFER_SIZE - 1){
    queue->last = 0;
  }
  else
    queue->last++;

  return 0;
}

int is_full(Cola* queue){
  char aux;
  if(!queue){
    return 0;
  }
  else if(queue->last ==  BUFFER_SIZE - 1){
    aux = 0;
  }
  else{
    aux = queue->last + 1;
  }

  if(aux == queue->first){
    printf("La cola está llena\n");
    return 0;
  }
  return -1;
}

char delete(Cola* queue){
  char aux;
  if(!queue)
    return -1;
  else if(is_empty(queue) == 0)
    return -1;

  aux = queue->cola[queue->first];

  if(queue->first == BUFFER_SIZE - 1){
    queue->first = 0;
  }
  else
    queue->first++;

  return aux;
}

int is_empty(Cola* queue){
  if(!queue){
    return -1;
  }
  if(queue->last == queue->first){
    printf("La cola está vacía\n");
    return 0;
  }

  return -1;
}
