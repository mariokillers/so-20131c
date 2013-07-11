#include "nivel.h"
#include <stdio.h>
#include <string.h>
#include "interbloqueo.h"

int main(int argc, char* argv[]) {
	pthread_mutex_init(&mutex, NULL );
	pthread_mutex_init(&deadlock_mutex, NULL );
	pthread_mutex_lock(&deadlock_mutex);

	signal(SIGINT,rutinaSignal);


	char *path_config;
	if (argc < 2) {
		fprintf(stderr, "%s: Faltan parametros (%s archivoconfig )\n",
				"nivel", "nivel");
		exit(1);
	}

	path_config = argv[1];

	//inicializo el nivel desde el archivo config
	nivel = read_nivel_archivo_configuracion(path_config);

	if (nivel == NULL ) {
		fprintf(stderr,
				"ERROR: no se pudo leer el archivo de configuracion %s\n",
				path_config);
		exit(1);
	}

	logger = log_create(string_from_format("Proceso%s.log", nivel->nombre), "ProcesoNivel", false,
			LOG_LEVEL_INFO);
	loggerInterbloqueo = log_create(string_from_format("Interbloqueo%s.log", nivel->nombre), "Interbloqueo", false, LOG_LEVEL_INFO);

	Nivel yoNivel;

	//inicializo el proceso nivel
	nivel_gui_inicializar();

	serverCCB = initServer(nivel->miDireccion->PORT);

	clientCCB = connectServer(nivel->orquestador->IP, nivel->orquestador->PORT);

	log_info(logger,
			string_from_format("El nivel se conecto al puerto: %d",  nivel->orquestador->PORT));

	//le mando handshake al orquestador
	strcpy(yoNivel.ID, nivel->nombre);
	strcpy(yoNivel.IP, nivel->miDireccion->IP);
	yoNivel.PORT = nivel->miDireccion->PORT;
	mandarMensaje(clientCCB.sockfd, HANDSHAKE, sizeof(Nivel), &yoNivel);
	log_info(logger,
			string_from_format("El nivel: %s le hizo HANDSHAKE al socket: %d",
					yoNivel.ID, clientCCB.sockfd));

	//inicializo el recovery
	recovery = nivel->recovery;
	if (recovery)
		log_info(logger, "El recovery esta activado");

	recovery_time = nivel->tiempo_deadlcok;

	//inicializo la cola de mensajes
	t_queue* colaDeMensajes;
	colaDeMensajes = queue_create();
	Mensaje* mensaje;

	//entro en la region critica
	pthread_mutex_lock(&mutex);

	//inicializo la lista de items a dibujar y controlar
	ListaItems = nivel->nivel_items;
	recursosIniciales = nivel->nivel_items; //esta lista es para saber la cantidad total de recursos que hay en el nivel (interbloqueo)

	//inicializo el hilo que maneja interbloqueo VER ACA!
	pthread_create(&thread_interbloqueo, NULL, &interbloqueo, NULL );

	nivel_gui_dibujar(ListaItems);

	pthread_mutex_unlock(&mutex);
	//salgo de la region critica

	while (1) {
		//Recibo mensajes, y bloqueo si no hay.
		while (!mensajes(colaDeMensajes, serverCCB))
			;

		//mientras tenga algun mensaje, ya sea de server o cliente, agarra de la cola de mensajes un mensaje
		mensaje = queue_pop(colaDeMensajes);

		//analiza los mensajes recibidos y en base a eso, actua
		switch (mensaje->type) {

		default: {
			log_error(logger, "No recibi ningun mensaje");
		}
		break;

		case HANDSHAKE: {
			Personaje* personajeNuevo = malloc(sizeof(Personaje));
			memcpy(personajeNuevo, mensaje->data, sizeof(Personaje));
			personajeNuevo->FD = mensaje->from;

			log_info(logger,
					string_from_format("Recibi HANDSHAKE del personaje: %s",
							personajeNuevo->ID));

			//entro en la region critica
			pthread_mutex_lock(&mutex);

			//cargo el personaje en las listas que voy a manejar para validar recursos e interbloqueo
			PersonajeEnNivel* miPersonaje = cargarPersonajeEnNivel(
					personajeNuevo);

			//creo el personaje en el nivel. Le pongo como pos inicial la (0,0) y lo dibujo
			CrearPersonaje(&ListaItems, miPersonaje->id, 0, 0);
			nivel_gui_dibujar(ListaItems);

			pthread_mutex_unlock(&mutex);
			//salgo de la region critica

			log_info(logger,
					string_from_format(
							"Se dibujo el personaje: %c en la pos (1,1)",
							miPersonaje->id));
		}
		break;

		case REQUEST_POS_RECURSO: {
			//le envia al personaje la pos del nuevo recurso

			log_info(logger, "Recibi REQUEST_POS_RECURSO");

			char recurso = *(char *) mensaje->data;
			log_info(logger,
					string_from_format("El recurso pedido es: %c", recurso));

			//entro en la region critica
			pthread_mutex_lock(&mutex);

			//obtengo la posicion del recurso
			Posicion pos = obtenerPosRecurso(recurso);

			pthread_mutex_unlock(&mutex);
			//salgo de la region critica

			mandarMensaje(mensaje->from, POSICION_RECURSO, sizeof(Posicion),
					&pos);

			PersonajeEnNivel* personaje = buscarPersonaje_byfd(mensaje->from);

			log_info(logger,
					string_from_format(
							"Mande la posicion del recurso: %c: (%d,%d) al personaje: %c",
							recurso,pos.POS_X, pos.POS_Y ,personaje->id));
		}
		break;

		case REQUEST_MOVIMIENTO: {
			log_info(logger, "Recibi REQUEST_MOVIMIENTO");

			//entro en la region critica
			pthread_mutex_lock(&mutex);

			//tomo del mensaje la posicion donde se va a mover el personaje
			Posicion* pos = ((Posicion*) mensaje->data);

			int posx = pos->POS_X;
			int posy = pos->POS_Y;

			PersonajeEnNivel* personaje = buscarPersonaje_byfd(mensaje->from);

			//muevo el personaje y lo dibujo
			MoverPersonaje(ListaItems, personaje->id, posx, posy);
			nivel_gui_dibujar(ListaItems);

			//modifico la posicion en la listaPersonajes
			modificarPosPersonaje(personaje, posx, posy);

			pthread_mutex_unlock(&mutex);
			//salgo de la region critica

			log_info(logger,
					string_from_format(
							"Movi el personaje: %c a la posicion:(%d,%d)",
							personaje->id, posx, posy));

		}
		break;

		case REQUEST_RECURSO: {
			log_info(logger, "Recibi REQUEST_RECURSO");

			char recurso = *(char *) mensaje->data;

			//entro en la region critica
			pthread_mutex_lock(&mutex);

			PersonajeEnNivel* personaje = buscarPersonaje_byfd(mensaje->from);

			log_info(logger,
					string_from_format(
							"El personaje: %c me pidio el recurso:%c",
							personaje->id, recurso));

			//le confirma al personaje que puede tomar ese recurso y lo resta de listaItems
			bool puedeTomarRecurso;
			if (validarPosYRecursos(personaje, recurso)) {
				puedeTomarRecurso = true;
				log_info(logger,
						string_from_format(
								"El personaje:%c pudo obtener el recurso:%c",
								personaje->id, recurso));

				mandarMensaje(mensaje->from, CONFIRMAR_RECURSO, sizeof(bool),
						&puedeTomarRecurso);

				restarRecurso(ListaItems, recurso);

				log_info(logger,
								string_from_format("Resto el recurso: %c de ListaItems",
												recurso));
				agregarRecursoAPersonaje(personaje, recurso);
				log_info(logger,
						string_from_format(
								"Se agrega recurso: %c al personaje: %c",
								recurso, personaje->id));
				nivel_gui_dibujar(ListaItems);

			} else {
				puedeTomarRecurso = false;
				log_info(logger,
						string_from_format(
								"El personaje:%c no pudo obtener el recurso:%c",
								personaje->id, recurso));
				//si no pudo, le manda 0/FALSE porque no lo puede tomar
				mandarMensaje(mensaje->from, CONFIRMAR_RECURSO, sizeof(bool),
						&puedeTomarRecurso);
				//actualiza la lista de recursos pendientes
				agregarARecursosPendientes(personaje, recurso);
				log_info(logger,
						string_from_format(
								"Se agrego la solicitud del recurso:%c del personaje:%c a solicitudes pendientes",
								recurso, personaje->id));
			}

			pthread_mutex_unlock(&mutex);
			//salgo de la region critica

		}
		break;

		//es avisado de que libere recursos y llama al orquestador para avisarle de liberarlos
		// y que este le diga cuales fueron re-asignados.
		case TERMINE_NIVEL: {
			log_info(logger, "Recibi TERMINE_NIVEL");

			//entro en la region critica
			pthread_mutex_lock(&mutex);

			PersonajeEnNivel* personaje = buscarPersonaje_byfd(mensaje->from);

			pthread_mutex_unlock(&mutex);
			//salgo de la region critica

			log_info(logger,
							string_from_format(
								"El personaje: %c ha terminado el nivel",
								personaje->id));

			matarPersonaje(mensaje->from);

		}
		break;

		case NOMBRE_VICTIMA: {
			log_info(logger, "Recibi NOMBRE_VICTIMA");

			//loggeo la victima de interbloqueo

			char idVictima = *(char *) mensaje->data;

			log_info(logger,
					string_from_format(
							"El personaje: %c ha sido elegido como victima del interbloqueo",
							idVictima));

		}
		break;

		//borro el mensaje
		borrarMensaje(mensaje);

		}
		log_info(logger, "----------------------------------------------");
	}

	//cierro el hilo de interbloqueo
	pthread_mutex_unlock(&deadlock_mutex);

	pthread_join(thread_interbloqueo, NULL );

	//cierro el socket del cliente
	close(clientCCB.sockfd);

	return 0;

}

void matarPersonaje(int fdPersonaje) {
	/** @NAME: matarPersonaje
	 * @DESC: mata al personaje, es decir, libera sus recursos y lo borra del nivel
	 */

	//entro en la region critica
	pthread_mutex_lock(&mutex);

	PersonajeEnNivel* personaje = buscarPersonaje_byfd(fdPersonaje);

	//se fija que recursos tenia asignado el personaje para liberarlos
	t_recursos* recursosALiberar = personaje->recursos;

	log_info(logger,
				string_from_format("Tengo recursos a liberar por parte del personaje: %c ",
						personaje->id));

	//reasigno los recursos que libera el personaje a listaItems
	reasignarRecursosAListaItems(recursosALiberar);

	//le manda los recursos liberados, de a 1, al orquestador
	mandarRecursosLiberados(recursosALiberar, clientCCB.sockfd);

	log_info(logger, "Mande recursos liberados");

	log_info(logger,
				string_from_format("El personaje: %c ha sido borrado del nivel",
						personaje->id));

	//borra el personaje del nivel y libera al personaje de listaPersonajes
	BorrarItem(&ListaItems, personaje->id);
	borrarPersonajeEnNivel(personaje->id);


	//re-dibuja el nivel ya sin el personaje y con la cantidad de recursos nueva
	nivel_gui_dibujar(ListaItems);

	pthread_mutex_unlock(&mutex);
	//salgo de la region critica
}

void reasignarRecursosAListaItems(t_recursos *listaRecursos) {
	/** @NAME: reasignarRecursos
	 * @DESC: reasigna los recursos que libero el personaje
	 */
	t_recursos* recurso = listaRecursos;

	while(recurso != NULL){

		agregarRecursosAListaItems(recurso->idRecurso, recurso->cant);

		recurso = recurso->sig;
	}
}

void mandarRecursosLiberados(t_recursos* recursosALiberar, int fdOrquestador) {
	/** @NAME: mandarRecursosLiberados
	 * @DESC: le manda al orquestador la cantidad de recursos que se liberaron POR RECURSO
	 */
	t_recursos* aux = recursosALiberar;

	t_queue* colaDeMensajes = queue_create();
	Mensaje* mensaje;

	while (aux != NULL ) {
		//paso a la struct a la que voy a mandar los mensajes
		Recursos *recurso;
		recurso = malloc(sizeof(Recursos));

		recurso->idRecurso = aux->idRecurso;
		recurso->idPersonaje = '\0';
		recurso->cant = aux->cant;

		log_info(logger,
				string_from_format(
						"La cantidad del recurso: %c que sera liberado es: %d",
						recurso->idRecurso, recurso->cant));

		//le mando al orquestador los recursos liberados para que re-asigne
		mandarMensaje(fdOrquestador, RECURSOS_LIBERADOS, sizeof(Recursos),
				recurso);

		log_info(logger, "Mando mensaje al orquestador con los recursos que libero");

		//escucho al orquestador que me va a mandar los que re-asigno
		while ((!mensajes(colaDeMensajes, clientCCB)))
			;

		mensaje = queue_pop(colaDeMensajes);

		//mientras no sea el mensaje REASIGNACION_FINALIZADA... quiere decir que me esta mandando re-asignaciones

		while (mensaje->type != REASIGNACION_FINALIZADA) {
			if (mensaje->type == RECURSOS_REASIGNADOS) {

				//llamo a reasignar con la data que me envio
				Recursos* recursoAReasignar = (Recursos*)mensaje->data;

				restarRecurso(ListaItems, recursoAReasignar->idRecurso);

				PersonajeEnNivel* personaje = buscarPersonaje_byid( recursoAReasignar->idPersonaje);
				agregarRecursoAPersonaje(personaje, recursoAReasignar->idRecurso);

				log_info(logger,
								string_from_format(
										"Reasigno el recurso: %c al personaje: %c",
										recursoAReasignar->idRecurso, personaje->id));

			}
			borrarMensaje(mensaje);
			//levanto nuevo mensaje
			while ((!mensajes(colaDeMensajes, clientCCB)))
				;
			mensaje = queue_pop(colaDeMensajes);
		}

		aux = aux->sig;
	}
}

Posicion obtenerPosRecurso(char recurso) {
	/** @NAME: obtenerPosRecurso
	 * @DESC: devuelve la pos en la que se encuentra el recurso
	 */

	ITEM_NIVEL* itemRecurso = buscarItem(recurso);
	if (itemRecurso == NULL ) {
		log_error(logger, "No se encontro el recurso en el nivel!");
	}

	Posicion posicion = Pos(itemRecurso->posx, itemRecurso->posy);
	return posicion;

}

int validarPosYRecursos(PersonajeEnNivel* personaje, char idRecurso) {
	/** @NAME: validarPosYRecursos
	 * @DESC: Verifico que el personaje este sobre el recurso que dice estar y a su vez veo si hay instancias del mismo para
	 * asignarle al personaje.
	 */

	//busco la posicion del personaje en el mapa
	int personajePosx = (personaje->pos).POS_X;
	int personajePosy = (personaje->pos).POS_Y;

	//busco la posicion del recurso en el mapa y la cantidad de recursos que tiene
	ITEM_NIVEL* recurso = buscarItem(idRecurso);

	log_info(logger,
			string_from_format(
					"La posicion del personaje: %c es:(%d,%d) y la posicion del recurso: %c es:(%d,%d) y su cantidad: %d",
					personaje->id, personajePosx, personajePosy, idRecurso,
					recurso->posx, recurso->posy, recurso->quantity));

	//comparo
	if (((personajePosx == recurso->posx) && (personajePosy == recurso->posy))
			&& ((recurso->quantity) > 0)) {
		return 1;
	}
	return 0;
}

ITEM_NIVEL* buscarItem(char id) {
	ITEM_NIVEL* temp = ListaItems;

	while ((temp != NULL )&& (temp->id != id)){
		temp = temp->next;
	}

	return temp;
}

PersonajeEnNivel* cargarPersonajeEnNivel(Personaje* miPersonaje) {
	/*@NAME: cargarPersonaje
	 * @DESC: cuando se conecta un personaje al nivel, lo agrega a la listaPersonajes que es la lista para la cual el nivel
	 * tiene actualizado los recursos que ese personaje posee en el nivel
	 */

	PersonajeEnNivel* personaje;
	personaje = malloc(sizeof(PersonajeEnNivel));

	personaje->id = miPersonaje->ID[1];
	personaje->fd = miPersonaje->FD;

	Posicion pos;
	pos = Pos(0, 0);
	personaje->pos = pos;

	personaje->recursos = NULL;

	personaje->recursoPendiente = '\0';

	personaje->sig = listaPersonajes;
	listaPersonajes = personaje;

	return personaje;

}

void agregarARecursosPendientes(PersonajeEnNivel *personaje, char recurso) {
	/*@NAME: agregarAListaRecursosPendientes
	 * @DESC: actualiza la lista de recursos pendientes del personaje para luego utilizarla en el interbloqueo
	 */

	if (personaje != NULL ) {
		personaje->recursoPendiente = recurso;
	}
}

void quitarSolicitudesDeRecurso(PersonajeEnNivel *personaje, char idRecurso) {
	/*@NAME: quitarSolicitudesDeRecurso
	 * @DESC: quita de la lista de recursos pendientes el recurs que obtuvo el personaje tras la re-asignacion
	 */

	if (personaje->recursoPendiente == idRecurso) {
		personaje->recursoPendiente = '\0';
	}
}

void agregarRecursoAPersonaje(PersonajeEnNivel *personaje, char recurso) {
	/*@NAME: agregarRecursoAPersonaje
	 * @DESC: cuando le confirman al personaje que puede tomar el recurso, el nivel agrega a la listaPersonajes, en el
	 * personaje, el recurso que obtuvo
	 */

	//si es el recurso que tenia pendiente, lo borra
	if (personaje->recursoPendiente == recurso) {

		log_info(logger, "Es recurso pendiente");
		quitarSolicitudesDeRecurso(personaje, recurso);
	}

	t_recursos* auxList = personaje->recursos;

	if (auxList == NULL ) {
		t_recursos* temp;
		temp = malloc(sizeof(t_recursos));

		temp->idRecurso = recurso;
		temp->cant = 1;
		temp->sig = NULL;

		log_info(logger, string_from_format("El personaje no tenia recursos.El recurso: %c se agrega al personaje: %c", temp->idRecurso, personaje->id));

		personaje->recursos = temp;

	} else {
		while ((auxList->sig != NULL )&& (auxList->idRecurso != recurso) ){
			auxList = auxList->sig;
		}if( auxList->idRecurso == recurso ) {
			log_info(logger, string_from_format("El recurso: %c se agrega al personaje: %c", auxList->idRecurso, personaje->id));

			//si lo encontro, le suma el recurso a la cant que ya tenia
			auxList->cant++ ;

			log_info(logger, string_from_format("El recurso: %c tiene: %d recursos", auxList->idRecurso, auxList->cant));

		}else if(auxList->sig == NULL) {

			//si no lo encontro, lo agrega en la lista de recursos del personaje

			t_recursos* temp;
			temp = malloc(sizeof(t_recursos));

			temp->idRecurso = recurso;
			temp->cant = 1;
			temp->sig = personaje->recursos;

			personaje->recursos = temp;

		}
	}

}

void borrarPersonajeEnNivel(char idPersonaje) {
	/*@NAME: borrarPersonaje
	 * @DESC: cuando un personaje termina el nivel, lo borra de listaPersonajes
	 */

	PersonajeEnNivel* personaje = listaPersonajes;
	PersonajeEnNivel* personajeAnterior;

	if ((personaje != NULL )&& (personaje->id == idPersonaje)){
		listaPersonajes = listaPersonajes->sig;
		free(personaje);
	} else {
		while((personaje != NULL) && (personaje->id != idPersonaje)) {
			personajeAnterior = personaje;
			personaje = personaje->sig;
		}
		if ((personaje != NULL) && (personaje->id == idPersonaje)) {
			personajeAnterior->sig = personaje->sig;
			free(personaje);
		}
	}
}

void aumentarRecursos(t_recursos* recursosALiberar) {
	/*@NAME: aumentarRecursos
	 * @DESC: actualiza el estado de los recursos en ListaItems, dependiendo de los recursos liberados
	 */

	t_recursos* aux = recursosALiberar;

	while (aux != NULL ) {

		//me fijo de que recurso se trata y sumo cantidades a liberar donde corresponda

		agregarRecursosAListaItems(aux->idRecurso, aux->cant);

		aux = aux->sig;
	}
}

void agregarRecursosAListaItems(char idRecurso, int cant) {
	/*@NAME: agregarRecursosAListaItems
	 * @DESC: dado un id de recurso, lo busco en la lista Items y le sumo la cantidad que libero
	 */

	ITEM_NIVEL* temp = buscarItem(idRecurso);

	temp->quantity = temp->quantity + cant;

	log_info(logger,
					string_from_format("Reasigno a ListaItems el recurso: %c con la cant: %d",
						temp->id, cant));
}

void modificarPosPersonaje(PersonajeEnNivel* personaje, int posx, int posy) {
	/*@NAME: modificarPosPersonaje
	 * @DESC: actualiza la pos del personaje
	 */

	log_info(logger,
			string_from_format(
					"Modifico posicion personaje: %c en posicion (%d,%d)",
					personaje->id, posx, posy));

	if (personaje != NULL ) {
		personaje->pos = Pos(posx, posy);
	}

}

PersonajeEnNivel* buscarPersonaje_byfd(int fd) {
	/*@NAME: buscarPersonaje
	 * @DESC: devuelve un PersonajeEnNivel que tenga ese fd
	 */
	PersonajeEnNivel* personaje = listaPersonajes;

	//busco el personaje
	while ((personaje != NULL )&& (personaje->fd != fd)){
		personaje = personaje->sig;
	}
	if ((personaje != NULL )&& (personaje->fd == fd)){
		return personaje;
	}
	return NULL ;

}

PersonajeEnNivel* buscarPersonaje_byid(char id) {
	/*@NAME: buscarPersonaje
	 * @DESC: devuelve un PersonajeEnNivel que tenga ese id
	 */
	PersonajeEnNivel* personaje = listaPersonajes;

	//busco el personaje
	while ((personaje != NULL )&& (personaje->id != id)){
		personaje = personaje->sig;
	}
	if ((personaje != NULL )&& (personaje->id == id)){
		return personaje;
	}
	return NULL ;

}

void rutinaSignal(int n){
	/*@NAME: rutinaSignal
	 * @DESC: cuando llega la senal cntrl+c, le avisa al orquestador
	 */
	log_info(logger, "Recibi senal de morir");

	mandarMensaje(clientCCB.sockfd, CERRANDO_NIVEL, sizeof(nivel->nombre),nivel->nombre );

	exit(1);
}
