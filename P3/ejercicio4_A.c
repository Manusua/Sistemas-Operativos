/**
 * @file
 *
 * @brief Código de ejemplo de cola de mensajes, para un proceso emisor.
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAXIMO 2048

int main(int argc, char* argv[]) {
	mqd_t queue;
	struct mq_attr attributes;
  struct stat status;
	char* msg;
  char aux = 'a';
  int f, i;
	attributes.mq_flags = 0;
	attributes.mq_maxmsg = 10;
	attributes.mq_curmsgs = 0;
	attributes.mq_msgsize = 2048;

  if(argc != 3){
    printf("Introduzca los parametros correctamente\n" );
    exit(EXIT_FAILURE);
  }

  f = open(argv[1], O_RDONLY, S_IRUSR);
  if(f < 0){
    perror("open");
    exit(EXIT_FAILURE);
  }
	queue = mq_open(argv[2],
				O_CREAT | O_WRONLY, /* This process is only going to send messages */
				S_IRUSR | S_IWUSR, /* The user can read and write */
				&attributes);

	if(queue == (mqd_t)-1) {
		perror("mq_open");
		return EXIT_FAILURE;
	}
  if(fstat(f, &status)<0) {
		perror("fstat");
		return EXIT_FAILURE;
	}

  msg = (char*)mmap(NULL, status.st_size, PROT_READ, MAP_PRIVATE, f, 0);

  if(msg == MAP_FAILED){
    perror("mmap");
    return EXIT_FAILURE;
  }

  for(i = 0; i < status.st_size; i =+ MAXIMO){
    if(status.st_size < MAXIMO){

      if(mq_send(queue, (char *)msg, status.st_size, 1) == -1) {
    		perror("mq_send");
    		return EXIT_FAILURE;
    	}
    }
    if(mq_send(queue, (char *)msg, MAXIMO, 1) == -1) {
  		perror("mq_send");
  		return EXIT_FAILURE;
  	}
    msg = msg + 2048;
  }

	if(status.st_size%2048 != 0) {
		if(mq_send(queue, (char*)msg, status.st_size%MAXIMO, 1) == -1) {
		 	perror("mq_send");
			return EXIT_FAILURE;
		}
	}
  else{
		if(mq_send(queue,(char*)&aux, 1, 1) == -1) {
		 	perror("mq_send");
			return EXIT_FAILURE;
		}
  }

  munmap(msg, status.st_size);
	mq_close(queue);
	return EXIT_SUCCESS;
}
