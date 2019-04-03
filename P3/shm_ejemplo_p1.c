/**
 * @file
 *
 * @brief Código de ejemplo de memoria compartida, para un proceso que crea
 * la memoria.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define SHM_NAME "/shm_example"
#define INT_LIST_SIZE 10
#define MSG_MAX 100
#define MESSAGE "Hello world shared memory!"


typedef struct{
	int integer_list[INT_LIST_SIZE];
	char message[MSG_MAX];
} ShmExampleStruct;

#define MAXBUF 10


int main(void) {

	int i;
	int fd_shm;
	int error;
	ShmExampleStruct *example_struct;

    /* We create the shared memory */
	fd_shm = shm_open(SHM_NAME,
		O_RDWR | O_CREAT | O_EXCL, /* Create it and open for reading and writing */ 
		S_IRUSR | S_IWUSR); /* The current user can read and write */

	if(fd_shm == -1) {
		fprintf (stderr, "Error creating the shared memory segment \n");
		return EXIT_FAILURE;
	}
    
    /* Resize the memory segment */
	error = ftruncate(fd_shm, sizeof(ShmExampleStruct));

	if(error == -1) {
		fprintf (stderr, "Error resizing the shared memory segment \n");
		shm_unlink(SHM_NAME);
		return EXIT_FAILURE;
	}
    
	/* Map the memory segment */
	example_struct = (ShmExampleStruct *)mmap(NULL, sizeof(*example_struct), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);

	if(example_struct == MAP_FAILED) {
		fprintf (stderr, "Error mapping the shared memory segment \n");
		shm_unlink(SHM_NAME);
		return EXIT_FAILURE;
	}
	printf("Pointer to shared memory segment: %p\n", (void*)example_struct);

	/* Initialize the memory */
	memcpy(example_struct->message, MESSAGE, sizeof(MESSAGE));
	for (i = 0; i < MAXBUF; i++) {
		example_struct->integer_list[i] = i;
	}
      
	/* The daemon executes until press some character */
	getchar();
    
	/* Free the shared memory */
	munmap(example_struct, sizeof(*example_struct));
	shm_unlink(SHM_NAME);

	return EXIT_SUCCESS;
}
