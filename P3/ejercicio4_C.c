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
	char* msg;
  int aux = MAXIMO;
	attributes.mq_flags = 0;
	attributes.mq_maxmsg = 10;
	attributes.mq_curmsgs = 0;
	attributes.mq_msgsize = 2048;

  msg = (char*)malloc(sizeof(char)*2048);

  if(argc != 2){
    printf("Introduzca los parametros correctamente\n" );
    exit(EXIT_FAILURE);
  }

	queue = mq_open(argv[1],
			  O_RDONLY,
				S_IRUSR | S_IWUSR,
				&attributes);

	if(queue == (mqd_t)-1) {
		perror("mq_open");
		return EXIT_FAILURE;
	}

while(aux == MAXIMO){
  aux = mq_receive(queue, msg, sizeof(char)*MAXIMO, NULL);
  if (aux == -1) {
    perror("mq_receive");
    return EXIT_FAILURE;
  }

  printf("%s", msg);
}

	mq_close(queue);
  mq_unlink(argv[1]);
	return EXIT_SUCCESS;
}
