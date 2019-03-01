/*
Fichero: p1_exercise_4.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es, 
		 Manuel Cintado Puerta: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 8/2/2019
Descripción: Apartado c) del ejercicio 4: El proceso padre genera un proceso hijo que 
generará otro hijo y así hasta llegar a NUM_PROC hijos
*/

/* Librerías utilizadas*/

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
			sleep(3);
			printf("PPID:  %d, PID: %d\n", (long)getppid(), (long)getpid());    /* El proceso padre espera a que finalice el hijo */
		}
		else if(pid >  0)
		{
			printf("PADRE %d\n", (long)getppid());
		}
	}
	wait(NULL);
	exit(EXIT_SUCCESS);
}
