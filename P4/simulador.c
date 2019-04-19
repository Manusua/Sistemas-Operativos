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
#include <time.h>

#include <mapa.h>

int turno;
bool nuevo_turno;

/*Para que cuando salgamos de una ejeccion con Ctrl+C se eliminen rodos los recursos*/
void manejador_SIGINT(int sig){
  printf("\nrecibida SIGINT\n");
	shm_unlink(SHM_MAP_NAME);
	mq_unlink(MQ_NAME);
  exit(EXIT_FAILURE);
}

void manejador_SIGALARM(int sig){
	//TODO mover esto al printf
	printf("\nNuevo turno\n");
	turno ++;
	turno = turno%N_EQUIPOS;
	nuevo_turno = true;
}

void procesa_accion(){

}

void inicializar_mapa(tipo_mapa* mapa){
	int i,j, posx = 0, posy = 0;
	tipo_nave nave;
	nave.vida = VIDA_MAX;
	nave.viva = true;

	//TODO no se si esto es necesario
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
			//TODO mejorar el como inicia cada nave
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


//TODO preguntar que se puede hacer global para hacer una funcion de controlde errores y liberar recursos
int main() {
	pid_t pid_jefe, pid_nave;
	//Una cola por naves o una cola para todas las naves
	mqd_t queue;
	struct mq_attr attributes;
	//TODO cambiar los pipes a un vector continup fd[2*N_EQUIPOS] para ocupar menos espacio
	int i, j,auxi_nave, aux_accion, fd_jefe[N_EQUIPOS][2], pipe_status, fd_naves[N_EQUIPOS][N_NAVES][2], fd_shm_mapa, error_mapa;
	//TODO ver si merece la pena ponelro como variable global para el sigint
	tipo_mapa* mapa;
  struct sigaction act;
	int error;
	//TODO creo que se puede usar unicamente aux al sser procesos distintos.
	char aux[20], aux_jefe[20], aux_nave[20];
	attributes.mq_flags = 0;
	attributes.mq_maxmsg = 10;
	attributes.mq_curmsgs = 0;
	attributes.mq_msgsize = sizeof(aux);


	queue = mq_open(MQ_NAME,
				O_CREAT | O_WRONLY, /* This process is only going to send messages */
				S_IRUSR | S_IWUSR, /* The user can read and write */
				&attributes);

	if(queue == (mqd_t)-1) {
		perror("mq_open");
		return EXIT_FAILURE;
	}

  sigemptyset(&(act.sa_mask));
  act.sa_flags = 0;
  act.sa_handler = manejador_SIGINT;

  if (sigaction(SIGINT, &act, NULL) < 0) {
    perror("sigaction");
		mq_close(queue);
		mq_unlink(MQ_NAME);
    exit(EXIT_FAILURE);
  }

	fd_shm_mapa = shm_open(SHM_MAP_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if(fd_shm_mapa == -1){
		perror("shm_open");
		mq_close(queue);
		mq_unlink(MQ_NAME);
		exit(EXIT_FAILURE);
	}

	error_mapa = ftruncate(fd_shm_mapa, sizeof(tipo_mapa));
	if(fd_shm_mapa == -1){
		perror("ftruncate");
		mq_close(queue);
		mq_unlink(MQ_NAME);
		shm_unlink(SHM_MAP_NAME);
		exit(EXIT_FAILURE);
	}

	mapa = (tipo_mapa*)mmap(NULL, sizeof(tipo_mapa), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm_mapa, 0);
	if(mapa == MAP_FAILED){
		perror("mmap");
		mq_close(queue);
		mq_unlink(MQ_NAME);
		shm_unlink(SHM_MAP_NAME);
		exit(EXIT_FAILURE);
	}

//i va a ser el numeor de identificador(interno) de la nave jefe
	for(i = 0; i < N_EQUIPOS;++i){//Abrimos la tubería con el simulador
	pipe_status = pipe(fd_jefe[i]);
	if(pipe_status == -1) {
		perror("pipe");
		mq_close(queue);
		mq_unlink(MQ_NAME);
		shm_unlink(SHM_MAP_NAME);
		exit(EXIT_FAILURE);
	}
		pid_jefe = fork();
		if(pid_jefe < 0){
			perror("fork");
			mq_close(queue);
			mq_unlink(MQ_NAME);
			shm_unlink(SHM_MAP_NAME);
			exit(EXIT_FAILURE);
		}
		else if(pid_jefe == 0){
			//Proceso de la nave jefe

			//j va a ser el numero de identificador (interno) de la nave
			for(j = 0; j < N_NAVES; ++j){

					//Abrimos la tubería con la nave jefe
	 			pipe_status = pipe(fd_naves[i][j]);
				if(pipe_status == -1) {
			 		perror("Error creando la tuberia\n");
					mq_close(queue);
					mq_unlink(MQ_NAME);
					shm_unlink(SHM_MAP_NAME);
					exit(EXIT_FAILURE);
			  }
				pid_nave = fork();
				if(pid_nave < 0){
					perror("fork");
					mq_close(queue);
					mq_unlink(MQ_NAME);
					shm_unlink(SHM_MAP_NAME);
					exit(EXIT_FAILURE);
				}
				else if(pid_nave == 0){
					//Proceso de la nave
					while(1){

						close(fd_naves[i][j][1]);
						error = read(fd_naves[i][j][0], aux_nave, sizeof(aux_nave));
						if(error == -1){
							perror("read");
							mq_close(queue);
							mq_unlink(MQ_NAME);
							shm_unlink(SHM_MAP_NAME);
							exit(EXIT_FAILURE);
						}

						if(aux_nave[0] == 'A'){
							//Tenemos que ATACAR
							//Buscamos nave enemiga
							buscar_nave_enemiga();
							//Enviamos el mensaje a simulador
							strcpy(aux_nave, "ATACAR");
							if(mq_send(queue, (char *)&aux_nave, sizeof(aux_nave), 1) == -1){
								perror("mq_send");mq_close(queue);
  							mq_unlink(MQ_NAME);
  							shm_unlink(SHM_MAP_NAME);
  							exit(EXIT_FAILURE);
							}
						}
						else if(aux_nave[0] == 'M'){
							//Movimiento aleatorio
              //TODO, poner la x en el 0 y la y en el 1 de un array de tamaño dos
              int aux_movx, aux_movy, aux_mov_posx,aux_mov_posy;
              srand(getpid() + time(0));
              //Por si tiene mas movimiento que mapa
              aux_mov_posx = mapa_get_nave(mapa, i, j).posx;
              aux_mov_posy = mapa_get_nave(mapa, i, j).posy;
              aux_movx = (rand()%MAPA_MAXX);
              aux_movy = (rand()%MAPA_MAXY);
              while((mapa_get_distancia(mapa,aux_mov_posy, aux_mov_posx, aux_movy, aux_movx ) <= MOVER_ALCANCE)&&!mapa_is_casilla_vacia(mapa, aux_mov_posy + aux_movy, aux_mov_posx + aux_movx)){
                //TODO No se si me va a generar el mismo numero, hacer ++ sino o algo asi
                aux_movx = (rand()%MAPA_MAXX)%MOVER_ALCANCE;
                aux_movy = (rand()%MAPA_MAXY)%MOVER_ALCANCE;
              }
              //Fin movimiento aleatorio
							strcpy(aux_nave, "MOVER");
              //TODO hacer esto como funcion add_data_mover(aux_nave, aux_movx, aux_movy, i, j);
              itoa(aux_nave[6],aux_movx, 10);
              itoa(aux_nave[7],aux_movy, 10);
              itoa(aux_nave[8],i, 10);
              itoa(aux_nave[9],j, 10);

							if(mq_send(queue, (char *)&aux_nave, sizeof(aux_nave), 1) == -1){
								perror("mq_send");
                mq_close(queue);
  							mq_unlink(MQ_NAME);
  							shm_unlink(SHM_MAP_NAME);
  							exit(EXIT_FAILURE);
							}
						}
						else if(aux_nave[0] == 'D'){
							//Destruir
							//Entiendo que destruir es pq ya se ha comprobado la vida y esas cosas;
              mapa->info_naves[i][j].viva = false;
              mapa->num_naves[i] --;
              exit(EXIT_SUCCESS);

						}
						else{
							//ha habido un error en el read o en el write pues solo se envian dos tipos de mensajes
							mq_close(queue);
							mq_unlink(MQ_NAME);
							shm_unlink(SHM_MAP_NAME);
							exit(EXIT_FAILURE);
						}

					}

					//TODO ver como afecta lo de char* en el mq_send al tipo accio

					exit(EXIT_SUCCESS);
				}
			}



			while(1){
				close(fd_jefe[i][1]);
				error = read(fd_jefe[i][0], aux, sizeof(aux));
				if(error == -1){
					perror("read");
					mq_close(queue);
					mq_unlink(MQ_NAME);
					shm_unlink(SHM_MAP_NAME);
					exit(EXIT_FAILURE);

				}
				/*Comprobamos que mensaje hemos recibido, al solo haber tres
				posibilidades unicamente comprobamos la primera letras*/
				//TODO mejorar esto
				if(aux[0] == 'F'){
					//Finalizar la ejecucion

          //TODO no se que con sigterm-->P2
				}
				else if(aux[0] == 'D'){
					//Destruir
          strcpy(aux_jefe, "DESTRUIR");
          close(fd_naves[i][auxi_nave][0]);
					write(fd_naves[i][auxi_nave][1], aux_jefe, sizeof(aux_jefe));


				}
				else if(aux[0] == 'T'){
					//Nuevo turno

          //Elegimos la accion aleatoriamente(atacar o mover)
					srand(time(NULL));
					aux_accion = rand()%2;
					if(aux_accion == 0)
						strcpy(aux_jefe, "ATACAR");
					else
						strcpy(aux_jefe, "MOVER");
					//Escribimos a una nave aleatoria que ataque
					auxi_nave = rand()%3;
					close(fd_naves[i][auxi_nave][0]);
					write(fd_naves[i][auxi_nave][1], aux_jefe, sizeof(aux_jefe));

				}
				else{
					//Se ha recibido algo inesperado pq o recibe FIn o Turno o Desgtruir
					perror("read");
					mq_close(queue);
					mq_unlink(MQ_NAME);
					shm_unlink(SHM_MAP_NAME);
					exit(EXIT_FAILURE);

				}
			}
			//TODO tiene que acabar aquí la ejecución pues luego ya pase a proceso simulador
			exit(EXIT_SUCCESS);
		}
	}
	//Proceso del simulador
	inicializar_mapa(mapa);
	//Armamos el recibidor de la alarma
	sigemptyset(&act.sa_mask);
	act.sa_handler = manejador_SIGALARM;

	turno = 0;
	nuevo_turno = true;
  bool finalizado = false;
	//TODO cambiar la condicion a que le juego siga?¿
	while(!finalizado){
		if(nuevo_turno){
			//Establecemos una alarma para cambiar al siguiente tunro en un futuro
			alarm(TURNO_SECS);
			nuevo_turno = false;

      mapa_restore(mapa);

      //Comprobamos si hay un ganador
      int p,q;
      int auxigan = 0;
      for(p = 0; p < N_EQUIPOS; ++p){
        if(mapa->num_naves[p] == 0)
          auxigan++;
      }
      if(auxigan == 0){
        //Se han destruido todas las naves a la vez
        for(q = 0; q < N_EQUIPOS; ++q){
          close(fd_jefe[q][0]);
    			strcpy(aux, "FIN");
          write(fd_jefe[q][1], aux, sizeof(aux));
        }
        finalizado = true;
        printf("Fin del juego, no hay ganadores!\n");
      }
      else if(auxigan == 1){
        //Hay un ganador
        for(q = 0; q < N_EQUIPOS; ++q){
          close(fd_jefe[q][0]);
    			strcpy(aux, "FIN");
          write(fd_jefe[q][1], aux, sizeof(aux));
        }
        finalizado = true;
        for(q = 0; q < N_EQUIPOS; ++q){
          if(mapa->num_naves[q] != 0){
            printf("Fin del juego, el ganador es el jugador %d. \n¡¡ENHORABUENA!!\n", q);
          }
        }
      }
      else{
  			//Comunicamos al jefe que es nuevo turno pq no hay ganadores
  			close(fd_jefe[turno][0]);


  			strcpy(aux, "TURNO");
        write(fd_jefe[turno][1], aux, sizeof(aux));
      }
		}
    /*No tengo nada claro este else, es para que no procese acciones tras encontrar
    que ha finalizado la partida y se quede esperando sin comprobar finalizado*/
    else{
      //Recibimos la accion por la cola de mensajes de parte de las naves
      char aux_cola[20];
      if(mq_receive(queue, aux_cola, sizeof(aux_cola), NULL) == -1){
        perror("mq_receive");
    		mq_close(queue);
    		mq_unlink(MQ_NAME);
    		shm_unlink(SHM_MAP_NAME);
    		exit(EXIT_FAILURE);
      }
      if(aux_cola[0] == 'M'){
        /*Una vez llegados aquí, ya sabemos que la casilla de destino es valida,
        pero volvemos a comprobar*/
        int auxx_cola, auxy_cola, auxc_equipo, auxc_nave;
        auxx_cola = atoi(aux_cola[6]);
        auxy_cola = atoi(aux_cola[7]);
        auxc_equipo = atoi(aux_cola[8]);
        auxc_nave = atoi(aux_cola[9]);
        //TODO supongo que la primera columna del mapa tendra iedentificador 0
        if(auxx_cola >=0 && auxx_cola < MAPA_MAXX && auxy_cola >=0 && auxy_cola < MAPA_MAXY ){
          if(mapa_is_casilla_vacia(mapa, auxy_cola, auxx_cola)){
            mapa_clean_casilla(mapa, mapa->info_naves[auxc_equipo][auxc_nave].posy, mapa->info_naves[auxc_equipo][auxc_nave].posx);
            //TODO ponerlo como funcion de mapa.h
            mapa->info_naves[auxc_equipo][auxc_nave].posx = auxx_cola;
            mapa->info_naves[auxc_equipo][auxc_nave].posy = auxy_cola;
            mapa_set_nave(mapa, mapa->info_naves[auxc_equipo][auxc_nave]);
          }
        }
        //sino no pasa nada, volvemos a ejecutar
      }
      else if(aux_cola[0] == 'A'){
        /*Una vez llegados aquí, ya sabemos que la casilla de destino es valida,
        pero volvemos a comprobar*/
        int auxx_cola, auxy_cola, auxc_equipo, auxc_nave;
        auxx_cola = atoi(aux_cola[7]);
        auxy_cola = atoi(aux_cola[8]);
        //EQuipo e identificado rde la nave que ordena
        auxc_equipo = atoi(aux_cola[9]);
        auxc_nave = atoi(aux_cola[10]);
        //TODO supongo que la primera columna del mapa tendra iedentificador 0
        if(auxx_cola >=0 && auxx_cola < MAPA_MAXX && auxy_cola >=0 && auxy_cola < MAPA_MAXY ){
          tipo_nave nave_aux = mapa_get_nave(mapa, auxc_equipo, auxc_nave);
          if(mapa_get_distancia(mapa, nave_aux.posy, nave_aux.posx, auxy_cola, auxx_cola)>ATAQUE_ALCANCE){
            mapa_send_misil(mapa, nave_aux.posy, nave_aux.posx, auxy_cola, auxx_cola);

            //TODO voy a poner todo esto seguido
            if(mapa_is_casilla_vacia(mapa, auxy_cola, auxx_cola)){
              mapa_set_symbol(mapa, auxy_cola, auxx_cola, SYMB_AGUA);
            }
            else{
              //Si no es que hay una nave
              tipo_casilla aux_casilla = mapa_get_casilla(mapa, auxy_cola, auxx_cola));
              tipo_nave aux_nave_cas = mapa_get_nave(mapa,aux_casilla.equipo, aux_casilla.numNave);
              //Si es fuego amigo no pasa nada
              if(aux_nave_cas.equipo != auxc_equipo){
              
              }
            }
          }
        }
      }
      else{
        //Ha habido un error al recibir pues solo podemos recibir atacar y mover
        perror("mq_receive");
        mq_close(queue);
        mq_unlink(MQ_NAME);
        shm_unlink(SHM_MAP_NAME);
        exit(EXIT_FAILURE);
      }
      usleep(100000);
    }
	}


	while(wait(NULL)>0);

	if(munmap(mapa, sizeof(tipo_mapa)) == -1){
		perror("munmap");
		mq_close(queue);
		mq_unlink(MQ_NAME);
		shm_unlink(SHM_MAP_NAME);
		exit(EXIT_FAILURE);
	}
	shm_unlink(SHM_MAP_NAME);

	exit(EXIT_SUCCESS);
}
