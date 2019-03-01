/*
Fichero: p1_exercise_11.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es, 
     Manuel Cintado Puerta: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 20/2/2019
Descripción: Código del ejercicio 11 NO ENTREGABLE
*/

/* Librerías utilizadas*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void *slowprintf (void *arg) {
   char *msg;
   int i;
   msg = (char *)arg;

   for ( i = 0 ; i < strlen(msg) ; i++ ) {
    printf(" %c", msg[i]);
    fflush(stdout);
    usleep (1000000) ;
   }
   pthread_exit(NULL);
}

int main(int argc , char *argv[]) {
   pthread_t h1;
   pthread_t h2;
   char *hola = "Hola ";
   char *mundo = "Mundo";

   pthread_create(&h1, NULL , slowprintf , (void *)hola);
   pthread_create(&h2, NULL , slowprintf , (void *)mundo);

   pthread_join(h1,NULL);
   pthread_join(h2,NULL);

   printf("El programa %s termino correctamente \n", argv[0]);
   exit(EXIT_SUCCESS);
}
