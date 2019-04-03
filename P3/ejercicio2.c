#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

#define SHM_NAME "/clientInfo"
#define NAME_MAX 50

typedef struct {
  int previous_id;
  int id;
  char name[NAME_MAX];
} ClientInfo;

ClientInfo *clienteinformacion;

void manejador_SIGUSR1(int sig){
  printf("recibida SIGUSR1\n");
  printf("ID: %d.\nPrevious_id: %d\n Name: %s.\n", clienteinformacion->id, clienteinformacion->previous_id, clienteinformacion->name);
}
int main(int argc, char *argv[]){
  int n, i;
  pid_t pid;
	int fd_shm;
	int error;
  char* nombreaux;
  struct sigaction act;

  if(argc != 2) {
    printf("Introduzca el numero de procesos hijo\n");
    exit(EXIT_FAILURE);
  }
  n = (int)argv[1];

  fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

  if(fd_shm == -1) {
  	fprintf (stderr, "Error creando el segmento de memoria compartida \n");
  	return EXIT_FAILURE;
  }

  error = ftruncate(fd_shm, sizeof(ClientInfo));

  if(error == -1) {
    fprintf (stderr, "Error resizing the shared memory segment \n");
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
      srand(pid);
      sleep(rand()%10 +1);
      clienteinformacion->previous_id ++;
      printf("Introduzca el nombre del nuevo usuario\n");
      scanf("%s\n",nombreaux);
      strcpy(clienteinformacion->name, nombreaux);
      clienteinformacion->id++;
      kill(getppid(), SIGUSR1);
      exit(EXIT_SUCCESS);
    }
    else{
      /*TODO tengo que hacer que clienteinformacion sea una global par ameterlo en el manejador no?¿*/


      sigemptyset(&(act.sa_mask));
      act.sa_handler = manejador_SIGUSR1;
      act.sa_flags = 0;
      if(sigaction(SIGUSR1, &act, NULL) < 0){
        perror("Sigaction");
        exit(EXIT_FAILURE);
      }

      while(wait(NULL)>0);
      munmap(example_struct, sizeof(*example_struct));
    	shm_unlink(SHM_NAME);
      exit(EXIT_SUCCESS);
    }
  }


  return EXIT_SUCCESS;
}
