#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>


#define SEM "/sem"
#define N_PROC 5

sem_t *sem = NULL;

void manejador_SIGTERM(int sig) {
  sem_close(sem_fichero);
  exit(EXIT_SUCCESS);
}

/*Manejador para que no haya que borrar manualmente el semaforo de dev/shm cuando hacermos Ctrl+C*/
void manejador_SIGINT(int sig) {
  sem_close(sem);
  sem_unlink(sem);
  exit(EXIT_SUCCESS);
}

int main(){
  pid_t pid;
  int aux, aux2, i, fin, num_lect[N_PROC];
  FILE *arch;
  sigset_t set1, setaux;
  struct sigaction act;

  if ((sem_fichero = sem_open(SEM, O_CREAT, 0)) == SEM_FAILED) {
      perror("sem_open");
      exit(EXIT_FAILURE);
  }

  if (sigaction(SIGTERM, &act, NULL) < 0) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  sigaddset(&set1, SIGTERM);

  for(i = 0;i< N_PROC; ++i){
    pid = fork(){
      if(pid < 0){
        perror("fork");
        exit(EXIT_FAILURE);
      }
      else if(pid == 0){
        while(1){
          /*De esta manera controlamos que solo un proceso escriba a la vez*/
          if(sem_getvalue(sem, &aux) == 0 && aux == 0){
            sem_post(sem);
            if (sigprocmask(SIG_BLOCK, &set1, &setaux) < 0) {
              perror("sigprocmask");
              exit(EXIT_FAILURE);
            }

            arch = fopen("file.txt", "a");
            fprintf(arch, "%d\n", i);
            fclose(arch);
            sem_wait(sem);

            if (sigprocmask(SIG_UNBLOCK, &set1, &setaux) < 0) {
                perror("sigprocmask");
                exit(EXIT_FAILURE);
            }
            srand(time(NULL)*(i+1));
            usleep(rand()%100000);
          }
        }
      }
    }
  }


  if (sigprocmask(SIG_BLOCK, &set, &oset) < 0) {
    perror("sigprocmask");
    exit(EXIT_FAILURE);
  }

  sleep(1);
  for(i = 0; i < N_PROC; ++i)
    num_lect[i] = 0;

  fin = -1;
  aux2 = 0;

  while(1){
    /*cuando el padre esta comprobando, ninguno puede escribir*/
    sem_post(sem);
    arch = fopen("file.txt", "a");
    while( aux2 != fin && fscanf(arch, %d, & aux2) != EOF ){
      num_lect[aux2]++;
      if(num_lect[aux2] > 20){
        fin = aux2;
      }
    }

    if(fin != -1){
      printf("Proceso ganador: %d con %d escrituras\n", aux2, num_lect[aux2]);

      kill(0, SIGTERM);

      while(wait(NULL) > 0);

      /*No Reiniciamos el archivo pues es la ultima(para comprobar que ha hecho las cosas bien)*/
      fclose(arch);
      sem_close(sem);
      sem_unlink(SEM);
      return EXIT_SUCCESS;
    }

    /*Reiniciamos el archivo*/
    fclose(arch);
    arch = fopen("file.txt", "w");
    fclose(arch);
    sem_wait(sem);
    sleep(1);
  }
  return EXIT_SUCCESS;
}
