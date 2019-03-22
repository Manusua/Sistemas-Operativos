#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
/* manejador: rutina de tratamiento de la señal SIGINT. */
void manejador(int sig){
printf("He conseguido capturar SIGKILL");
fflush(stdout);
}

int main(void){
struct sigaction act;
    act.sa_handler = manejador;
    sigemptyset(&(act.sa_mask));
    act.sa_flags =0;
if(sigaction(SIGKLL,&act,NULL)<0){
  perror("sigaction");
  exit(EXIT_FAILURE);
}
while(1){
printf("En espera de SigKill (PID = %d)\n", getpid());
        sleep(9999);
}
}