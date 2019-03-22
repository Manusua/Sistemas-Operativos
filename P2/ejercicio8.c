#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define N_READ 1
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
  /*Mandamos la señal de SIGTERM a todos los hijos*/

  sem_unlink(SEM_LECTURA);
  sem_unlink(SEM_ESCRITURA);
  sem_unlink(LECTORES);

  if(kill(0, SIGTERM) < 0){
    perror("kill");
    exit(EXIT_FAILURE);
  };


  while(wait(NULL)>0);

  return;
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
  struct sigaction act;

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

    sigemptyset(&(act.sa_mask));
    act.sa_handler = manejador_SIGTERM;
    act.sa_flags = 0;
    if(sigaction(SIGTERM, &act, NULL) < 0){
      perror("Sigaction");
      exit(EXIT_FAILURE);
    }

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

      leer();

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
    sigemptyset(&(act.sa_mask));
    act.sa_handler = manejador_SIGINT;
    act.sa_flags = 0;
    if(sigaction(SIGINT, &act, NULL) < 0){
      perror("Sigaction");
      exit(EXIT_FAILURE);
    }

    while(1){
      if(sem_wait(sem_escritura) == -1){
        perror("sem_wait");
        exit(EXIT_FAILURE);
      }

      escribir();

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
