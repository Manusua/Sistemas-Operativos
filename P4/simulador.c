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
#include <errno.h>
#include <time.h>
#include <semaphore.h>

#include <mapa.h>

bool nuevo_turno;
//Declaramos de manera global también el semafoto y la cola para poder cerrarlos en el manejador
sem_t *sem_moni = NULL;
sem_t *sem_mapa = NULL;
mqd_t queue;
tipo_mapa* mapa;

typedef struct{
  int tipo;
  int dirx;
  int diry;
  int nave;
  int equipo;
} ACCIONES;

/*Para que cuando salgamos de una ejeccion con Ctrl+C se eliminen rodos los recursos*/
void manejador_SIGINT(int sig){
  munmap(mapa, sizeof(tipo_mapa));
	shm_unlink(SHM_MAP_NAME);
  mq_close(queue);
	mq_unlink(MQ_NAME);
  sem_close(sem_mapa);
  sem_unlink(SEM_MAPA);
  sem_close(sem_moni);
  sem_unlink(SHM_MONITOR);
  exit(EXIT_FAILURE);
}
//TODO cambiar lo de los turnos
void manejador_SIGALARM(int sig){

	printf("\nNuevo turno\n");
	nuevo_turno = true;
}


void manejador_SIGTERM(int sig){
  printf("\nRecibida SIGTERM. PID; %ld\n", (long)getpid());
  munmap(mapa, sizeof(tipo_mapa));
  shm_unlink(SHM_MAP_NAME);
  mq_close(queue);
	mq_unlink(MQ_NAME);
  sem_close(sem_mapa);
  sem_unlink(SEM_MAPA);
  sem_close(sem_moni);
  sem_unlink(SHM_MONITOR);
  exit(EXIT_FAILURE);
}

void inicializar_mapa(tipo_mapa* mapa){
	int i,j, posx = 0, posy = 0;
	tipo_nave nave;
	nave.vida = VIDA_MAX;
	nave.viva = true;

	/*Inicilaizamos todas las casillas del mapa a vacías inicialmente*/
	for(i = 0; i < MAPA_MAXX;++i){
		for(j = 0; j < MAPA_MAXY; ++j){
			mapa_clean_casilla(mapa, j, i);
		}
	}
	for(i = 0; i < N_EQUIPOS; ++i){
		nave.equipo = i;
		mapa_set_num_naves(mapa, i, N_NAVES);
		for(j = 0; j < N_NAVES; ++j){
			nave.numNave = j;
			srand(time(NULL));
			while(!mapa_is_casilla_vacia(mapa, posy, posx)){
				posx = rand()%MAPA_MAXX;
				posy = rand()%MAPA_MAXY;
			}
			nave.posx = posx;
			nave.posy = posy;
			mapa_set_nave(mapa, nave);
		}
	}
}

int main() {
	pid_t pid_jefe, pid_nave;
	//Una cola por naves o una cola para todas las naves
	struct mq_attr attributes;
	int i, j,auxi_nave, aux_accion, fd_jefe[N_EQUIPOS][2], pipe_status, fd_naves[N_NAVES][2],
  fd_shm_mapa, error_mapa, error, num_naves_vivas, aux_des, minimo, y, z, aux_minimo, ui, 
  aux_num_buc, dest[2], aux_mov[2], aux_mov_pos[2];
  struct sigaction act;
	char aux[20], aux_jefe[20], aux_nave[20];
  bool finalizado = false, encontrado;
  ACCIONES acc;
	attributes.mq_flags = 0;
	attributes.mq_maxmsg = 10;
	attributes.mq_curmsgs = 0;
	attributes.mq_msgsize = sizeof(aux);

  printf("Simulador: Inicializando recursos\n");

  sigemptyset(&(act.sa_mask));
  act.sa_flags = 0;
  act.sa_handler = manejador_SIGINT;

  if (sigaction(SIGINT, &act, NULL) < 0) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }
  act.sa_handler = manejador_SIGTERM;

  if (sigaction(SIGTERM, &act, NULL) < 0) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

	fd_shm_mapa = shm_open(SHM_MAP_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if(fd_shm_mapa == -1){
		perror("shm_open");
		exit(EXIT_FAILURE);
	}

	error_mapa = ftruncate(fd_shm_mapa, sizeof(tipo_mapa));
	if(error_mapa == -1){
		perror("ftruncate");
		shm_unlink(SHM_MAP_NAME);
		exit(EXIT_FAILURE);
	}

	mapa = (tipo_mapa*)mmap(NULL, sizeof(tipo_mapa), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm_mapa, 0);
	if(mapa == MAP_FAILED){
		perror("mmap");
		shm_unlink(SHM_MAP_NAME);
		exit(EXIT_FAILURE);
	}

  if((sem_mapa = sem_open(SEM_MAPA, O_CREAT, S_IWUSR | S_IRUSR, 1)) == SEM_FAILED){
    //TODO cerrar en control
      perror("sem_open");
      sem_close(sem_moni);
      sem_unlink(SHM_MONITOR);
      munmap(mapa,sizeof(tipo_mapa));
      shm_unlink(SHM_MAP_NAME);
      exit(EXIT_FAILURE);
    }
//i va a ser el numeor de identificador(interno) de la nave jefe
	for(i = 0; i < N_EQUIPOS;++i){//Abrimos la tubería con el simulador
  	pipe_status = pipe(fd_jefe[i]);
  	if(pipe_status == -1) {
  		perror("pipe");
  		shm_unlink(SHM_MAP_NAME);
  		exit(EXIT_FAILURE);

    }
		pid_jefe = fork();
		if(pid_jefe < 0){
			perror("fork");
			shm_unlink(SHM_MAP_NAME);
			exit(EXIT_FAILURE);
		}
		else if(pid_jefe == 0){
			//Proceso de la nave jefe

      sigemptyset(&act.sa_mask);

    	act.sa_handler = SIG_IGN;

      if (sigaction(SIGALRM, &act, NULL) < 0) {
        perror("sigaction");
        sem_close(sem_moni);
        sem_unlink(SHM_MONITOR);
        munmap(mapa,sizeof(tipo_mapa));
        shm_unlink(SHM_MAP_NAME);
        exit(EXIT_FAILURE);
      }
			//j va a ser el numero de identificador (interno) de la nave
			for(j = 0; j < N_NAVES; ++j){
				//Abrimos la tubería con la nave jefe
	 			pipe_status = pipe(fd_naves[j]);
				if(pipe_status == -1) {
			 		perror("Error creando la tuberia\n");
          sem_close(sem_moni);
          sem_unlink(SHM_MONITOR);
          munmap(mapa,sizeof(tipo_mapa));
          shm_unlink(SHM_MAP_NAME);
					exit(EXIT_FAILURE);
			  }
				pid_nave = fork();
				if(pid_nave < 0){
					perror("fork");
          sem_close(sem_moni);
          sem_unlink(SHM_MONITOR);
          munmap(mapa,sizeof(tipo_mapa));
          shm_unlink(SHM_MAP_NAME);
					exit(EXIT_FAILURE);
				}
				else if(pid_nave == 0){
					//Proceso de la nave
          printf("Nave %c/%d: abriendo cola\n",symbol_equipos[i], j);
          queue = mq_open(MQ_NAME,
                O_CREAT | O_WRONLY,
                S_IRUSR | S_IWUSR,
                &attributes);

          if(queue == (mqd_t)-1) {
            perror("mq_open");
            sem_close(sem_moni);
            sem_unlink(SHM_MONITOR);
            munmap(mapa,sizeof(tipo_mapa));
            shm_unlink(SHM_MAP_NAME);
            return EXIT_FAILURE;
          }

          printf("Nave %c/%d: cola abierta\n",symbol_equipos[i], j);

					while(1){
            printf("Nave %c/%d: esperando a recibir algo por pipe\n",symbol_equipos[i], j);
						close(fd_naves[j][1]);
						error = read(fd_naves[j][0], aux_nave, sizeof(aux_nave));
            printf("Nave %c/%d: ya ha recibido algo por pipe\n",symbol_equipos[i], j);
						if(error == -1){
							perror("read");
							mq_close(queue);
							mq_unlink(MQ_NAME);
              sem_close(sem_moni);
              sem_unlink(SHM_MONITOR);
              munmap(mapa,sizeof(tipo_mapa));
              shm_unlink(SHM_MAP_NAME);
							exit(EXIT_FAILURE);
						}
            sem_wait(sem_mapa);
						if(aux_nave[0] == 'A'){
							//Tenemos que ATACAR
							//Buscamos nave enemiga

              printf("Nave %c/%d: recibida orden de atacar\n",symbol_equipos[i], j);
              //Aunque mas adelante también comprobamos el rango, solo vamos a buscar naves a nuestro alcance
              //Vamos a hacer una IA basada en localizar a la nave enemiga mas cercana
              minimo = ATAQUE_ALCANCE ;
              dest[0] = -1;
              dest[1] = -1;
              encontrado = false;
              for(y = 0; y < N_EQUIPOS; ++y){
                //Si no es del mismo equipo
                if(y != i){
                    for(z = 0; z < N_NAVES; ++z){
                      if((aux_minimo = mapa_get_distancia(mapa, mapa->info_naves[i][j].posy, mapa->info_naves[i][j].posx, mapa->info_naves[y][z].posy, mapa->info_naves[y][z].posx)) <= minimo
                      && mapa->info_naves[y][z].viva){
                        minimo = aux_minimo;
                        dest[0] = mapa->info_naves[y][z].posx;
                        dest[1] = mapa->info_naves[y][z].posy;
                        encontrado = true;
                      }
                    }
                }
              }
							//Enviamos el mensaje a simulador
              //Solo enviamos el mensaje al simulador si se ha encontrado nave a la que disparar
              if(encontrado){
                acc.tipo = ATACAR;
                acc.dirx = dest[0];
                acc.diry = dest[1];
                acc.nave = j;
                acc.equipo = i;
                printf("ACCION ATAQUE[%c%d] %d,%d->%d,%d\n", symbol_equipos[i],j, mapa->info_naves[i][j].posy, mapa->info_naves[i][j].posx, acc.dirx, acc.diry);

                printf("Nave %c/%d: enviando mensaje por cola de mensajes\n",symbol_equipos[i], j);
                if(mq_send(queue, (char *)&acc, sizeof(acc), 1) == -1){
  								perror("mq_send");
                  mq_close(queue);
    							mq_unlink(MQ_NAME);
                  sem_close(sem_moni);
                  sem_unlink(SHM_MONITOR);
                  munmap(mapa,sizeof(tipo_mapa));
                  shm_unlink(SHM_MAP_NAME);
    							exit(EXIT_FAILURE);
  							}
              }
              else{
                acc.tipo = ERROR;

                printf("ACCION ERROR[%c%d] \n", symbol_equipos[i], j);

                printf("Nave %c/%d: enviando mensaje por cola de mensajes\n",symbol_equipos[i], j);
                if(mq_send(queue, (char *)&acc, sizeof(acc), 1) == -1){
  								perror("mq_send");
                  mq_close(queue);
    							mq_unlink(MQ_NAME);
                  sem_close(sem_moni);
                  sem_unlink(SHM_MONITOR);
                  munmap(mapa,sizeof(tipo_mapa));
                  shm_unlink(SHM_MAP_NAME);
    							exit(EXIT_FAILURE);
  							}

              }
						}
						else if(aux_nave[0] == 'M'){
							//Movimiento aleatorio

              printf("Nave %c/%d: recibida orden de mover\n",symbol_equipos[i], j);

              srand(getpid() + time(0));
              //Por si tiene mas movimiento que mapa
              aux_mov_pos[0] = mapa_get_nave(mapa, i, j).posx;
              aux_mov_pos[1] = mapa_get_nave(mapa, i, j).posy;
              aux_mov[0] = (rand()%MOVER_ALCANCE);
              aux_mov[1] = (rand()%MOVER_ALCANCE);
              aux_num_buc = 0;
              //Para el caso en el que una nave no puede moverse, metemos una variable y, si al paso 100 no lo ha encontrado, por probabilidades, supondremos que está bloqueado
              while((aux_num_buc < 100)&&(((aux_mov[0] == 0) && (aux_mov[1] == 0)) || ((mapa_get_distancia(mapa,aux_mov_pos[1], aux_mov_pos[0], aux_mov[1], aux_mov[0] ) <= MOVER_ALCANCE)&&!mapa_is_casilla_vacia(mapa, aux_mov_pos[1] + aux_mov[1], aux_mov_pos[0] + aux_mov[0])))){

                aux_mov[0] = (rand()%MAPA_MAXX)%MOVER_ALCANCE;
                aux_mov[1] = (rand()%MAPA_MAXY)%MOVER_ALCANCE;
                aux_num_buc++;
              }
              //Fin movimiento aleatorio
							strcpy(aux_nave, "MOVER");
              acc.tipo = MOVER;
              acc.dirx = aux_mov_pos[0] + aux_mov[0];
              acc.diry = aux_mov_pos[1] + aux_mov[1];
              acc.nave = j;
              acc.equipo = i;
              printf("Nave %c/%d: ACCION MOVER %d,%d->%d,%d\n", symbol_equipos[i],j, mapa->info_naves[i][j].posx, mapa->info_naves[i][j].posy, acc.dirx, acc.diry);

            //  printf("Nave %d/%d: enviando mensaje por cola de mensajes\n",i, j);

							if(mq_send(queue, (char *)&acc, sizeof(acc), 1) == -1){
								perror("mq_send");
                mq_close(queue);
  							mq_unlink(MQ_NAME);
                sem_close(sem_moni);
                sem_unlink(SHM_MONITOR);
                munmap(mapa,sizeof(tipo_mapa));
                shm_unlink(SHM_MAP_NAME);
  							exit(EXIT_FAILURE);
							}
						}
						else if(aux_nave[0] == 'D'){
							//Destruir
							//Termina el proceso de la nave
              printf("Nave %c/%d: ACCION DESTRUIR, CHAO!\n", symbol_equipos[i],j);
              sem_post(sem_mapa);
              mq_close(queue);
              munmap(mapa, sizeof(tipo_mapa));
              exit(EXIT_SUCCESS);
						}
						else{
							//ha habido un error en el read o en el write pues solo se envian dos tipos de mensajes
              sem_post(sem_mapa);
							mq_close(queue);
							mq_unlink(MQ_NAME);
              sem_close(sem_moni);
              sem_unlink(SHM_MONITOR);
              munmap(mapa,sizeof(tipo_mapa));
              shm_unlink(SHM_MAP_NAME);
							exit(EXIT_FAILURE);
						}
            sem_post(sem_mapa);
					}
					exit(EXIT_SUCCESS);
				}
			}



			while(1){
        printf("Jefe %c: esperando a recibir algo por pipe\n", symbol_equipos[i]);
				close(fd_jefe[i][1]);
				error = read(fd_jefe[i][0], aux, sizeof(aux));

        printf("Jefe %c: ya ha recibido algo por pipe\n", symbol_equipos[i]);
				if(error == -1){
					perror("read");
          sem_close(sem_moni);
          sem_unlink(SHM_MONITOR);
          munmap(mapa,sizeof(tipo_mapa));
          shm_unlink(SHM_MAP_NAME);
					exit(EXIT_FAILURE);

				}
				/*Comprobamos que mensaje hemos recibido, al solo haber tres
				posibilidades unicamente comprobamos la primera letras*/
				if(aux[0] == 'F'){
					//Finalizar la ejecucion

          printf("Jefe %c: recibido fin del juego\n", symbol_equipos[i]);
          kill(0, SIGTERM);
				}
				else if(aux[0] == 'D'){
					//Destruir
          aux_des = (int)aux[8];

          printf("Jefe %c: recibido destruir nave %d\n", symbol_equipos[i], aux_des);
          close(fd_naves[aux_des][0]);
					write(fd_naves[aux_des][1], aux, sizeof(aux));


				}
				else if(aux[0] == 'T'){
					//Nuevo turno

          printf("Jefe %c: recibido nuevo turno\n", symbol_equipos[i]);
          //Elegimos la accion aleatoriamente(atacar o mover)
          for(ui = 0; ui < 2; ui++){
          //  printf("Generando accion %d del jefe %d\n",ui, i );
            usleep(10);
  					srand(time(NULL) + ui);
  					aux_accion = rand()%2;
  					if(aux_accion == 0)
  						strcpy(aux_jefe, "ATACAR");
  					else
  						strcpy(aux_jefe, "MOVER");
  					//Escribimos a una nave aleatoria que haga la accion
  					auxi_nave = rand()%3;
            while(!mapa->info_naves[i][auxi_nave].viva){
              auxi_nave = (auxi_nave+ 1)%3;
            }

            printf("Jefe %c, enviando accion %d: %s a la nave %d\n",symbol_equipos[i], ui,aux_jefe, auxi_nave);
  					close(fd_naves[auxi_nave][0]);
  					write(fd_naves[auxi_nave][1], aux_jefe, sizeof(aux_jefe));
            printf("Jefe %c, ya esta enviada la accion %d: %s a la nave %d\n",symbol_equipos[i], ui,aux_jefe, auxi_nave);
          }
				}
				else{
					//Se ha recibido algo inesperado pq o recibe FIn o Turno o Desgtruir
          printf("Jefe %c recibido error\n", symbol_equipos[i]);
          printf("Recibido por pipe: %s\n",aux);
					perror("read");
          sem_close(sem_moni);
          sem_unlink(SHM_MONITOR);
          munmap(mapa,sizeof(tipo_mapa));
          shm_unlink(SHM_MAP_NAME);
					exit(EXIT_FAILURE);

				}
			}
			exit(EXIT_SUCCESS);
		}
	}
	//Proceso del simulador
	inicializar_mapa(mapa);
	//Armamos el recibidor de la alarma

  if((sem_moni = sem_open(SHM_MONITOR, O_CREAT, S_IWUSR | S_IRUSR, 1)) == SEM_FAILED){
      perror("sem_open");
      sem_close(sem_moni);
      sem_unlink(SHM_MONITOR);
      sem_close(sem_mapa);
      sem_unlink(SEM_MAPA);
      munmap(mapa,sizeof(tipo_mapa));
      shm_unlink(SHM_MAP_NAME);
      exit(EXIT_FAILURE);
    }

  printf("SIMULADOR: abriendo cola\n");
  queue = mq_open(MQ_NAME,
          O_CREAT | O_RDONLY,
          S_IRUSR | S_IWUSR,
          &attributes);

  if(queue == (mqd_t)-1) {
      perror("mq_open");
      sem_close(sem_moni);
      sem_unlink(SHM_MONITOR);
      sem_close(sem_mapa);
      sem_unlink(SEM_MAPA);
      munmap(mapa,sizeof(tipo_mapa));
      shm_unlink(SHM_MAP_NAME);
      return EXIT_FAILURE;
    }
  printf("SIMULADOR: cola abierta\n");

  sem_post(sem_moni);

  sigemptyset(&act.sa_mask);
  act.sa_handler = manejador_SIGALARM;

  if (sigaction(SIGALRM, &act, NULL) < 0) {
      perror("sigaction");
      mq_close(queue);
      mq_unlink(MQ_NAME);
      sem_close(sem_mapa);
      sem_unlink(SEM_MAPA);
      sem_close(sem_moni);
      sem_unlink(SHM_MONITOR);
      munmap(mapa,sizeof(tipo_mapa));
      shm_unlink(SHM_MAP_NAME);
      exit(EXIT_FAILURE);
    }
  printf("Simulador: mapa y recursos iniciliazados\n");

	nuevo_turno = true;

	while(!finalizado){
    printf("Nuevo procesamiento por parte del simulador %ld\n", (long) getpid());
		if(nuevo_turno){

      printf("Simulador: Nuevo turno\n");
			//Establecemos una alarma para cambiar al siguiente tunro en un futuro
			alarm(TURNO_SECS);
			nuevo_turno = false;
      printf("Simulador: restaurando mapa\n");
      mapa_restore(mapa);


  			//Comunicamos al jefe que es nuevo turno pq no hay ganadores
        printf("Simulador: no hay ningun ganador\n");
        //Comprobamos que el jugador asignado sigue vivo

        for(i = 0; i < N_EQUIPOS; i++){
          num_naves_vivas = 0;
          for(j = 0; j < N_NAVES; j++){
            if(mapa->info_naves[i][j].viva)
              num_naves_vivas++;
          }
          if(num_naves_vivas != 0){
            close(fd_jefe[i][0]);

            strcpy(aux, "TURNO");

            printf("Simulador: enviando el nuevo turno al jefe\n");
            write(fd_jefe[i][1], aux, sizeof(aux));
            printf("Simulador: enviado el nuevo turno al jefe\n");
          }
        }
      }

    /*No tengo nada claro este else, es para que no procese acciones tras encontrar
    que ha finalizado la partida y se quede esperando sin comprobar finalizado*/
    else{

      //Comprobamos si hay un ganador
      int p,q;
      int auxigan = 0;
      printf("Simulador: comprobando si hay  un ganador\n");
      for(p = 0; p < N_EQUIPOS; ++p){
        if(mapa->num_naves[p] != 0)
          auxigan++;
      }
      for (size_t yt = 0; yt < N_EQUIPOS; yt++) {
        printf("Equipo %c, numero naves vivas %d\n", symbol_equipos[yt], mapa->num_naves[yt]);
      }
      if(auxigan == 0){
        //Se han destruido todas las naves a la vez
        for(q = 0; q < N_EQUIPOS; ++q){
          close(fd_jefe[q][0]);
    			strcpy(aux, "FIN");
          write(fd_jefe[q][1], aux, sizeof(aux));
        }
        finalizado = true;
        printf("Simulador: Fin del juego, no hay ganadores!\n");
      }
      else if(auxigan == 1){
        //Hay un ganador
        printf("Simulador: hay un ganador\n");
        for(q = 0; q < N_EQUIPOS; ++q){
          close(fd_jefe[q][0]);
    			strcpy(aux, "FIN");
          write(fd_jefe[q][1], aux, sizeof(aux));
        }
        finalizado = true;
        for(q = 0; q < N_EQUIPOS; ++q){
          if(mapa->num_naves[q] != 0){
            printf("Simulador: Fin del juego, el ganador es el equipo %c. \n¡¡ENHORABUENA!!\n", symbol_equipos[q]);
          }
        }
      }
      else{
      //No  hay ganadores
      printf("Simulador: escuchando cola de mensajes\n");
      if(mq_receive(queue, (char*)&acc, sizeof(acc), NULL) == -1){
        if (errno == EINTR ){
          if (!nuevo_turno)
            break;
          continue;
        }
        perror("mq_receive");
    		mq_close(queue);
    		mq_unlink(MQ_NAME);
        sem_close(sem_mapa);
        sem_unlink(SEM_MAPA);
        sem_close(sem_moni);
        sem_unlink(SHM_MONITOR);
        munmap(mapa,sizeof(tipo_mapa));
        shm_unlink(SHM_MAP_NAME);
    		exit(EXIT_FAILURE);
      }

      printf("Simulador: recibido cola de mensajes\n");
      sem_wait(sem_mapa);
      if(acc.tipo == ERROR){
        //Error
        //Se llega aqui si no ha encontrado a quien atacar
        printf("Simulador: ACCION ERROR\n");
      }
      else if(acc.tipo == MOVER){
        //MOVER
        printf("Simulador: ACCION MOVER: nave %d equipo %c hasta %d,%d\n",acc.nave,symbol_equipos[acc.equipo], acc.dirx, acc.diry);
        /*Una vez llegados aquí, ya sabemos que la casilla de destino es valida,
        pero volvemos a comprobar*/

        if(acc.dirx >=0 && acc.dirx < MAPA_MAXX && acc.diry >=0 && acc.diry < MAPA_MAXY ){
          if(mapa_is_casilla_vacia(mapa, acc.diry, acc.dirx)){
            mapa_clean_casilla(mapa, mapa->info_naves[acc.equipo][acc.nave].posy, mapa->info_naves[acc.equipo][acc.nave].posx);
            mapa->info_naves[acc.equipo][acc.nave].posx = acc.dirx;
            mapa->info_naves[acc.equipo][acc.nave].posy = acc.diry;
            mapa_set_nave(mapa, mapa->info_naves[acc.equipo][acc.nave]);
          }
        }
        //sino no pasa nada, volvemos a ejecutar
      }
      else if(acc.tipo == ATACAR){
        //ATACAR
        printf("Simulador: ACCION ATACAR: nave %d equipo %c hacia %d,%d\n", acc.nave, symbol_equipos[acc.equipo], acc.dirx, acc.diry);

        /*Una vez llegados aquí, ya sabemos que la casilla de destino es valida,
        pero volvemos a comprobar*/

        if(acc.dirx >=0 && acc.dirx < MAPA_MAXX && acc.diry >=0 && acc.diry < MAPA_MAXY ){
          tipo_nave nave_aux = mapa_get_nave(mapa, acc.equipo, acc.nave);
          if(mapa_get_distancia(mapa, nave_aux.posy, nave_aux.posx, acc.diry, acc.dirx)<ATAQUE_ALCANCE){
            mapa_send_misil(mapa, nave_aux.posy, nave_aux.posx, acc.diry, acc.dirx);

            if(mapa_is_casilla_vacia(mapa, acc.diry, acc.dirx)){
              mapa_set_symbol(mapa, acc.diry, acc.dirx, SYMB_AGUA);
              printf("Simulador: el disparo acabó en agua, VAYA!\n");

            }
            else{
              //Si no es que hay una nave
              tipo_casilla aux_casilla = mapa_get_casilla(mapa, acc.diry, acc.dirx);
              //aux_nave_cas es la nave atacada
              tipo_nave aux_nave_cas = mapa_get_nave(mapa,aux_casilla.equipo, aux_casilla.numNave);
              //Si es fuego amigo no pasa nada
              if(aux_nave_cas.equipo != acc.equipo){
                mapa->info_naves[aux_nave_cas.equipo][aux_nave_cas.numNave].vida =-ATAQUE_DANO;
                if(mapa->info_naves[aux_nave_cas.equipo][aux_nave_cas.numNave].vida <= 0){
                  //La nave debe ser destruida
                  mapa_set_symbol(mapa, acc.diry, acc.dirx, SYMB_DESTRUIDO);

                  printf("Simulador: la nave %d del equipo %c ha sido destruida\n", aux_nave_cas.numNave, symbol_equipos[aux_nave_cas.equipo]);
                  strcpy(aux, "DESTRUIR");
                  aux[8] = (char)aux_nave_cas.numNave;
                  //Comunicamos a la nave jefe correspondiente que su nave debe morir
                  close(fd_jefe[aux_nave_cas.equipo][0]);

                  write(fd_jefe[aux_nave_cas.equipo][1], aux, sizeof(aux));

                  //Actualizamos la informacion del mapa
                  mapa->num_naves[aux_nave_cas.equipo]--;
                  mapa->info_naves[aux_nave_cas.equipo][aux_nave_cas.numNave].viva = false;
                  mapa->info_naves[aux_nave_cas.equipo][aux_nave_cas.numNave].vida = 0;
                  mapa_clean_casilla(mapa, mapa->info_naves[aux_nave_cas.equipo][aux_nave_cas.numNave].posy, mapa->info_naves[aux_nave_cas.equipo][aux_nave_cas.numNave].posx);

                }
                else{
                  mapa_set_symbol(mapa, acc.diry, acc.dirx, SYMB_TOCADO);
                  mapa->info_naves[aux_nave_cas.equipo][aux_nave_cas.numNave].vida =- 10;
                  printf("Simulador: la nave %d del equipo %d no ha sido destruida, aun\n", aux_nave_cas.numNave, symbol_equipos[aux_nave_cas.equipo]);

                }
              }
            }
          }
        }
      }
      else{
        //Ha habido un error al recibir pues solo podemos recibir atacar, mover y error
        sem_post(sem_mapa);
        perror("mq_receive");
        mq_close(queue);
        mq_unlink(MQ_NAME);
        sem_close(sem_mapa);
        sem_unlink(SEM_MAPA);
        sem_close(sem_moni);
        sem_unlink(SHM_MONITOR);
        munmap(mapa,sizeof(tipo_mapa));
        shm_unlink(SHM_MAP_NAME);
        exit(EXIT_FAILURE);
      }
      sem_post(sem_mapa);
      printf("Simulador: esperando para procesar de nuevo\n");
      usleep(100000);
    }
  }
	}

  printf("Simulador: esperando a que accaben los hijos\n");
	while(wait(NULL)>0);

	if(munmap(mapa, sizeof(tipo_mapa)) == -1){
		perror("munmap");
		mq_close(queue);
		mq_unlink(MQ_NAME);
    sem_close(sem_mapa);
    sem_unlink(SEM_MAPA);
    sem_close(sem_moni);
    sem_unlink(SHM_MONITOR);
    munmap(mapa,sizeof(tipo_mapa));
    shm_unlink(SHM_MAP_NAME);
		exit(EXIT_FAILURE);
	}

  mq_close(queue);
  mq_unlink(MQ_NAME);
  sem_close(sem_mapa);
  sem_unlink(SEM_MAPA);
  sem_close(sem_moni);
  sem_unlink(SHM_MONITOR);
  munmap(mapa,sizeof(tipo_mapa));
  shm_unlink(SHM_MAP_NAME);

	exit(EXIT_SUCCESS);
}
