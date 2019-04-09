/*
Fichero: ejercicio2_solved.c
Autores: Manuel Suárez Román: manuel.suarezr@estudiante.uam.es,
		 Manuel Cintado: manuel.cintado@estudiante.uam.es
Grupo: 2202
Fecha: 05/04/2019
Descripción: ejericcio que crea hijos que modifican la memoria compartida arreglando los
problemas de escritura
*/

/* Librerías utilizadas*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

#define SHM_NAME "/clientInfo"
#define SEMA "/sem_escritura"
#define NAME_MAX 50

typedef struct {
  int previous_id;
  int id;
  char name[NAME_MAX];
} ClientInfo;
/*Ponemos clienteinformacion como variable flobal para pode rmostrar los datos en el manejador*/
ClientInfo *clienteinformacion;
/*Imprimimos la informacion al recibir la señal*/
void manejador_SIGUSR1(int sig){
  printf("recibida SIGUSR1\n");
  printf("\tID: %d\n\tPrevious_id: %d\n\tName: %s\n", clienteinformacion->id, clienteinformacion->previous_id, clienteinformacion->name);

}
/*Para que cuando salgamos de una ejeccion con Ctrl+C se elimine le semaforo*/
void manejador_SIGINT(int sig){
  printf("recibida SIGINT\n");
  munmap(clienteinformacion, sizeof(*clienteinformacion));
  shm_unlink(SHM_NAME);
  sem_unlink(SEMA);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
  int n, i;
  pid_t pid;
	int fd_shm;
	int error;
  char nombreaux[NAME_MAX];
  struct sigaction act;
  sem_t *sem_lect = NULL;

/*Controlamos que recibamos el numeor correcto de paramteros*/
  if(argc != 2) {
    printf("Introduzca el numero de procesos hijo\n");
    exit(EXIT_FAILURE);
  }

  n = atoi(argv[1]);

  /*Creamos el semáforo de lectura*/
  if((sem_lect = sem_open(SEMA, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

/*manejador de Ctrl +C*/
  sigemptyset(&(act.sa_mask));
  act.sa_handler = manejador_SIGINT;
  act.sa_flags = 0;
  if(sigaction(SIGINT, &act, NULL) < 0){
    perror("Sigaction");
    exit(EXIT_FAILURE);
  }
/*Creamos el segmento de memoria comaprtida*/
  fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

  if(fd_shm == -1) {
  	fprintf (stderr, "Error creando el segmento de memoria compartida \n");
  	return EXIT_FAILURE;
  }

  error = ftruncate(fd_shm, sizeof(ClientInfo));

  if(error == -1) {
    fprintf (stderr, "Error redimensionando el segmento de memoria compartida \n");
    shm_unlink(SHM_NAME);
    return EXIT_FAILURE;
  }

  clienteinformacion = (ClientInfo *)mmap(NULL, sizeof(*clienteinformacion), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);

	if(clienteinformacion == MAP_FAILED) {
		fprintf (stderr, "Error mapeando el egmento de memoria compartida \n");
		shm_unlink(SHM_NAME);
		return EXIT_FAILURE;
	}

  /*Inicializamos id previous_id*/
  clienteinformacion->id = 0;
  clienteinformacion->previous_id = -1;

  for(i = 0; i < n; ++i){
    pid = fork();

    if(pid < 0){
      perror("fork");
      exit(EXIT_FAILURE);
    }
    else if(pid == 0){
      while(1){
        sem_wait(sem_lect);

        /*Si es el hijo generamos un numero aleatorio*/
        srand(pid);
        sleep(rand()%10 +1);

        clienteinformacion->previous_id ++;
        printf("Introduzca el nombre del nuevo usuario ( %d)\n", getpid());
        scanf("%s",nombreaux);
        strcpy(clienteinformacion->name, nombreaux);

        clienteinformacion->id++;
        /*Mandamos la señal al padre*/
        kill(getppid(), SIGUSR1);
        /*Permitimos que otro hijo pueda hacer sus funciones*/

        exit(EXIT_SUCCESS);

      }
    }
    else{

    }
  }

  if(pid > 0){
    sigemptyset(&(act.sa_mask));
    act.sa_handler = manejador_SIGUSR1;
    act.sa_flags = 0;

    if(sigaction(SIGUSR1, &act, NULL) < 0){
      perror("Sigaction");
      exit(EXIT_FAILURE);
    }
    for(i = 0; i < n; ++i){
      pause();
      sem_post(sem_lect);
    }
    while(wait(NULL)>0);
    /*Liberamos todos los recursos*/
    munmap(clienteinformacion, sizeof(*clienteinformacion));
  	shm_unlink(SHM_NAME);
    sem_close(sem_lect);
    sem_unlink(SEMA);
    exit(EXIT_SUCCESS);
  }
  return EXIT_SUCCESS;
}
