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
	mqd_t queue, queue2;
	struct mq_attr attributes, attributes2;
	char *msg;
  int i, aux = MAXIMO;
	attributes.mq_flags = 0;
	attributes.mq_maxmsg = 10;
	attributes.mq_curmsgs = 0;
	attributes.mq_msgsize = 2048;

	attributes2.mq_flags = 0;
	attributes2.mq_maxmsg = 10;
	attributes2.mq_curmsgs = 0;
	attributes2.mq_msgsize = 2048;

  msg = (char*)malloc(sizeof(char)*MAXIMO);

  if(argc != 3){
    printf("Introduzca los parametros correctamente\n" );
    exit(EXIT_FAILURE);
  }

	queue = mq_open(argv[1], O_RDONLY,
				S_IRUSR | S_IWUSR,
				&attributes);

	if(queue == (mqd_t)-1) {
		perror("mq_open");
		return EXIT_FAILURE;
	}

  queue2 = mq_open(argv[2],
        O_CREAT | O_WRONLY,
        S_IRUSR | S_IWUSR,
        &attributes2);

  if(queue2 == (mqd_t)-1) {
    perror("mq_open");
    return EXIT_FAILURE;
  }

  while(aux == MAXIMO){

    aux = mq_receive(queue, msg, sizeof(char)*MAXIMO, NULL);

    if (aux == -1) {
      perror("mq_receive");
      return EXIT_FAILURE;
    }
		for (i = 0; i < aux; i++){

			if(msg[i] < 'z' && msg[i] >= 'a'){
        msg[i]++;
      }
      else if(msg[i] == 'z'){
        msg[i] = 'a';
      }
		}

		if(mq_send(queue2, msg, aux, 1) == -1) {  //parará cuando la cola esté vacia
				fprintf (stderr, "Error sending message\n");
				return EXIT_FAILURE;
		}

  }
	mq_close(queue);
  mq_unlink(argv[1]);
  mq_close(queue2);
	return EXIT_SUCCESS;
}
