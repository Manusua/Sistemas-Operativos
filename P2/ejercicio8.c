#include <stdio.h>
#include <stdlib.h>
#include <semaphore>
#inlcude <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define SEM "/example_sem"

void imprimir_semaforo(sem_t *sem){
  int sval;

  if(sem_getvalue(sem, &sval) == -1){
    perror("sem_getvalue");
    sem_unlink(SEM);
    exit(EXIT_FAILURE);
  }

  printf("Valor del semaforo: %d\n", sval);
  fflush(stdout);
}

Lectura(){
  Down(sem_lectura);
  lectores++;
  if(lectores == 1)
    Down(sem_escritura);
  Up(sem_lectura);

  Leer();
  Down(sem_lectura);
  lectores--;
  if(lectores == 0)
    Up(sem_escritura);
  Up(sem_lectura);
}

Escritura(){
  Down(sem_escritura);

  Escribir();

  Up(sem_escritura);
}

int main(void){
  sem_t *sem = NULL;
  pid_t pid;

  if((sem = sem_open(SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  imprimir_semaforo(sem);
  sem_post(sem);
  imprimir_semaforo(sem);
  sem_post(sem);
  imprimir_semaforo(sem);
  sem_wait(sem);
  imprimir_semaforo(sem);

  pid = fork();
  if(pid < 0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if(pid == 0){
    sem_wait(sem);
    printf("Zona protegida (hijo)\n");
    sleep(5);
    printf("Fin zona protegida\n");
    sem_post(sem);

    sem_close(sem);
    exit(EXIT_SUCCES);
  }
  else{
    sem_wait(sem);
    printf("Zona protegida (hijo)\n");
    sleep(5);
    printf("Fin zona protegida\n");
    sem_post(sem);

    sem_close(sem);
    sem_unlink(SEM);
    wait(NULL);
    exit(EXIT_SUCCES);
  }
}
