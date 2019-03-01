/*
Fichero: p1_exercise_3.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es, 
		 Manuel Cintado Puerta: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 8/2/2019
Descripción: Apartado b) del ejercicio 3: modificación del fichero correspondiente a 
este ejercicio para que el proceso hijo imprima su pid y el de su padre en vez de i
*/

/*Librerías utilizadas*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROC 3

int main(void)
{
	pid_t pid;
	int i;
	for(i = 0; i < NUM_PROC; i++)
	{
		pid = fork();
		if(pid <  0)
		{
			printf("Error al emplear fork\n");
			exit(EXIT_FAILURE);
		}
		else if(pid ==  0)
		{
			printf("PPID:  %ld, PID: %ld\n", (long)getppid(), (long)getpid());  /* Aquí se ha realizado la modificación */
			exit(EXIT_SUCCESS);
		}
		else if(pid >  0)
		{
			printf("PADRE %ld\n", (long)getppid());
		}
	}
	wait(NULL);
	exit(EXIT_SUCCESS);
}
