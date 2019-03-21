#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define N_READ 2
#define SECS 1

#define SEM_LECTURA "/sem_lectura"
#define SEM_ESCRITURA "/sem_escritura"
#define LECTORES "/lectores"

void leer(){
  printf("R-INI %ld\n", getpid());
  sleep(1);
  printf("R-FIN %ld\n", getpid());
}

void escribir(){
  printf("W-INI %ld\n", getpid());
  sleep(1);
  printf("W-FIN %ld\n", getpid());
}


void manejador_SIGINT(int sig){
  printf("eee\n");
  /*Mandamos la señal de SIGTERM a todos los hijos*/
  if(kill(-1, SIGTERM) < 0){
    perror("kill");
    exit(EXIT_FAILURE);
  };


  while(wait(NULL)>0);

  sem_unlink(SEM_LECTURA);
  sem_unlink(SEM_ESCRITURA);
  sem_unlink(LECTORES);

  return;
}




void manejador_SIGTERM(int sig){
  printf("Hijo %ld finalizado\n", getpid());
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
  if((lectores = sem_open(LECTORES, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  /*Supongo sin problema que minimo creara un hijo, si no el programa no tiene sentido*/
  pid = fork();
  for(i = 1; i < N_READ && pid > 0; ++i){
    pid = fork();
    printf("hijo creado\n");
  }

  if(pid < 0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if(pid == 0){
    printf("HIJO\n" );

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

      sleep(1);
    }
    exit(EXIT_SUCCESS);
  }
  else{
    printf("PADRE\n" );
    /*Establezco la señal de interrupcion(SIGINT) y su comportamiento*/
    sigemptyset(&(act.sa_mask));
    act.sa_handler = manejador_SIGINT;
    act.sa_flags = 0;
    if(sigaction(SIGINT, &act, NULL) < 0){
      perror("Sigaction");
      exit(EXIT_FAILURE);
    }

    while(1){
    printf("BUCLE\n" );
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
    /*sem_wait(sem);
    printf("Zona protegida (hijo)\n");
    sleep(5);
    printf("Fin zona protegida\n");
    sem_post(sem);

    sem_close(sem);
    sem_unlink(SEM);
    wait(NULL);
    exit(EXIT_SUCCESS);*/
    sem_close(sem_lectura);
    sem_unlink("/sem_lectura");
    sem_close(sem_escritura);
    sem_unlink("/sem_escritura");
    sem_close(lectores);
    sem_unlink("/lectores");
    exit(EXIT_SUCCESS);
  }

}
