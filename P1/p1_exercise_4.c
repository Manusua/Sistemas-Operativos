#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROG 4

int main(void)
{
	pid_t pid;
  int i;
  i = 0;
  pid = fork();
	while(pid == 0 && i < NUM_PROG){
    printf("GETPID:   %d   GETPPID: %d\n", getpid(), getppid() );
    pid = fork();
    i++;
  }
	wait(NULL);
	exit(EXIT_SUCCESS);
}
