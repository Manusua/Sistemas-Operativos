#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>

#include <simulador.h>
#include <gamescreen.h>
#include <mapa.h>


void mapa_print(tipo_mapa *mapa)
{
	int i,j;

	for(j=0;j<MAPA_MAXY;j++) {
		for(i=0;i<MAPA_MAXX;i++) {
			tipo_casilla cas=mapa_get_casilla(mapa,j, i);
			//printf("%c",cas.simbolo);
			screen_addch(j, i, cas.simbolo);
		}
		//printf("\n");
	}
	screen_refresh();
}


int main() {

	int fd_shm;
	sem_t *sem_moni = NULL;
	tipo_mapa *map;
	if((sem_moni = sem_open(SHM_MONITOR, O_CREAT, S_IWUSR | S_IRUSR, 0)) == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
	sem_wait(sem_moni);
	printf("HOLI\n");
  fd_shm = shm_open(SHM_MAP_NAME, O_RDWR | O_CREAT,  S_IWUSR | S_IRUSR);
  if(fd_shm == -1){
    perror("shm_open");
    return EXIT_FAILURE;
  }

  if(ftruncate(fd_shm, sizeof(tipo_mapa)) == -1) {
      perror("Error resizing the shared memory segment");
      shm_unlink(SHM_MAP_NAME);
      return EXIT_FAILURE;
  }

  map = mmap(NULL, sizeof(*map),PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
  if(map == MAP_FAILED){
      perror("mmap");
			shm_unlink(SHM_MAP_NAME);
      return EXIT_FAILURE;
  }
	screen_init();
	printf("Monitor: empiezo a imprimir\n");
	while(1) {
		mapa_print(map);

		usleep(SCREEN_REFRESH);
	}



	screen_end();

	sem_close(sem_moni);
	sem_unlink(SHM_MONITOR);
	exit(EXIT_SUCCESS);
}
