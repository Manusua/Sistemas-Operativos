/*
Fichero: p1_exercise_9.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
     Manuel Cintado Puerta: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 20/2/2019
Descripción: Ejercicio 9: Creación de dos procesos hijos y generación de un número aleatorio
que servirá de comunicación con el padre, lectura
*/

/* Librerías utilizadas*/

 #include <sys/types.h>
 #include <sys/wait.h>

 #include <time.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <unistd.h>

int main(int argc, char *argv[]) {
  int fd[2], fd1[2];
  int aux, aux2;
  pid_t pid;
  int auxi;
  aux = pipe(fd);
  if(aux == -1) {
  	perror("Error creando la tuberia\n");
  	exit(EXIT_FAILURE);
  }
   if((pid = fork()) < 0) {
  	perror("fork");
  	exit(EXIT_FAILURE);
   }

   if(pid == 0){
      /* Inicializa el generador con una semilla cualquiera, OJO! este metodo solo
       se llama una vez */
      srand(time(NULL));
      /* Devuelve un numero aleatorio en 0 y MAX_RAND(un número alto que varia
       segun el sistema) */
      int r = rand();
      printf("HIJO 1, Número aleatorio: %d\n", r);
      close(fd[0]);
      write(fd[1], &r, sizeof(r));
      exit(EXIT_SUCCESS);
   }
   wait(NULL);

  if(pid>0) {
    /*En caso de que sea un hijo*/
    close(fd[1]);
    int ran = read(fd[0], &auxi, sizeof(auxi) );
    if(ran < 0)
      exit(EXIT_FAILURE);
    aux2 = pipe(fd1);
    if(aux2 == -1) {
    	perror("Error creando la tuberia\n");
    	exit(EXIT_FAILURE);
    }
    if((pid = fork()) == -1) {
   	  perror("fork");
   	  exit(EXIT_FAILURE);
    }
    else{
      /*En caso de que no haya habido error*/
      close(fd1[1]);
      write(fd1[0], &ran, sizeof(ran));
    if(pid == 0){
      /*Si es el hijo el proceso en el que estamos*/
      close(fd1[1]);
      int rand = read(fd1[0], &auxi, sizeof(auxi));

      if(rand < 0)
        exit(EXIT_FAILURE);

      printf("HIJO 2, Número aleatorio: %d\n", auxi);
      exit(EXIT_SUCCESS);
    }}
  }
  wait(NULL);
  exit(EXIT_SUCCESS);
}
