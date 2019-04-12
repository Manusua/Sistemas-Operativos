/*
Fichero: queue.h
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 10/04/2019
Ficher que implemena las funciones de la cola circular
*/

/* Librerías utilizadas*/
#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define BUFFER_SIZE 10

typedef struct _Cola{
  /*last nos va a indicar donde introducir el nuevo elemnto (es decir si hay un elemento
sera el 0)*/
  int last;
  int first;
  char cola[BUFFER_SIZE];
} Cola;


Cola* init();

void destroy(Cola* queue);

int size(Cola *queue);

int insert(Cola* queue, char letra);

int is_full(Cola* queue);

char delete(Cola* queue);

int is_empty(Cola* queue);

#endif
