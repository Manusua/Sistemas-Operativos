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
#define TRUE 1
#define FALSE 0

typedef struct _Cola Cola;

typedef int bool;

Cola* init();

void destroy(Cola* queue);

int queue_size(Cola *queue);

bool insert(Cola* queue, char letra);

bool is_full(Cola* queue);

char delete(Cola* queue);

bool is_empty(Cola* queue);

#endif
