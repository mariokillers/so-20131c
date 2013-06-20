#include "personaje.h"


t_personaje *personaje;

//variables globales
char state;
Posicion *posicionProximoRecurso;
char proxRecurso;
char proxNivel[20];
char nivelActual[20];
Posicion *nuevaPosicion;
Posicion *posicionActual;
bool respuestaConfirmacionRecurso;
char *archivo_config;

Nivel nivelActualData ;
Planificador planificadorActualData;

t_log* logger;

CCB clientCCB_orq;
CCB clientCCB_niv;
CCB clientCCB_pln;


int main(int argc, char *argv[]) {

	if (argc < 2) {
		fprintf(stderr, "Faltan argumentos. personaje archivoconfig\n");
		exit(1);
	}

	archivo_config = argv[1];

	//inicializo el personaje desde el archivo config
	inicializarPersonaje();

	//instancio el logger
	logger = log_create("personaje.log", "ProcesoPersonaje", true, LOG_LEVEL_INFO);

	//declaro las seniales
	signal(SIGTERM, rutinaSignal);
	signal(SIGUSR1, rutinaSignal);

	//inicializo la cola de mensajes
	t_queue* colaDeMensajes;
	colaDeMensajes = queue_create();
	Mensaje* mensaje;

	//mientras tenga algun mensaje del Server
		while(1){
			//analiza el estado actual del proceso y en base a eso, actua
			switch (state){
				case NUEVO_NIVEL:
					clientCCB_orq = connectServer("localhost", 5000/*((char*)((Direccion*)(personaje->personaje_orquestador))->IP), ((Direccion*)(personaje->personaje_orquestador))->PORT*/);

					//loggeo la conexion del personaje al orquestador
					log_info(logger, string_from_format("se conecta personaje %s a orquestador", personaje->personaje_nombre));

					//verifico si gane
					if(ganado(personaje->personaje_niveles)){

						state = WIN;
						log_info(logger, string_from_format("personaje %s cambia a estado %d", personaje->personaje_nombre, state));

					} else{

					//mando el primer mensaje al orquestador solicitando informacion del nivel y su planificador
					strcpy(proxNivel, proximoNivel(personaje->personaje_niveles));

					solicitarDataNivel();
					mandarMensaje(clientCCB_orq.sockfd,REQUEST_DATA_NIVEL,strlen(proxNivel)+1,proxNivel);

					//loggeo la solicitud de DATA_NIVEL al orquestador
					log_info(logger, string_from_format("personaje %s solicita al orquestador DATA_NIVEL del %s", personaje->personaje_nombre, proxNivel));

					state = WAIT_DATA_LEVEL;
					log_info(logger, string_from_format("personaje %s cambia a estado %d", personaje->personaje_nombre, state));
					}

					break;

				case WAIT_DATA_LEVEL:
					if (mensajes(colaDeMensajes, clientCCB_orq)){
						//agarra de la cola de mensajes un mensaje
						mensaje = queue_pop(colaDeMensajes);

						//analiza los mensajes recibidos y en base a eso, actua
						switch(mensaje->type){
							case DATANIVEL:
								//almaceno los datos recibidos en estructuras auxiliares
								strcpy(nivelActual, proxNivel);
								Data_Nivel *data_new = (Data_Nivel*) mensaje->data;
								nivelActualData = data_new->miNivel;
								planificadorActualData = data_new->miPlanificador;

								log_info(logger, string_from_format("IP: %s, PORT: %d", nivelActualData.IP, nivelActualData.PORT));

								//desconecta del orquestador y loggeo
								close(clientCCB_orq.sockfd);
								log_info(logger, string_from_format("personaje %s se desconecta del orquestador", personaje->personaje_nombre));

								//conecto a nivel y planificador y hago HANDSHAKE
								conectarNivel(nivelActualData.IP, nivelActualData.PORT);

								conectarPlanificador(planificadorActualData.IP, planificadorActualData.PORT);

								hacerHandshake();

								imprimirObjetivos(personaje->personaje_niveles);

								state = STANDBY;
								log_info(logger, string_from_format("personaje %s cambia a estado %d", personaje->personaje_nombre, state));

								break;
							case NODATANIVEL:
								//orquestador no pudo mandar la data de nivel, la vuelve a pedir
								log_info(logger, string_from_format("el personaje %s no recibio data de %s", personaje->personaje_nombre, proxNivel));
								solicitarDataNivel();
								break;
						}
						borrarMensaje(mensaje);
					}
					break;

				case STANDBY:
					if (mensajes(colaDeMensajes, clientCCB_pln)){
						mensaje = queue_pop(colaDeMensajes);

						switch(mensaje->type){
							case MOVIMIENTO_PERMITIDO:
								log_info(logger, "llego mensaje MOVIMIENTO_PERMITIDO");
								//si tengo la poscion del proximo recurso se mueve
								if(posicionProximoRecurso != NULL){
									log_info(logger,"tengo posicion de recurso que necesito");

									//realizo el movimiento y guardo nueva posicion del personaje
									nuevaPosicion = realizarMovimiento();
									posicionActual = nuevaPosicion;
									((Posicion*)(personaje->personaje_posicion_actual))->POS_X = posicionActual->POS_X;
									((Posicion*)(personaje->personaje_posicion_actual))->POS_Y = posicionActual->POS_Y;

									//loggeo de la nueva posicion del personaje
									log_info(logger, string_from_format("personaje %s nueva posicion: (%d,%d)", personaje->personaje_nombre, personaje->personaje_posicion_actual->POS_X, personaje->personaje_posicion_actual->POS_Y));

									//analiza si llego a la posicion del proximo recurso (adentro manda los mensajes)
									analizarRecurso();
								} else{
									log_info(logger,"no tengo posicion de recurso");
									//solicitar posicion del proximo recurso y loggea solicitud
									proxRecurso = proximoRecurso(personaje->personaje_niveles);

									mandarMensaje(clientCCB_niv.sockfd,REQUEST_POS_RECURSO, sizeof(proxRecurso),(char *)&proxRecurso);

									log_info(logger, string_from_format("personaje %s solicita posicion del recurso %c a %s", personaje->personaje_nombre, proxRecurso, nivelActual));

									state = WAIT_POS_REC;
									log_info(logger, string_from_format("personaje %s cambia a estado %d", personaje->personaje_nombre, state));
								}
								break;

							case MORISTE_PERSONAJE:
								//recibe mensaje de muerte por interbloqueo, muere y loggea
								morir();

								log_info(logger, string_from_format("personaje %s murio por interbloqueo en %s", personaje->personaje_nombre, nivelActual));

								break;
							}
						borrarMensaje(mensaje);
					}
					break;

				case WAIT_POS_REC:
					if (mensajes(colaDeMensajes, clientCCB_niv)){
						mensaje = queue_pop(colaDeMensajes);

						switch(mensaje->type){
							case POSICION_RECURSO:
								posicionProximoRecurso = (Posicion*) mensaje->data;
								log_info(logger, string_from_format("recibi la posicion del recurso %c: (%d,%d)", proxRecurso, posicionProximoRecurso->POS_X, posicionProximoRecurso->POS_Y));
								nuevaPosicion = realizarMovimiento();
								posicionActual = nuevaPosicion;
								((Posicion*)(personaje->personaje_posicion_actual))->POS_X = posicionActual->POS_X;
								((Posicion*)(personaje->personaje_posicion_actual))->POS_Y = posicionActual->POS_Y;

								log_info(logger, string_from_format("personaje %s nueva posicion: (%d,%d)", personaje->personaje_nombre, personaje->personaje_posicion_actual->POS_X, personaje->personaje_posicion_actual->POS_Y));

								analizarRecurso();
						}
						break;
						borrarMensaje(mensaje);
					}
					break;
				case WAIT_REC:
					if (mensajes(colaDeMensajes, clientCCB_niv)){
						mensaje = queue_pop(colaDeMensajes);

						log_info(logger, string_from_format("Recibi mensaje: %d", (int)(mensaje->type)));

						respuestaConfirmacionRecurso = *(bool*)mensaje->data;

						log_info(logger, string_from_format("Recibi respuesta: %d", (respuestaConfirmacionRecurso)));

						switch(mensaje->type){
							case CONFIRMAR_RECURSO:
								//si le otorgaron el recurso lo agrega y avisa que termino el turno
								if(respuestaConfirmacionRecurso){

									agregarRecurso(personaje->personaje_niveles);

									posicionProximoRecurso = NULL;

									//loggea la obtencion del recurso y la finalizacion del turno
									log_info(logger, string_from_format("personaje %s obtuvo recurso %c en %s", personaje->personaje_nombre, proxRecurso, nivelActual));

									log_info(logger, string_from_format("personaje %s termino turno en %s", personaje->personaje_nombre, nivelActual));

									//verifico si el personaje finalizo el nivel
									if(nivelTerminado(personaje->personaje_niveles, nivelActual)){
										mandarMensaje(clientCCB_niv.sockfd,TERMINE_NIVEL,0,NULL);
										mandarMensaje(clientCCB_pln.sockfd,TERMINE_NIVEL,0,NULL);

										//loggea la finalizacion del nivel actual
										log_info(logger, string_from_format("personaje %s termino %s", personaje->personaje_nombre, nivelActual));

										((Posicion*)(personaje->personaje_posicion_actual))->POS_X = 0;
										((Posicion*)(personaje->personaje_posicion_actual))->POS_Y = 0;

										//personaje se desconecta del nivel actual y loggea la desconexion
										close(clientCCB_niv.sockfd);
										log_info(logger, string_from_format("personaje %s se desconecta del %s", personaje->personaje_nombre, nivelActual));

										//personaje se desconecta del planificador del nivel actual y loggea la desconexion
										close(clientCCB_pln.sockfd);
										log_info(logger, string_from_format("personaje %s se desconecta del planificador de %s", personaje->personaje_nombre, nivelActual));

										state = NUEVO_NIVEL;
										log_info(logger, string_from_format("personaje %s cambia a estado %d", personaje->personaje_nombre, state));
										break;
									}else{

										//si no termino el nivel, aviso a planificador que finalizo el turno
										mandarMensaje(clientCCB_pln.sockfd,TERMINE_TURNO,0,NULL);
										state = STANDBY;
										log_info(logger, string_from_format("personaje %s cambia a estado %d", personaje->personaje_nombre, state));
										break;
									}

								} else{
									mandarMensaje(clientCCB_pln.sockfd,PERSONAJE_BLOQUEADO,sizeof(proxRecurso), (char *)&proxRecurso);

									log_info(logger, string_from_format("personaje %s se encuentra bloqueado por el recurso %c en el %s", personaje->personaje_nombre, proxRecurso, nivelActual));

									state = STANDBY;
									log_info(logger, string_from_format("personaje %s cambia a estado %d", personaje->personaje_nombre, state));
								}
								break;
							}

							borrarMensaje(mensaje);
						}
						break;

					case WIN:

						log_info(logger, string_from_format("personaje %s gano su plan de niveles!",personaje->personaje_nombre));

						//aviso al orquestador que gane
						mandarMensaje(clientCCB_orq.sockfd,GANE,sizeof(personaje->personaje_simbolo),(char *)&personaje->personaje_simbolo);

						log_info(logger, string_from_format("personaje %s informo al orquestador que gano", personaje->personaje_nombre));
						log_destroy(logger);

						return 1;
						break;
					}
			}

	return 1;
}


int _is_next_level(t_personaje_nivel *p){
	return !(p->termino_nivel);
}

char* proximoNivel(t_list *niveles){
	t_personaje_nivel *auxNivel = list_find(niveles, (void*) _is_next_level);
	return auxNivel->personaje_nivel;
}

char* transformNivel_to_send(char *nivel, char **miNivAux){
	strcpy(*miNivAux, string_from_format("N%c", nivel[(strlen(nivel)-1)]));
	return *miNivAux;
}

int _is_next_obj(t_personaje_objetivo *o){
	return !(o->tiene_objetivo);
}

char proximoRecurso(t_list *niveles){
	t_personaje_nivel *auxNivel = list_find(niveles, (void*) _is_next_level);
	t_personaje_objetivo *auxObjetivo = list_find(auxNivel->personaje_objetivos, (void*) _is_next_obj);
	log_info(logger, string_from_format("proximo recurso: %c", auxObjetivo->objetivo));

	return auxObjetivo->objetivo;
}

Posicion *proximaPosicion(){
	Posicion *pos = posicionActual;
	if((pos->POS_X < posicionProximoRecurso->POS_X) && (pos->POS_X != posicionProximoRecurso->POS_X)){
		pos->POS_X ++;
		return pos;
	} else if((pos->POS_X > posicionProximoRecurso->POS_X) && (pos->POS_X != posicionProximoRecurso->POS_X)){
		pos->POS_X --;
		return pos;
	} else if((pos->POS_Y > posicionProximoRecurso->POS_Y) && (pos->POS_Y != posicionProximoRecurso->POS_Y)){
		pos->POS_Y --;
		return pos;
	} else if((pos->POS_Y < posicionProximoRecurso->POS_Y) && (pos->POS_Y != posicionProximoRecurso->POS_Y)){
		posicionActual->POS_Y ++;
		return posicionActual;
	}
	return posicionActual;
}

void reiniciarNivel(t_list *niveles){
	int i = 0;
	int j = 0;
	t_personaje_objetivo *auxObjetivo;
	t_personaje_nivel *auxNivel = list_get(niveles, 0);
	while((!(string_equals_ignore_case(nivelActual, auxNivel->personaje_nivel)))){
		auxNivel = list_get(niveles,j);
		j++;
	}
	int lenObjetivos = list_size(auxNivel->personaje_objetivos);
	while(i<lenObjetivos){
		auxObjetivo = list_get(auxNivel->personaje_objetivos,i);
		auxObjetivo->tiene_objetivo = false;
		i++;
	}
	auxNivel->termino_nivel = false;

	personaje->personaje_posicion_actual->POS_X = 0;
	personaje->personaje_posicion_actual->POS_Y = 0;
}

bool recursoAlcanzado(Posicion *pos1, Posicion *pos2){
	return ((pos1->POS_X==pos2->POS_X)&&(pos1->POS_Y==pos2->POS_Y));
}


Posicion *realizarMovimiento(){
	Posicion *nuevaPos = proximaPosicion();

	mandarMensaje(clientCCB_niv.sockfd,REQUEST_MOVIMIENTO,sizeof(Posicion), nuevaPos);
	
	log_info(logger, string_from_format("personaje %s solicita movimiento a %s a la posicion (%d, %d)", personaje->personaje_nombre, nivelActual, nuevaPos->POS_X, nuevaPos->POS_Y));

	return nuevaPos;
}

void analizarRecurso(){
	if(recursoAlcanzado(posicionActual, posicionProximoRecurso)){
		int res;
		res = mandarMensaje(clientCCB_niv.sockfd ,REQUEST_RECURSO,sizeof(proxRecurso), &proxRecurso);
		log_info(logger, string_from_format("Pedido enviado recurso %c con resultado %d, al filedesc %d", proxRecurso, res, clientCCB_niv.sockfd));
		state = WAIT_REC;
	} else{
		log_info(logger, "no llegue al recurso");
		
		mandarMensaje(clientCCB_pln.sockfd,TERMINE_TURNO,0,NULL);

		log_info(logger, string_from_format("personaje %s termino turno en %s", personaje->personaje_nombre, nivelActual));

		state = STANDBY;
	}
}

void agregarRecurso(t_list *niveles){
	t_personaje_nivel *auxNivel = list_find(niveles, (void*) _is_next_level);
	int i = 0;
	t_list *auxListObjetivos = auxNivel->personaje_objetivos;
	int len = list_size(auxListObjetivos);
	t_personaje_objetivo *auxObjetivo;
	while(i<len){
		auxObjetivo = list_get(auxListObjetivos, i);
		if(auxObjetivo->objetivo == proxRecurso && (!(auxObjetivo->tiene_objetivo))){
			auxObjetivo->tiene_objetivo = true;
			break;
		} else{
			i++;
		}
	}
}

bool nivelTerminado(t_list *niveles, char *nivActual){
	int i = 0;
	int j = 0;
	int lenNiveles = list_size(niveles);
	t_personaje_objetivo *auxObjetivo;
	t_personaje_nivel *auxNivel = list_get(niveles,0);
	while((j<lenNiveles) && (!(string_equals_ignore_case(nivActual, auxNivel->personaje_nivel)))){
		auxNivel = list_get(niveles,j);
		j++;
	}
	t_list *auxListObjetivos = auxNivel->personaje_objetivos;
	int lenObjetivos = list_size(auxListObjetivos);
	while(i<lenObjetivos){
		auxObjetivo = list_get(auxListObjetivos, i);
		if(auxObjetivo->tiene_objetivo){
			i++;
		} else{
			return false;
		}
	}
	auxNivel->termino_nivel = true;
	list_replace(niveles, i, auxNivel);
	return true;
}

bool ganado(t_list *niveles){
	int i = 0;
	t_personaje_nivel *auxNivel;
	int len = list_size(niveles);
	while(i<len){
		auxNivel = list_get(niveles, i);
		if(nivelTerminado(niveles, auxNivel->personaje_nivel)){
			i++;
		} else{
			return false;
		}
	}
	return true;
}

void morir(){

	mandarMensaje(clientCCB_niv.sockfd,TERMINE_NIVEL,0,NULL);
	mandarMensaje(clientCCB_pln.sockfd,TERMINE_NIVEL,0,NULL);
	personaje->personaje_vidas_restantes --;
	log_info(logger, string_from_format("vidas restantes del personaje %s: %d", personaje->personaje_nombre, personaje->personaje_vidas_restantes));

	//desconecta del nivel y del planificador
	close(clientCCB_niv.sockfd);
	close(clientCCB_pln.sockfd);
	log_info(logger, string_from_format("personaje %s se desconecto del %s y de su planificador", personaje->personaje_nombre, nivelActual));

	//si no le quedan vidas, reinicia todo de nuevo -> personaje apunta al personaje inicial
	if(personaje->personaje_vidas_restantes == 0){
		//reinicio el personaje
		inicializarPersonaje();
	} else{

		reiniciarNivel(personaje->personaje_niveles);

		posicionProximoRecurso = NULL;

		//conecto a nivel y planificador y hago HANDSHAKE
		conectarNivel(nivelActualData.IP, nivelActualData.PORT);

		conectarPlanificador(planificadorActualData.IP, planificadorActualData.PORT);

		hacerHandshake();

		log_info(logger, string_from_format("personaje %s reinicio el %s", personaje->personaje_nombre, nivelActual));

		state = STANDBY;
	}
}

void rutinaSignal(int n){
	switch (n) {
		case SIGTERM:
			log_info(logger, string_from_format("personaje %s murio por SIGTERM", personaje->personaje_nombre));
			morir();

		break;
		case SIGUSR1:
			personaje->personaje_vidas_restantes ++;
			log_info(logger, string_from_format("personaje %s recibio SIGUSR1, vidas actuales: %d", personaje->personaje_nombre, personaje->personaje_vidas_restantes));
		break;
	}
}



/*NAME: read_personaje_archivo_configuracion
PARAM: char* path -> direccion del archivo de configuracion
RETURN: t_personaje * -> un personaje creado en base a un archivo de configuracion
DESC: instancia un t_config (struct de commons/config.h) tomando valores del archivo de configuracion y devuelve
	un personaje creado en base a este t_config
	*/
t_personaje *read_personaje_archivo_configuracion(char* path){

	t_personaje *personaje;
	t_config * p;

	p = config_create(path);
	if (p == NULL) {
		return NULL;
	}

	personaje = create_personaje(p);

	return personaje;
}

/*NAME: create_personaje
PARAM: t_config *n -> una instancia de t_config con los valores de un archivo de ocnfiguracion
RETURN: t_personaje * -> el personaje creado
DESC: con las funciones de commons/config.h va tomando los valores del t_config dependiendo de la key pasada
	como parametro a cada funci?n
	*/

t_personaje *create_personaje(t_config *p){
	t_personaje *personaje;
	char *aux = config_get_string_value(p, "simbolo");

	personaje = (t_personaje*)malloc(sizeof(t_personaje));
	personaje->personaje_orquestador = (Direccion *)malloc(sizeof(Direccion));
	personaje->personaje_posicion_actual = (Posicion *)malloc(sizeof(Posicion));

	personaje->personaje_nombre = config_get_string_value(p, "nombre");

	personaje->personaje_simbolo = aux[0];

	personaje->personaje_niveles = create_lista_niveles(personaje, p);

	personaje->personaje_vidas = config_get_int_value(p, "vidas");

	personaje->personaje_vidas_restantes = personaje->personaje_vidas;

	strcpy(personaje->personaje_orquestador->IP, tomarIP(config_get_string_value(p, "orquestador")));

	personaje->personaje_orquestador->PORT = tomarPuerto(config_get_string_value(p, "orquestador"));

	personaje->personaje_posicion_actual->POS_X = 0;

	personaje->personaje_posicion_actual->POS_Y = 0;

	return personaje;
}

/*NAME: create_personaje_nivel
PARAM: char *nivel, char **objetivos -> un nivel y un array de strings que contienen los objetivos
	de un nivel
RETURN: t_personaje_nivel -> devuelve una instancia de t_personaje_nivel con los valores tomados del t_config utilizando la key
pasada como argumento
DESC: crea una nueva instancia de t_personaje_nivel
	*/
t_personaje_nivel *create_personaje_nivel(char *nivel, char **objetivos){
	t_personaje_nivel *new = malloc(sizeof(t_personaje_nivel));
	t_list *list_objetivos = list_create();
	int i = 0;

	new->personaje_nivel = nivel;

	while(objetivos[i] != NULL){

		add_list_nivel_objetivos(objetivos[i], list_objetivos);
		i++;
	}

	new->personaje_objetivos = list_objetivos;

	new->termino_nivel = false;

	new->sig = NULL;

	return new;
}

void add_list_nivel_objetivos(char *objetivo, t_list *list_objetivos){

	char objetivo_char = objetivo[0];

	list_add(list_objetivos, create_nivel_objetivo(objetivo_char));
}

t_personaje_objetivo *create_nivel_objetivo(char objetivo){
	t_personaje_objetivo *new = malloc(sizeof(t_personaje_objetivo));

	new->objetivo = objetivo;

	new->tiene_objetivo = false;

	new->sig = NULL;

	return new;
}

void add_list_personaje_niveles(char **arr, char *buffer_nivel, t_list *list){

		list_add(list, create_personaje_nivel(buffer_nivel, arr));
}

t_list *create_lista_niveles(t_personaje *personaje, t_config *p){

	t_list *list = list_create();
	char **aux_niveles = config_get_array_value(p, "planDeNiveles");
	int i = 0;

	char *key_obj = malloc(sizeof(char)*((strlen((aux_niveles[i])))+(sizeof(char)*6)));

	while((aux_niveles)[i] != NULL){

		strcpy(key_obj, string_from_format("obj[%s]", (aux_niveles)[i])); //key almacenada en key_obj

		add_list_personaje_niveles(config_get_array_value(p, key_obj), (aux_niveles)[i], list);

		i++;
	}

	free(key_obj);

	return list;
}

void imprimirObjetivos(t_list *niveles){
	int i = 0;
	int j = 0;
	int lenNiv = list_size(niveles);
	t_personaje_objetivo *auxObj;
	t_personaje_nivel *auxNiv = list_get(niveles,0);
	while((j<lenNiv) && (!(string_equals_ignore_case(nivelActual, auxNiv->personaje_nivel)))){
		auxNiv = list_get(niveles,j);
		j++;
	}
	t_list *auxListObj = auxNiv->personaje_objetivos;
	int lenObj = list_size(auxListObj);
	while(i<lenObj){
		auxObj = list_get(auxListObj, i);
		log_info(logger, string_from_format("recurso del nivel: %c", auxObj->objetivo));
		i++;
	}
}

int conectarNivel(char *IP, int PORT){
	//conecto al nivel actual y loggeo
	clientCCB_niv = connectServer(IP, PORT);
	log_info(logger, string_from_format("personaje %s se conecta a %s", personaje->personaje_nombre, nivelActual));
	return 1;
}

int conectarPlanificador(char *IP, int PORT){
	//conecto al planificador del nivel actual y loggeo
	clientCCB_pln = connectServer(IP, PORT);
	log_info(logger, string_from_format("personaje %s se conecta a planificador de %s", personaje->personaje_nombre, nivelActual));
	return 1;
}

Personaje *hacerHandshake(){
	Personaje *personajeSend = malloc(sizeof(Personaje));
	strcpy(personajeSend->ID,string_from_format("P%c",personaje->personaje_simbolo));

	mandarMensaje(clientCCB_niv.sockfd, HANDSHAKE,sizeof(personajeSend),personajeSend);
	log_info(logger,"mande HANDSHAKE a %s", nivelActual);


	mandarMensaje(clientCCB_pln.sockfd, HANDSHAKE,sizeof(personajeSend),personajeSend);
	log_info(logger,"mande HANDSHAKE a planificador");

	return personajeSend;
}

void inicializarPersonaje(){
	personaje = read_personaje_archivo_configuracion(archivo_config);
	if (personaje == NULL) {
		fprintf(stderr, "No se pudo leer el archivo de configuracion %s\n", archivo_config);
		exit(1);
	}

	strcpy(nivelActual,"");
	posicionActual = (Posicion*)personaje->personaje_posicion_actual;
	state = NUEVO_NIVEL;
}

void solicitarDataNivel(){

	mandarMensaje(clientCCB_orq.sockfd,REQUEST_DATA_NIVEL,strlen(proxNivel)+1,proxNivel);

	//loggeo la solicitud de DATA_NIVEL al orquestador
	log_info(logger, string_from_format("personaje %s solicita al orquestador DATA_NIVEL del %s", personaje->personaje_nombre, proxNivel));

	state = WAIT_DATA_LEVEL;
	log_info(logger, string_from_format("personaje %s cambia a estado %d", personaje->personaje_nombre, state));
}
