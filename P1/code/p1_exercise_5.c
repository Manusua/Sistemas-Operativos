/*
Fichero: p1_exercise_5.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es, 
		 Manuel Cintado Puerta: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 10/2/2019
Descripción: Apartado b) del ejercicio 5: Modificar el código para eliminar el memory leak
*/

/* wait and return process info */

/* Librerías utilizadas */

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int  main ( void )
{
	pid_t pid;
	char * sentence = (char *)malloc(5 * sizeof (char));
	pid = fork();
	if (pid <  0  )
	{
		printf("Error al emplear fork\n");
		exit (EXIT_FAILURE);
	}
	else  if(pid ==  0)
	{
		strcpy(sentence, "hola");
		free(sentence);		/* Libera memoria en el hijo */
		exit(EXIT_SUCCESS);
	}
	else
	{
		wait(NULL);
		printf("Padre: %s\n", sentence);
		free(sentence);		/* Libera memoria en el padre */
		exit(EXIT_SUCCESS);
	}
}
