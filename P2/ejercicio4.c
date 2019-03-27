#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>


#define N_PROC 5

void manejador_SIGTERM(int sig){
  printf("Terminacion del proceso %ld a peticion del usuario\n" , getpid());
  fflush(stdout);
  exit(EXIT_SUCCESS);
}

void manejador_SIGUSR1(int sig){
  printf("PID: %ld ha recibido SIGUSR1\n", getpid());
  exit(EXIT_SUCCESS);
}

void manejador_SIGUSR1b(int sig){
  printf("PID(Gestor): %ld ha recibido SIGUSR1\n", getpid());
  while(wait(NULL)>= 0);
  exit(EXIT_SUCCESS);
}

void manejador_SIGUSR2(int sig){
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
      pid = fork();
      if(pid < 0){
        perror("fork");
        exit(EXIT_FAILURE);
      }
    for(i = 1; i < N_PROC; ++i){
      if(pid > 0){
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
          printf("El proceso %ld esta listo\n", getpid());
          if(kill(getppid(), SIGUSR2) < 0){
              perror("kill");
              exit(EXIT_FAILURE);
            }
        }
        else{
          /*No se si tengo que añadir un manejador espcifico para el gestor con SIGUSR2*/
          if(pause() < 0){
            perror("pause");
            exit(EXIT_FAILURE);
          }
        }
      }
    }
    if(kill(getppid(), SIGUSR2) < 0){
        perror("kill");
        exit(EXIT_FAILURE);
      }
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
  }
/*

  sigemptyset(&(act.sa_mask));
  act.sa_flags = 0;

  act.sa_handler = manejador_SIGTERM;
  if(sigaction(SIGTERM, &act, NULL) < 0){
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  act.sa_handler = manejador_SIGUSR1;
  if(sigaction(SIGUSR1, &act, NULL) < 0){
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  printf("PID: %d. Se esperan las señales\n",getpid());

  while(1)
    pause();*/
}
