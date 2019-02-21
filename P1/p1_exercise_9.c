/*
 * Ejemplo de codigo que genera un numero aleatorio y lo muestra por pantalla
 */
 #include <sys/types.h>
 #include <time.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <unistd.h>

int main(int argc, char *argv[]) {
  int fd[2], fd1[2];
  int aux, aux2;
  pid_t pid;
  char readbuffer[80];
  aux = pipe(fd);
  if(aux == -1) {
  	perror("Error creando la tuberia\n");
  	exit(EXIT_FAILURE);
  }
   if((pid = fork()) == -1) {
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
    printf("HIJO 1, Número aleatorio:%d\n", r);
    close(fd[1]);
    write(fd[0], &r,sizeof(r));
    //TODO no se si deberia eliminar aqui ya el proceso hijo
   }




  if(pid>0) {
    close(fd[0]);
    int ran = read(fd[0], readbuffer, sizeof(readbuffer) );
    aux2 = pipe(fd1);
    if(aux2 == -1) {
    	perror("Error creando la tuberia\n");
    	exit(EXIT_FAILURE);
    }
    if((pid = fork()) == -1) {
   	perror("fork");
   	exit(EXIT_FAILURE);
    }
    else
    close(fd[1]);
    write(fd[0], &ran, sizeof(ran));
    if(pid == 0){
      close(fd[0]);
      int rand = read(fd[0], readbuffer, sizeof(readbuffer));
      printf("HIJO 2, Número aleatorio: %d\n", rand);
    }
  }
//TODO falta cerrar bein los hijos
  exit(EXIT_SUCCESS);
}
