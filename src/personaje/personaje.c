#include "personaje.h"

t_personaje *personaje;

//variables globales
char miEstado;
Posicion *posicionProximoRecurso;
char proxRecurso;
char nombreProximoNivel[20];
char nombreNivelActual[20];
Posicion *nuevaPosicion;
Posicion *posicionActual;
bool respuestaConfirmacionRecurso;
char *pathArchivoConfig;
char flag;

Nivel nivelActualData;
Planificador planificadorActualData;

t_log* logger;

CCB orquestadorCCB;
CCB nivelCCB;
CCB planificadorCCB;

int main(int argc, char *argv[]) {

	if (argc < 2) {
		fprintf(stderr, "Faltan argumentos. personaje archivoconfig\n");
		exit(1);
	}

	pathArchivoConfig = argv[1];

	//inicializo el personaje desde el archivo config
	inicializarPersonaje();

	//instancio el logger
	logger = log_create(string_from_format("ProcesoPersonaje%s.log", personaje->nombre), "ProcesoPersonaje", true,
			LOG_LEVEL_INFO);

	//declaro las seniales
	signal(SIGTERM, rutinaSignal);
	signal(SIGUSR1, rutinaSignal);

	//inicializo la cola de mensajes
	t_queue* colaDeMensajes;
	colaDeMensajes = queue_create();
	Mensaje* mensaje;

	//mientras tenga algun mensaje del Server
	while (1) {
		//analiza el estado actual del proceso y en base a eso, actua
		switch (miEstado) {
		case NUEVO_NIVEL:
			log_info(logger, "intentando");
			orquestadorCCB =
					connectServer("localhost",
							5000/*((char*)((Direccion*)(personaje->personaje_orquestador))->IP), ((Direccion*)(personaje->personaje_orquestador))->PORT*/);

			//loggeo la conexion del personaje al orquestador
			log_info(logger,
					string_from_format("se conecta personaje %s a orquestador",
							personaje->nombre));

			//verifico si gane
			if (ganado(personaje->niveles)) {

				miEstado = WIN;
				log_info(logger,
						string_from_format("personaje %s cambia a estado %d",
								personaje->nombre, miEstado));

			} else {

				//mando el primer mensaje al orquestador solicitando informacion del nivel y su planificador
				strcpy(nombreProximoNivel, proximoNivel(personaje->niveles));

				solicitarDataNivel();
				mandarMensaje(orquestadorCCB.sockfd, REQUEST_DATA_NIVEL,
						strlen(nombreProximoNivel) + 1, nombreProximoNivel);

				//loggeo la solicitud de DATA_NIVEL al orquestador
				log_info(logger,
						string_from_format(
								"personaje %s solicita al orquestador DATA_NIVEL del %s",
								personaje->nombre, nombreProximoNivel));

				miEstado = WAIT_DATA_LEVEL;
				log_info(logger,
						string_from_format("personaje %s cambia a estado %d",
								personaje->nombre, miEstado));
			}

			break;

		case WAIT_DATA_LEVEL:
			if (mensajes(colaDeMensajes, orquestadorCCB)) {
				//agarra de la cola de mensajes un mensaje
				mensaje = queue_pop(colaDeMensajes);

				//analiza los mensajes recibidos y en base a eso, actua
				switch (mensaje->type) {
				case DATANIVEL:
					//almaceno los datos recibidos en estructuras auxiliares
					strcpy(nombreNivelActual, nombreProximoNivel);
					Data_Nivel *data_new = (Data_Nivel*) mensaje->data;
					nivelActualData = data_new->miNivel;
					planificadorActualData = data_new->miPlanificador;

					log_info(logger,
							string_from_format("IP: %s, PORT: %d",
									nivelActualData.IP, nivelActualData.PORT));

					//desconecta del orquestador y loggeo
					close(orquestadorCCB.sockfd);
					log_info(logger,
							string_from_format(
									"personaje %s se desconecta del orquestador",
									personaje->nombre));

					//conecto a nivel y planificador y hago HANDSHAKE
					conectarNivel(nivelActualData.IP, nivelActualData.PORT);

					conectarPlanificador(planificadorActualData.IP,
							planificadorActualData.PORT);

					hacerHandshake();

					imprimirObjetivos(personaje->niveles);

					miEstado = STANDBY;
					log_info(logger,
							string_from_format(
									"personaje %s cambia a estado %d",
									personaje->nombre, miEstado));

					break;
				case NODATANIVEL:
					sleep(5);
					//orquestador no pudo mandar la data de nivel, la vuelve a pedir
					log_info(logger,
							string_from_format(
									"el personaje %s no recibio data de %s",
									personaje->nombre, nombreProximoNivel));
					solicitarDataNivel();
					break;
				}
				borrarMensaje(mensaje);
			}
			break;

		case STANDBY:
			if (mensajes(colaDeMensajes, planificadorCCB)) {
				mensaje = queue_pop(colaDeMensajes);

				switch (mensaje->type) {
				case MOVIMIENTO_PERMITIDO:
					log_trace(logger, "llego mensaje MOVIMIENTO_PERMITIDO");
					//si tengo la poscion del proximo recurso se mueve
					if(flag != 0){
						flag = 0;
						llegoRecurso();
					}
						//verifico si el personaje finalizo el nivel
					if (nivelTerminado(personaje->niveles,
							nombreNivelActual)) {
						break;
					}								
								
					if (posicionProximoRecurso != NULL ) {
						log_trace(logger,
								"tengo posicion de recurso que necesito");

						//realizo el movimiento y guardo nueva posicion del personaje
						nuevaPosicion = realizarMovimiento();
						posicionActual = nuevaPosicion;
						((Posicion*) (personaje->posActual))->POS_X =
								posicionActual->POS_X;
						((Posicion*) (personaje->posActual))->POS_Y =
								posicionActual->POS_Y;

						//loggeo de la nueva posicion del personaje
						log_info(logger,
								string_from_format(
										"personaje %s nueva posicion: (%d,%d)",
										personaje->nombre,
										personaje->posActual->POS_X,
										personaje->posActual->POS_Y));

						//analiza si llego a la posicion del proximo recurso (adentro manda los mensajes)
						analizarRecurso();
					} else {
						log_trace(logger, "no tengo posicion de recurso");
						//solicitar posicion del proximo recurso y loggea solicitud
						proxRecurso = proximoRecurso(
								personaje->niveles);

						mandarMensaje(nivelCCB.sockfd, REQUEST_POS_RECURSO,
								sizeof(proxRecurso), (char *) &proxRecurso);

						log_trace(logger,
								string_from_format(
										"personaje %s solicita posicion del recurso %c a %s",
										personaje->nombre,
										proxRecurso, nombreNivelActual));

						miEstado = WAIT_POS_REC;
						log_info(logger,
								string_from_format(
										"personaje %s cambia a estado %d",
										personaje->nombre, miEstado));
						break;
					}
					break;

				case MORISTE_PERSONAJE:
					//recibe mensaje de muerte por interbloqueo, muere y loggea
					morir();

					log_info(logger,
							string_from_format(
									"personaje %s murio por interbloqueo en %s",
									personaje->nombre, nombreNivelActual));

					break;
				}
				borrarMensaje(mensaje);
			}
			break;

		case WAIT_POS_REC:
			if (mensajes(colaDeMensajes, nivelCCB)) {
				mensaje = queue_pop(colaDeMensajes);

				switch (mensaje->type) {
				case POSICION_RECURSO:
					posicionProximoRecurso = (Posicion*) mensaje->data;
					log_trace(logger,
							string_from_format(
									"recibi la posicion del recurso %c: (%d,%d)",
									proxRecurso, posicionProximoRecurso->POS_X,
									posicionProximoRecurso->POS_Y));
					nuevaPosicion = realizarMovimiento();
					posicionActual = nuevaPosicion;
					((Posicion*) (personaje->posActual))->POS_X =
							posicionActual->POS_X;
					((Posicion*) (personaje->posActual))->POS_Y =
							posicionActual->POS_Y;

					log_info(logger,
							string_from_format(
									"personaje %s nueva posicion: (%d,%d)",
									personaje->nombre,
									personaje->posActual->POS_X,
									personaje->posActual->POS_Y));

					analizarRecurso();
				}
				break;
				borrarMensaje(mensaje);
			}
			break;
		case WAIT_REC:
			if (mensajes(colaDeMensajes, nivelCCB)) {
				mensaje = queue_pop(colaDeMensajes);

				log_trace(logger,
						string_from_format("Recibi mensaje: %d",
								(int) (mensaje->type)));

				respuestaConfirmacionRecurso = *(bool*) mensaje->data;

				log_trace(logger,
						string_from_format("Recibi respuesta: %d",
								(respuestaConfirmacionRecurso)));

				switch (mensaje->type) {
				case CONFIRMAR_RECURSO:
					//si le otorgaron el recurso lo agrega y avisa que termino el turno
					if (respuestaConfirmacionRecurso) {

						llegoRecurso();

					} else {
						mandarMensaje(planificadorCCB.sockfd, PERSONAJE_BLOQUEADO,
								sizeof(proxRecurso), (char *) &proxRecurso);

						flag = 1;

						log_info(logger,
								string_from_format(
										"personaje %s se encuentra bloqueado por el recurso %c en el %s",
										personaje->nombre,
										proxRecurso, nombreNivelActual));

						miEstado = STANDBY;
						log_info(logger,
								string_from_format(
										"personaje %s cambia a estado %d",
										personaje->nombre, miEstado));
					}
					break;
				}

				borrarMensaje(mensaje);
			}
			break;

		case WIN:

			log_info(logger,
					string_from_format("personaje %s gano su plan de niveles!",
							personaje->nombre));

			Personaje *personajeSend = malloc(sizeof(Personaje));
			strcpy(personajeSend->ID,
					string_from_format("P%c", personaje->simbolo));

			//aviso al orquestador que gane
			mandarMensaje(orquestadorCCB.sockfd, GANE,
					sizeof(personajeSend), personajeSend);

			log_info(logger,
					string_from_format(
							"personaje %s informo al orquestador que gano",
							personaje->nombre));
			log_destroy(logger);

			return 1;
			break;
		}
	}

	return 1;
}

int _is_next_level(t_personaje_nivel *p) {
	return !(p->terminoNivel);
}

char* proximoNivel(t_list *niveles) {
	t_personaje_nivel *auxNivel = list_find(niveles, (void*) _is_next_level);
	return auxNivel->nivel;
}

char* transformNivel_to_send(char *nivel, char **miNivAux) {
	strcpy(*miNivAux, string_from_format("N%c", nivel[(strlen(nivel) - 1)]));
	return *miNivAux;
}

int _is_next_obj(t_personaje_objetivo *o) {
	return !(o->tieneObjetivo);
}

char proximoRecurso(t_list *niveles) {
	t_personaje_nivel *auxNivel = list_find(niveles, (void*) _is_next_level);
	t_personaje_objetivo *auxObjetivo = list_find(auxNivel->objetivos,
			(void*) _is_next_obj);
	log_info(logger,
			string_from_format("proximo recurso: %c", auxObjetivo->objetivo));

	return auxObjetivo->objetivo;
}

Posicion *proximaPosicion() {
	Posicion *pos = posicionActual;
	if ((pos->POS_X < posicionProximoRecurso->POS_X)
			&& (pos->POS_X != posicionProximoRecurso->POS_X)) {
		pos->POS_X++;
		return pos;
	} else if ((pos->POS_X > posicionProximoRecurso->POS_X)
			&& (pos->POS_X != posicionProximoRecurso->POS_X)) {
		pos->POS_X--;
		return pos;
	} else if ((pos->POS_Y > posicionProximoRecurso->POS_Y)
			&& (pos->POS_Y != posicionProximoRecurso->POS_Y)) {
		pos->POS_Y--;
		return pos;
	} else if ((pos->POS_Y < posicionProximoRecurso->POS_Y)
			&& (pos->POS_Y != posicionProximoRecurso->POS_Y)) {
		posicionActual->POS_Y++;
		return posicionActual;
	}
	return posicionActual;
}

void reiniciarNivel(t_list *niveles) {
	int i = 0;
	int j = 0;
	t_personaje_objetivo *auxObjetivo;
	t_personaje_nivel *auxNivel = list_get(niveles, 0);
	while ((!(string_equals_ignore_case(nombreNivelActual, auxNivel->nivel)))) {
		auxNivel = list_get(niveles, j);
		j++;
	}
	int lenObjetivos = list_size(auxNivel->objetivos);
	while (i < lenObjetivos) {
		auxObjetivo = list_get(auxNivel->objetivos, i);
		auxObjetivo->tieneObjetivo = false;
		i++;
	}
	auxNivel->terminoNivel = false;

	personaje->posActual->POS_X = 0;
	personaje->posActual->POS_Y = 0;
}

bool recursoAlcanzado(Posicion *pos1, Posicion *pos2) {
	return ((pos1->POS_X == pos2->POS_X) && (pos1->POS_Y == pos2->POS_Y));
}

Posicion *realizarMovimiento() {
	Posicion *nuevaPos = proximaPosicion();

	mandarMensaje(nivelCCB.sockfd, REQUEST_MOVIMIENTO, sizeof(Posicion),
			nuevaPos);

	log_info(logger,
			string_from_format(
					"personaje %s solicita movimiento a %s a la posicion (%d, %d)",
					personaje->nombre, nombreNivelActual, nuevaPos->POS_X,
					nuevaPos->POS_Y));

	return nuevaPos;
}

void analizarRecurso() {
	if (recursoAlcanzado(posicionActual, posicionProximoRecurso)) {
		int res;
		res = mandarMensaje(nivelCCB.sockfd, REQUEST_RECURSO,
				sizeof(proxRecurso), &proxRecurso);
		log_trace(logger,
				string_from_format(
						"Pedido enviado recurso %c con resultado %d, al filedesc %d",
						proxRecurso, res, nivelCCB.sockfd));
		miEstado = WAIT_REC;
	} else {
		log_trace(logger, "no llegue al recurso");

		mandarMensaje(planificadorCCB.sockfd, TERMINE_TURNO, 0, NULL );

		log_trace(logger,
				string_from_format("personaje %s termino turno en %s",
						personaje->nombre, nombreNivelActual));

		miEstado = STANDBY;
	}
}

void agregarRecurso(t_list *niveles) {
	t_personaje_nivel *auxNivel = list_find(niveles, (void*) _is_next_level);
	int i = 0;
	t_list *auxListObjetivos = auxNivel->objetivos;
	int len = list_size(auxListObjetivos);
	t_personaje_objetivo *auxObjetivo;
	while (i < len) {
		auxObjetivo = list_get(auxListObjetivos, i);
		if (auxObjetivo->objetivo == proxRecurso
				&& (!(auxObjetivo->tieneObjetivo))) {
			auxObjetivo->tieneObjetivo = true;
			break;
		} else {
			i++;
		}
	}
}

bool nivelTerminado(t_list *niveles, char *nivActual) {
	int i = 0;
	int j = 0;
	int lenNiveles = list_size(niveles);
	t_personaje_objetivo *auxObjetivo;
	t_personaje_nivel *auxNivel = list_get(niveles, 0);
	while ((j < lenNiveles)
			&& (!(string_equals_ignore_case(nivActual,
					auxNivel->nivel)))) {
		auxNivel = list_get(niveles, j);
		j++;
	}
	t_list *auxListObjetivos = auxNivel->objetivos;
	int lenObjetivos = list_size(auxListObjetivos);
	while (i < lenObjetivos) {
		auxObjetivo = list_get(auxListObjetivos, i);
		if (auxObjetivo->tieneObjetivo) {
			i++;
		} else {
			return false;
		}
	}
	auxNivel->terminoNivel = true;
	list_replace(niveles, i, auxNivel);
	return true;
}

bool ganado(t_list *niveles) {
	int i = 0;
	t_personaje_nivel *auxNivel;
	int len = list_size(niveles);
	while (i < len) {
		auxNivel = list_get(niveles, i);
		if (nivelTerminado(niveles, auxNivel->nivel)) {
			i++;
		} else {
			return false;
		}
	}
	return true;
}

void morir() {

	mandarMensaje(nivelCCB.sockfd, TERMINE_NIVEL, 0, NULL );
	mandarMensaje(planificadorCCB.sockfd, TERMINE_NIVEL, 0, NULL );
	personaje->vidasRestantes--;
	log_info(logger,
			string_from_format("vidas restantes del personaje %s: %d",
					personaje->nombre,
					personaje->vidasRestantes));

	//desconecta del nivel y del planificador
	close(nivelCCB.sockfd);
	close(planificadorCCB.sockfd);
	log_debug(logger,
			string_from_format(
					"personaje %s se desconecto del %s y de su planificador",
					personaje->nombre, nombreNivelActual));

	//si no le quedan vidas, reinicia todo de nuevo -> personaje apunta al personaje inicial
	if (personaje->vidasRestantes == 0) {
		//reinicio el personaje
		inicializarPersonaje();
	} else {

		reiniciarNivel(personaje->niveles);

		posicionProximoRecurso = NULL;
		flag = 0;

		//conecto a nivel y planificador y hago HANDSHAKE
		conectarNivel(nivelActualData.IP, nivelActualData.PORT);

		conectarPlanificador(planificadorActualData.IP,
				planificadorActualData.PORT);

		hacerHandshake();

		log_info(logger,
				string_from_format("personaje %s reinicio el %s",
						personaje->nombre, nombreNivelActual));

		miEstado = STANDBY;
	}
}

void rutinaSignal(int n) {
	switch (n) {
	case SIGTERM:
		log_info(logger,
				string_from_format("personaje %s murio por SIGTERM",
						personaje->nombre));
		morir();

		break;
	case SIGUSR1:
		personaje->vidasRestantes++;
		log_info(logger,
				string_from_format(
						"personaje %s recibio SIGUSR1, vidas actuales: %d",
						personaje->nombre,
						personaje->vidasRestantes));
		break;
	}
}

/*NAME: read_personaje_archivo_configuracion
 PARAM: char* path -> direccion del archivo de configuracion
 RETURN: t_personaje * -> un personaje creado en base a un archivo de configuracion
 DESC: instancia un t_config (struct de commons/config.h) tomando valores del archivo de configuracion y devuelve
 un personaje creado en base a este t_config
 */
t_personaje *read_personaje_archivo_configuracion(char* path) {

	t_personaje *personaje;
	t_config * p;

	p = config_create(path);
	if (p == NULL ) {
		return NULL ;
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

t_personaje *create_personaje(t_config *p) {
	t_personaje *personaje;
	//char *aux = config_get_string_value(p, "simbolo");

	personaje = (t_personaje*) malloc(sizeof(t_personaje));
	personaje->orquestador = (Direccion *) malloc(sizeof(Direccion));
	personaje->posActual = (Posicion *) malloc(
			sizeof(Posicion));

	personaje->nombre = config_get_string_value(p, "nombre");

	//personaje->simbolo = aux[0];

	personaje->simbolo = (config_get_string_value(p, "simbolo"))[0];

	personaje->niveles = create_lista_niveles(personaje, p);

	personaje->vidas = config_get_int_value(p, "vidas");

	personaje->vidasRestantes = personaje->vidas;

	strcpy(personaje->orquestador->IP,
			tomarIP(config_get_string_value(p, "orquestador")));

	personaje->orquestador->PORT = tomarPuerto(
			config_get_string_value(p, "orquestador"));
/*
	strcpy(personaje->miDireccion->IP,
			tomarIP(config_get_string_value(p, "miDireccion")));

	personaje->miDireccion->PORT = tomarPuerto(
			config_get_string_value(p, "miDireccion"));*/

	personaje->posActual->POS_X = 1;

	personaje->posActual->POS_Y = 1;

	return personaje;
}

/*NAME: create_personaje_nivel
 PARAM: char *nivel, char **objetivos -> un nivel y un array de strings que contienen los objetivos
 de un nivel
 RETURN: t_personaje_nivel -> devuelve una instancia de t_personaje_nivel con los valores tomados del t_config utilizando la key
 pasada como argumento
 DESC: crea una nueva instancia de t_personaje_nivel
 */
t_personaje_nivel *create_personaje_nivel(char *nombreNivel, char **nivelesObjetivo) {
	t_personaje_nivel *new = malloc(sizeof(t_personaje_nivel));
	t_list *list_objetivos = list_create();
	int i = 0;

	new->nivel = nombreNivel;

	while (nivelesObjetivo[i] != NULL ) {

		add_list_nivel_objetivos(nivelesObjetivo[i], list_objetivos);
		i++;
	}

	new->objetivos = list_objetivos;

	new->terminoNivel = false;

	new->sig = NULL;

	return new;
}

void add_list_nivel_objetivos(char *objetivo, t_list *list_objetivos) {

	char objetivo_char = objetivo[0];

	list_add(list_objetivos, create_nivel_objetivo(objetivo_char));
}

t_personaje_objetivo *create_nivel_objetivo(char objetivo) {
	t_personaje_objetivo *new = malloc(sizeof(t_personaje_objetivo));

	new->objetivo = objetivo;

	new->tieneObjetivo = false;

	new->sig = NULL;

	return new;
}

void add_list_personaje_niveles(char **arr, char *buffer_nivel, t_list *list) {

	list_add(list, create_personaje_nivel(buffer_nivel, arr));
}

t_list *create_lista_niveles(t_personaje *personaje, t_config *p) {

	t_list *list = list_create();
	char **aux_niveles = config_get_array_value(p, "planDeNiveles");
	int i = 0;

	char *key_obj = malloc(
			sizeof(char) * ((strlen((aux_niveles[i]))) + (sizeof(char) * 6)));

	while ((aux_niveles)[i] != NULL ) {

		strcpy(key_obj, string_from_format("obj[%s]", (aux_niveles)[i])); //key almacenada en key_obj

		add_list_personaje_niveles(config_get_array_value(p, key_obj),
				(aux_niveles)[i], list);

		i++;
	}

	free(key_obj);

	return list;
}

void imprimirObjetivos(t_list *niveles) {
	int i = 0;
	int j = 0;
	int lenNiv = list_size(niveles);
	t_personaje_objetivo *auxObj;
	t_personaje_nivel *auxNiv = list_get(niveles, 0);
	while ((j < lenNiv)
			&& (!(string_equals_ignore_case(nombreNivelActual,
					auxNiv->nivel)))) {
		auxNiv = list_get(niveles, j);
		j++;
	}
	t_list *auxListObj = auxNiv->objetivos;
	int lenObj = list_size(auxListObj);
	while (i < lenObj) {
		auxObj = list_get(auxListObj, i);
		log_info(logger,
				string_from_format("recurso del nivel: %c", auxObj->objetivo));
		i++;
	}
}

int conectarNivel(char *IP, int PORT) {
	//conecto al nivel actual y loggeo
	nivelCCB = connectServer(IP, PORT);
	log_info(logger,
			string_from_format("personaje %s se conecta a %s",
					personaje->nombre, nombreNivelActual));
	return 1;
}

int conectarPlanificador(char *IP, int PORT) {
	//conecto al planificador del nivel actual y loggeo
	planificadorCCB = connectServer(IP, PORT);
	log_info(logger,
			string_from_format("personaje %s se conecta a planificador de %s",
					personaje->nombre, nombreNivelActual));
	return 1;
}

Personaje *hacerHandshake() {
	Personaje *personajeSend = malloc(sizeof(Personaje));
	strcpy(personajeSend->ID,
			string_from_format("P%c", personaje->simbolo));

	mandarMensaje(nivelCCB.sockfd, HANDSHAKE, sizeof(personajeSend),
			personajeSend);
	log_info(logger, "mande HANDSHAKE a %s", nombreNivelActual);

	mandarMensaje(planificadorCCB.sockfd, HANDSHAKE, sizeof(personajeSend),
			personajeSend);
	log_info(logger, "mande HANDSHAKE a planificador");

	return personajeSend;
}

void inicializarPersonaje() {
	personaje = read_personaje_archivo_configuracion(pathArchivoConfig);
	if (personaje == NULL ) {
		fprintf(stderr, "No se pudo leer el archivo de configuracion %s\n",
				pathArchivoConfig);
		exit(1);
	}

	strcpy(nombreNivelActual, "");
	posicionActual = (Posicion*) personaje->posActual;
	flag = 0;
	miEstado = NUEVO_NIVEL;
}

void solicitarDataNivel() {

	mandarMensaje(orquestadorCCB.sockfd, REQUEST_DATA_NIVEL,
			strlen(nombreProximoNivel) + 1, nombreProximoNivel);

	//loggeo la solicitud de DATA_NIVEL al orquestador
	log_info(logger,
			string_from_format(
					"personaje %s solicita al orquestador DATA_NIVEL del %s",
					personaje->nombre, nombreProximoNivel));

	miEstado = WAIT_DATA_LEVEL;
	log_info(logger,
			string_from_format("personaje %s cambia a estado %d",
					personaje->nombre, miEstado));
}

void llegoRecurso(){
	agregarRecurso(personaje->niveles);

	posicionProximoRecurso = NULL;

	//loggea la obtencion del recurso y la finalizacion del turno
	log_info(logger,
			string_from_format(
					"personaje %s obtuvo recurso %c en %s",
					personaje->nombre,
					proxRecurso, nombreNivelActual));

	log_trace(logger,
			string_from_format(
					"personaje %s termino turno en %s",
					personaje->nombre,
					nombreNivelActual));

	//verifico si el personaje finalizo el nivel
	if (nivelTerminado(personaje->niveles,
			nombreNivelActual)) {
		mandarMensaje(nivelCCB.sockfd, TERMINE_NIVEL,
				0, NULL );
		log_info(logger,
				string_from_format(
						"personaje %s avisa a %s que lo termino",
						personaje->nombre,
						nombreNivelActual));
		mandarMensaje(planificadorCCB.sockfd, TERMINE_NIVEL,
				0, NULL );
		log_info(logger,
				string_from_format(
						"personaje %s avisa al planificador de %s que lo termino",
						personaje->nombre,
						nombreNivelActual));

		//loggea la finalizacion del nivel actual
		log_info(logger,
				string_from_format(
						"personaje %s termino %s",
						personaje->nombre,
						nombreNivelActual));

		((Posicion*) (personaje->posActual))->POS_X =
				1;
		((Posicion*) (personaje->posActual))->POS_Y =
				1;

		//personaje se desconecta del nivel actual y loggea la desconexion
		close(nivelCCB.sockfd);
		log_info(logger,
				string_from_format(
						"personaje %s se desconecta del %s",
						personaje->nombre,
						nombreNivelActual));

		//personaje se desconecta del planificador del nivel actual y loggea la desconexion
		close(planificadorCCB.sockfd);
		log_info(logger,
				string_from_format(
						"personaje %s se desconecta del planificador de %s",
						personaje->nombre,
						nombreNivelActual));

		miEstado = NUEVO_NIVEL;
		log_info(logger,
				string_from_format(
						"personaje %s cambia a estado %d",
						personaje->nombre,
						miEstado));
	} else {

		//si no termino el nivel, aviso a planificador que finalizo el turno
		mandarMensaje(planificadorCCB.sockfd, TERMINE_TURNO,
				0, NULL );
		miEstado = STANDBY;
		log_info(logger,
				string_from_format(
						"personaje %s cambia a estado %d",
						personaje->nombre,
						miEstado));
	}
}
