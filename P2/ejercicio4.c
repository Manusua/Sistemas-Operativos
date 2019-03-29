/*
Fichero: ejercicio4.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 29/03/2019
Descripción:Ejercicio en el que simulamos el funcionamiento de una carrera de procesos
*/

/* Librerías utilizadas*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define N_PROC 5
/**
 * Manejador de la funcion SIGUSR1, definida para los participantes de la carrera
 */
void manejador_SIGUSR1(int sig){
  printf("PID: %ld ha recibido SIGUSR1\n", (long)getpid());
  exit(EXIT_SUCCESS);
}

/**
 * Manejador de la señar SIGUSR1 definida unicamente para el proceso gestor
 */
void manejador_SIGUSR1b(int sig){
  printf("PID(Gestor): %ld ha recibido SIGUSR1\n", (long)getpid());
  while(wait(NULL)>= 0);
  exit(EXIT_SUCCESS);
}

/**
 * Manejador de la señal USR2 definida unicamente para el proceso gestor y los
 * participantes de la carrera.
 */
void manejador_USR2b(int sig){
    printf("Señal SIGUSR2 recibida en proceso con pid = %ld\n", (long)getpid());
    fflush(stdout);
}

/**
 * Manejador de la señal USR2 defnida unicamente para el proceso padre.
 */
void manejador_SIGUSR2(int sig){
  printf("Aviso de que la competicion va a comenzar\n");
  if(kill(0, SIGUSR1) < 0){
      perror("kill");
      exit(EXIT_FAILURE);
    }
}

int main(void){
  struct sigaction act;
  pid_t pid;
  int i;

  pid = fork();
  if(pid < 0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else if(pid == 0){
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;

    act.sa_handler = manejador_SIGUSR1b;
    if(sigaction(SIGUSR1, &act, NULL) < 0){
      perror("sigaction");
      exit(EXIT_FAILURE);
    }

    act.sa_handler = manejador_USR2b;
    if (sigaction(SIGUSR2, &act, NULL) < 0) { //Establecemos la captura de la señal SIGUSR2
      perror("sigaction");
      exit(EXIT_FAILURE);
    }

    for(i = 0; i < N_PROC; ++i){
      pid = fork();
      if(pid < 0){
        perror("fork");
        exit(EXIT_FAILURE);
      }
      else if(pid == 0){
          sigemptyset(&(act.sa_mask));
          act.sa_flags = 0;
          act.sa_handler = manejador_SIGUSR1;

          if(sigaction(SIGUSR1, &act, NULL) < 0){
            perror("sigaction");
            exit(EXIT_FAILURE);
          }
          printf("Soy el proceso %ld, estoy listo\n", (long)getpid());

          kill(getppid(), SIGUSR2);
          pause(); //Esperamos a recibir la señal de que la competicion ha comenzado
        }
     else {
       pause(); //Esperamos a que el hijo que acabamos de crear esté listo

      }
    }
      printf("Aviso desde el proceso gestor de que todos los participantes están listos\n");
      if(kill(getppid(), SIGUSR2) < 0){
        perror("kill");
        exit(EXIT_FAILURE);
      }
      pause(); //Esperamos a recibir la señal de que la competicion ha comenzado
    }
  else{
  /*prcoeso padre tocho*/
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    act.sa_handler = manejador_SIGUSR2;
    if(sigaction(SIGUSR2, &act, NULL) < 0){
      perror("sigaction");
      exit(EXIT_FAILURE);
    }
    act.sa_handler = SIG_IGN; //Ignoramos la señal USR1
    if (sigaction(SIGUSR1, &act, NULL) < 0) {
      perror("sigaction");
      exit(EXIT_FAILURE);
    }
    pause(); //Esperamos a recibir la señal del proceso gestor
    wait(NULL); //Esperamos a que el hijo gestor termine
    return EXIT_SUCCESS;
  }
}
