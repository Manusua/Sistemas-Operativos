/**
 * @file
 *
 * @brief Código de ejemplo de cola de mensajes, para un proceso receptor.
 */
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>

#define MQ_NAME "/mq_example"
#define N 33

typedef struct {
	int valor;
	char aviso[80]; 
} Mensaje;

int main(void) {

	struct mq_attr attributes;
	mqd_t queue;
	Mensaje msg;

	attributes.mq_flags = 0;
	attributes.mq_maxmsg = 10;
	attributes.mq_curmsgs = 0;
	attributes.mq_msgsize = sizeof(Mensaje);

	queue = mq_open(MQ_NAME,
		O_CREAT | O_RDONLY, /* This process is only going to send messages */
		S_IRUSR | S_IWUSR, /* The user can read and write */
		&attributes); 

	if(queue == (mqd_t)-1) {
		fprintf (stderr, "Error opening the queue\n");
		return EXIT_FAILURE;
	}

	if(mq_receive(queue, (char *)&msg, sizeof(msg), NULL) == -1) {
		fprintf (stderr, "Error receiving message\n");
		return EXIT_FAILURE;
	}

	printf("%d: %s", msg.valor, msg.aviso); /* Wait for input to end the program */
	getchar();

	mq_close(queue);
	mq_unlink(MQ_NAME);

	return EXIT_SUCCESS;
}
