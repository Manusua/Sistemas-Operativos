/*
Fichero: ejercicio8.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 29/03/2019
Descripción: simulaicion del algoritmo de escritura y lectura
*/

/* Librerías utilizadas*/
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define N_READ 10
#define SECS 0

#define SEM_LECTURA "/sem_lectura"
#define SEM_ESCRITURA "/sem_escritura"
#define LECTORES "/lectores"

void leer(){
  printf("R-INI %ld\n", (long)getpid());
  sleep(1);
  printf("R-FIN %ld\n", (long)getpid());
}

void escribir(){
  printf("W-INI %ld\n", (long)getpid());
  sleep(1);
  printf("W-FIN %ld\n", (long)getpid());
}


void manejador_SIGINT(int sig){

  sem_unlink(SEM_LECTURA);
  sem_unlink(SEM_ESCRITURA);
  sem_unlink(LECTORES);

    /*Mandamos la señal de SIGTERM a todos los hijos*/
  if(kill(0, SIGTERM) < 0){
    perror("kill");
    exit(EXIT_FAILURE);
  };
  /*Esperamos a que todos los hijos finalicen*/
  while(wait(NULL)>0);
  exit(EXIT_SUCCESS);
}

void manejador_SIGTERM(int sig){
  printf("Hijo %ld finalizado\n", (long)getpid());
  exit(EXIT_SUCCESS);
  return;
}


int main(void){
  sem_t *sem_lectura = NULL, *sem_escritura = NULL, *lectores = NULL;
  pid_t pid;
  int i, aux;
  sigset_t set1, setaux;
  struct sigaction act;


  sigemptyset(&(act.sa_mask));
  act.sa_handler = manejador_SIGTERM;
  act.sa_flags = 0;
  if(sigaction(SIGTERM, &act, NULL) < 0){
    perror("Sigaction");
    exit(EXIT_FAILURE);
  }

  if((sem_lectura = sem_open(SEM_LECTURA, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  if((sem_escritura = sem_open(SEM_ESCRITURA, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  if((lectores = sem_open(LECTORES, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  /*Supongo sin problema que minimo creara un hijo, si no el programa no tiene sentido*/
  pid = fork();
  for(i = 1; i < N_READ && pid > 0; ++i){
    pid = fork();
  }

  if(pid < 0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if(pid == 0){
    act.sa_handler = SIG_IGN;
    if (sigaction(SIGINT, &act, NULL) < 0) {
      perror("sigaction");
      exit(EXIT_FAILURE);
    }
    sigaddset(&set1, SIGTERM);

    while(1){
      if(sem_wait(sem_lectura) == -1){
        perror("sem_wait");
        exit(EXIT_FAILURE);
      }
      if(sem_post(lectores) == -1){
        perror("sem_post");
        exit(EXIT_FAILURE);
      }
      if(sem_getvalue(lectores, &aux) == 0 && aux == 1)
        if(sem_wait(sem_escritura) == -1){
          perror("sem_wait");
          exit(EXIT_FAILURE);
        }
      if(sem_post(sem_lectura) == -1){
        perror("sem_post");
        exit(EXIT_FAILURE);
      }

      if (sigprocmask(SIG_BLOCK, &set1, &setaux) < 0) { //Bloqueamos máscara.
        perror("sigprocmask");
        exit(EXIT_FAILURE);
      }

      leer();

      if (sigprocmask(SIG_UNBLOCK, &set1, &setaux) < 0) { //Desbloqueamos máscara.
        perror("sigprocmask");
        exit(EXIT_FAILURE);
      }

      if(sem_wait(sem_lectura) == -1){
        perror("sem_wait");
        exit(EXIT_FAILURE);
      }
      if(sem_wait(lectores) == -1){
        perror("sem_wait");
        exit(EXIT_FAILURE);
      }
      if(sem_getvalue(lectores, &aux) == 0 && aux == 0)
        if(sem_post(sem_escritura) == -1){
          perror("sem_post");
          exit(EXIT_FAILURE);
        }
      if(sem_post(sem_lectura) == -1){
        perror("sem_post");
        exit(EXIT_FAILURE);
      }

      sleep(SECS);
    }
    exit(EXIT_SUCCESS);
  }
  else{
    /*Establezco la señal de interrupcion(SIGINT) y su comportamiento*/
    act.sa_handler = manejador_SIGINT;
    if(sigaction(SIGINT, &act, NULL) < 0){
      perror("Sigaction");
      exit(EXIT_FAILURE);
    }

    act.sa_handler = SIG_IGN;
    if (sigaction(SIGTERM, &act, NULL) < 0) {
      perror("sigaction");
      exit(EXIT_FAILURE);
    }
    sigaddset(&set1, SIGINT);

    while(1){
      if(sem_wait(sem_escritura) == -1){
        perror("sem_wait");
        exit(EXIT_FAILURE);
      }
      if (sigprocmask(SIG_BLOCK, &set1, &setaux) < 0) { //Bloqueamos máscara.
        perror("sigprocmask");
        exit(EXIT_FAILURE);
      }
      escribir();

      if (sigprocmask(SIG_UNBLOCK, &set1, &setaux) < 0) { //Desbloqueamos máscara.
        perror("sigprocmask");
        exit(EXIT_FAILURE);
      }

      if(sem_post(sem_escritura) == -1){
        perror("sem_post");
        exit(EXIT_FAILURE);
      }

      sleep(SECS);
    }
    sem_close(sem_lectura);
    sem_unlink(SEM_LECTURA);
    sem_close(sem_escritura);
    sem_unlink(SEM_ESCRITURA);
    sem_close(lectores);
    sem_unlink(LECTORES);
    exit(EXIT_SUCCESS);
  }

}
