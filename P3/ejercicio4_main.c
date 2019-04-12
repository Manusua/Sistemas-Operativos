

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define COLA1 "/cola1"
#define COLA2 "/cola2"

int main(){
  pid_t pid1, pid2, pid3;

  pid1 = fork();
  if (pid1 < 0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if (pid1 == 0){
    if (execl("./ejercicio4_A", "./ejercicio4_A", "file.txt", COLA1, (char*)NULL) == -1) {
      perror("execl");
      return EXIT_FAILURE;
    }
  }

  pid2 = fork();
  if (pid2 < 0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if (pid2 == 0){
    if (execl("./ejercicio4_B", "./ejercicio4_B", COLA1, COLA2, (char*)NULL) == -1) {
      perror("execl");
      return EXIT_FAILURE;
    }
  }
  pid3 = fork();
  if (pid3 < 0){
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if (pid3 == 0){
    if (execl("./ejercicio4_C", "./ejercicio4_C", COLA2, (char*)NULL) == -1) {
      perror("execl");
      return EXIT_FAILURE;
    }
  }

  while(wait(NULL)>0);

  exit(EXIT_SUCCESS);
}
