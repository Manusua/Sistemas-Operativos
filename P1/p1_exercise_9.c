/*
 * Ejemplo de codigo que genera un numero aleatorio y lo muestra por pantalla
 */
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
    printf("HIJO 1, Número aleatorio: %d\n", r);
    close(fd[0]);
    write(fd[1], &r, sizeof(r));
    //TODO no se si deberia eliminar aqui ya el proceso hijo
    exit(EXIT_SUCCESS);
   }


//ESTO IGUAL SOBRA
wait(NULL);


  if(pid>0) {
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
    close(fd[1]);
    write(fd[0], &ran, sizeof(ran));
    if(pid == 0){
      close(fd[1]);
      int rand = read(fd[0], &auxi, sizeof(auxi));
      if(rand < 0)
        exit(EXIT_FAILURE);
      printf("HIJO 2, Número aleatorio: %d\n", auxi);
      exit(EXIT_SUCCESS);
    }}
  }
//TODO falta cerrar bein los hijos
  wait(NULL);
  exit(EXIT_SUCCESS);
}
