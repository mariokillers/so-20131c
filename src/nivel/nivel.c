#include "nivel.h"
#include <stdio.h>
#include <string.h>

//inicializo la lista de personajes para controlar, dibujar e interbloqueo (en orden)
PersonajeEnNivel* listaPersonajes;
ITEM_NIVEL* ListaItems;
ITEM_NIVEL* recursosIniciales;

//inicio al proceso como servidor y me conecto como cliente
CCB serverCCB;
CCB clientCCB;

int recovery;

//instancio el logger
t_log* logger;


int main(int argc, char *argv[]) {
	char *path_config;
	int puerto;
	logger = log_create("ProcesoNivelTestingConexiones.log", "ProcesoNivel", false, LOG_LEVEL_INFO);
	if (argc < 3) {
		fprintf(stderr, "%s: Faltan parametros (%s archivoconfig puerto)\n", "nivel", "nivel");
		exit(1);
	}

	path_config = argv[1];
	puerto = atoi(argv[2]);

	//inicializo el nivel desde el archivo config 
	t_nivel *nivel= read_nivel_archivo_configuracion(path_config);
	if (nivel == NULL) {
		fprintf(stderr, "ERROR: no se pudo leer el archivo de configuracion %s\n", path_config);
		exit(1);
	}
	Nivel yoNivel;

	//inicializo el proceso nivel
	nivel_gui_inicializar();

	serverCCB = initServer(puerto);

	//Direccion * dir = (Direccion *)
	char ip[20];
	strcpy(ip,"localhost");
	clientCCB = connectServer("localhost", ((Direccion*)(nivel->nivel_orquestador))->PORT); 

	strcpy(yoNivel.ID,nivel->nivel_nombre);
	strcpy(yoNivel.IP,"localhost");
	yoNivel.PORT = puerto;
	mandarMensaje(clientCCB.sockfd, HANDSHAKE,sizeof(Nivel),&yoNivel); 

	//inicializo el hilo que maneja interbloqueo VER ACA!
	pthread_t interbloqueo;

	//inicializo la lista de items a dibujar y controlar
	ListaItems = nivel->nivel_items;
	recursosIniciales = nivel->nivel_items; //esta lista es para saber la cantidad total de recursos que hay en el nivel (interbloqueo)

	//inicializo el recovery
	recovery = nivel->nivel_recovery;

	//inicializo la cola de mensajes
	t_queue* colaDeMensajes;
	colaDeMensajes = queue_create();
	Mensaje* mensaje;


	nivel_gui_dibujar(ListaItems);
	log_info(logger, "Terminado dibujar nivel");

	while(1){
		while(!mensajes(colaDeMensajes,serverCCB)); 
		log_info(logger, "Buscando mensaje en cola");
		//mientras tenga algun mensaje, ya sea de server o cliente.... NO TENGO QUE PONER WHILE(1)?
		//agarra de la cola de mensajes un mensaje
		mensaje = queue_pop(colaDeMensajes);
		log_info(logger, string_from_format("Recibi mensaje de tipo: %d", mensaje->type));

		//analiza los mensajes recibidos y en base a eso, actua
		switch(mensaje->type){

			case HANDSHAKE:
				log_info(logger, "Recibi handshake");
				//((Personaje*)mensaje->data)->FD=mensaje->from;

				//cargo el personaje en las listas que voy a manejar para validar recursos e interbloque
				cargarPersonajeEnNivel(((Personaje*)mensaje->data),mensaje->from);

				//creo el personaje en el nivel. Le pongo como pos inicial la (0,0) y lo dibujo
				CrearPersonaje(&ListaItems,(buscarPersonaje_byfd(mensaje->from)),0,0);
				nivel_gui_dibujar(ListaItems);

				//mensajeLogeo= char del personaje que ingreso al nivel
				//char mensajeLogeo = (buscarPersonaje_byfd(mensaje->from));

				break;

			case REQUEST_POS_RECURSO:
			{
			/**le envia al personaje la pos del nuevo recurso **/
				log_info(logger, "Recibi REQUEST_POS_RECURSO");
				Posicion pos;
				//entro en la region critica
				pos =  obtenerPosRecurso((char)(*((char*)(mensaje->data))));
				log_info(logger, string_from_format("mando la posicion del recurso %c: (%d,%d)", ((char)mensaje->data), pos.POS_X, pos.POS_Y ));

				mandarMensaje(mensaje->from, POSICION_RECURSO,sizeof(Posicion),&pos);
				log_info(logger, "mande POSICION");
			}
				break;

			case REQUEST_MOVIMIENTO:
			{
				//tomo del mensaje la posicion donde se va a mover el personaje
				
				int posx = ((Posicion*)mensaje->data)->POS_X;
				int posy = ((Posicion*)mensaje->data)->POS_Y;

				log_info(logger, string_from_format("tomo la posicion del recurso que me mando personaje: (%d,%d)", posx,posy));

				//muevo el personaje y lo dibujo
				MoverPersonaje(ListaItems,buscarPersonaje_byfd(mensaje->from),posx,posy);
				nivel_gui_dibujar(ListaItems);
				
				log_info(logger, "MOVI EL PERSONAJE");

				//modifico la posicion del personaje en listaPersonajes
				modificarPosPersonaje(mensaje->from,posx,posy);

				log_info(logger, "MODIFICO LA POSICION DEL PERSONAJE EN LA LISTA: (%d,%d)", posx,posy);

				sleep(1);
			}
				break;

			case REQUEST_RECURSO:

				//le confirma al personaje que puede tomar ese recurso y lo resta de listaItems
				if(validarPosYRecursos( buscarPersonaje_byfd(mensaje->from), *((char*)mensaje->data))){

					//le manda 1/TRUE porque lo puede tomar
					bool a = 1;
					mandarMensaje(mensaje->from,CONFIRMAR_RECURSO,sizeof(bool),&a);
					restarRecurso(ListaItems, *((char*)mensaje->data));
					agregarRecursoAPersonaje(((char*)(mensaje->from))[1],*((char*)mensaje->data));
					//ver tema del recurso pendiente
					nivel_gui_dibujar(ListaItems);

				}else{
					//si no pudo, le manda 0/FALSE porque no lo puede tomar
					mandarMensaje(mensaje->from, CONFIRMAR_RECURSO,sizeof(0),0);

					//actualiza la lista de recursos pendientes
					agregarARecursosPendientes(buscarPersonaje_byfd(mensaje->from), *((char*)mensaje->data));

				}

				break;


			/**case ENVIAR_VICTIMAS://LOGEAR ACA Y VER COMO FUNCIONA EL TEMA DEL HILO

				//...
				if(recovery){
					manderMensaje(orquestador, sizeof(),personajes)
				}
				break; **/


			//es avisado de que libere recursos y llama al orquestador para avisarle de liberarlos
			// y que este le diga cuales fueron re-asignados.
			case TERMINE_NIVEL:
			{
				//se fija que recursos tenia asignado el personaje para liberarlos
				t_recursos *recursosALiberar = liberarRecursos(buscarPersonaje_byfd(mensaje->from));

				//le manda los recursos liberados, de a 1, al orquestador
				mandarRecursosLiberados(recursosALiberar,clientCCB.sockfd);

				//por cada recurso que libera tengo que sumarlo a la cantidad en listaItems
				aumentarRecursos(recursosALiberar);

				//borra el personaje del nivel y libera al personaje de listaPersonajes
				BorrarItem(&ListaItems,buscarPersonaje_byfd(mensaje->from));
				borrarPersonajeEnNivel(buscarPersonaje_byfd(mensaje->from));

				//re-dibuja el nivel ya sin el personaje y con la cantidad de recursos nueva
				nivel_gui_dibujar(ListaItems);
			}
				break;


			case NOMBRE_VICTIMA: //LOGEO QUE HUBO RECOVERY Y PONGO EL NOMBRE DE LA VICTIMA. TIPO DE DATO: CHAR
			{

				char mensajeLogeo= *((char*)mensaje->data);

				log_info(logger, &mensajeLogeo);

			}
				break;

			//borro el mensaje
			borrarMensaje(mensaje);

		}
	}

	//nivel_gui_terminar();

	//cierro el socket del cliente

	close(clientCCB.sockfd);
	return 0;

}

void reasignarRecursos(Recursos* listaRecursos){
	/** @NAME: reasignarRecursos
	 * @DESC: recibe del orquestador el recurso que re-asigno
	*/
	Recursos * recurso;
	recurso = listaRecursos;

	restarRecurso(ListaItems, recurso->idRecurso); //resta de la lista items el recurso que re-asigno
	agregarRecursoAPersonaje (recurso->idPersonaje, recurso->idRecurso); // le agrega a listaPersonajes el recurso que obtuvo el personaje
	quitarSolicitudesDeRecurso (recurso->idPersonaje, recurso->idRecurso); //quita de la lista de solicitudes los recursos que recibio

}

void mandarRecursosLiberados(t_recursos* recursosALiberar, int fd){
	/** @NAME: mandarRecursosLiberados
	 * @DESC: le manda al orquestador la cantidad de recursos que se liberaron POR RECURSO
	 */
	t_recursos * aux;
	aux = recursosALiberar;

	t_queue* colaDeMensajes;
	colaDeMensajes = queue_create();
	Mensaje* mensaje;




	while(aux != NULL){
		//paso a la struct a la que voy a mandar los mensajes
		Recursos  recurso;
		recurso.idRecurso = aux->idRecurso;
		recurso.idPersonaje = '\0';
		recurso.cant = aux->cant;

		//le mando al orquestador los recursos liberados para que re-asigne
		mandarMensaje(fd, RECURSOS_LIBERADOS,sizeof(recurso),&recurso);

		//escucho al orquestador que me va a mandar los que re-asigno
		while((!mensajes(colaDeMensajes,serverCCB)));
			mensaje = queue_pop(colaDeMensajes);

			//mientras no sea el mensaje REASIGNACION_FINALIZADA... quiere decir que me esta mandando re-asignaciones

			while(mensaje->type !=REASIGNACION_FINALIZADA ){
				if(mensaje->type == RECURSOS_REASIGNADOS){
					//llamo a reasignar con la data que me envio
					reasignarRecursos(mensaje->data);
				} borrarMensaje(mensaje);
				//levanto nuevo mensaje
			while((!mensajes(colaDeMensajes,serverCCB)));
				mensaje = queue_pop(colaDeMensajes);
			}

		aux = aux->sig;
	}
}

Posicion obtenerPosRecurso(char recurso){
	/** @NAME: obtenerPosRecurso
	 * @DESC: devuelve la pos en la que se encuentra el recurso
	 */
	ITEM_NIVEL * itemRecurso;
	itemRecurso = buscarItem(recurso);

	//if(itemRecurso != NULL){
		Posicion posicion;
		posicion= Pos(itemRecurso->posx, itemRecurso->posy);
		return posicion;
	//}

}

int validarPosYRecursos(char idPersonaje, char idRecurso){
	/** @NAME: validarPosYRecursos
	 * @DESC: Verifico que el personaje este sobre el recurso que dice estar y a su vez veo si hay instancias del mismo para
	 * asignarle al personaje.
	 **/

	//busco la posicion del personaje en el mapa.
	PersonajeEnNivel * personaje;
	personaje = buscarPersonaje (idPersonaje);
	int personajePosx = obtenerPosX(personaje->pos);
	int personajePosy = obtenerPosY(personaje->pos);

	//busco la posicion del recurso en el mapa y la cantidad de recursos que tiene
	ITEM_NIVEL * recurso;
	recurso = buscarItem(idRecurso);

	//comparo
	if( ((personajePosx == recurso->posx) && (personajePosy == recurso->posy)) && ( (recurso->quantity)>0) ){
		return 1;
	}
	return 0;
}

ITEM_NIVEL* buscarItem(char id){

	ITEM_NIVEL * temp;
	temp = ListaItems;

	while((temp != NULL) && (temp->id != id)){
		temp = temp->next;
	}if((temp != NULL) && (temp->id == id)){
		return temp;
	}return NULL;
}

void cargarPersonajeEnNivel(Personaje* miPersonaje, int fd){
	/*@NAME: cargarPersonaje
	 * @DESC: cuando se conecta un personaje al nivel, lo agrega a la listaPersonajes que es la lista para la cual el nivel
	 * tiene actualizado los recursos que ese personaje posee en el nivel
	 */

	PersonajeEnNivel* personaje;
	log_info(logger, "Reservando memoria para personaje");
	personaje = malloc(sizeof(PersonajeEnNivel));
	log_info(logger, "Reservada memoria para personaje");

	personaje->id = miPersonaje->ID[1];
	personaje->fd = fd;

	Posicion pos;
	pos = Pos(0,0);
	personaje->pos = pos;

	personaje->recursos = NULL;

	personaje->recursoPendiente = '\0';

	personaje->marcado = false;

	personaje->sig = listaPersonajes;
	listaPersonajes = personaje;

	//free(personaje);


}


void agregarARecursosPendientes(char idPersonaje, char recurso){
	/*@NAME: agregarAListaRecursosPendientes
	 * @DESC: actualiza la lista de recursos pendientes del personaje para luego utilizarla en el interbloqueo
	 */
	PersonajeEnNivel* personaje;
	personaje = listaPersonajes;

	while( (personaje != NULL) && (personaje->id != idPersonaje) ){
		personaje = personaje->sig;
	}if( (personaje != NULL) && (personaje->id == idPersonaje)){
		personaje->recursoPendiente = recurso;
	}
}

void quitarSolicitudesDeRecurso(char idPersonaje, char idRecurso){
	/*@NAME: quitarSolicitudesDeRecurso
	 * @DESC: quita de la lista de recursos pendientes el recurs que obtuvo el personaje tras la re-asignacion
	*/

	PersonajeEnNivel* personaje;
	personaje = listaPersonajes;

	while( (personaje != NULL) && (personaje->id != idPersonaje)){
		personaje = personaje->sig;
	}if( (personaje != NULL) && (personaje->id == idPersonaje)){
		if(personaje->recursoPendiente == idRecurso){
			personaje->recursoPendiente = '\0';
		}
	}
}

void agregarRecursoAPersonaje(char idPersonaje,char recurso){
	/*@NAME: agregarRecursoAPersonaje
	 * @DESC: cuando le confirman al personaje que puede tomar el recurso, el nivel agrega a la listaPersonajes, en el
	 * personaje, el recurso que obtuvo
	*/

	PersonajeEnNivel * personaje;
	personaje = buscarPersonaje (idPersonaje);

	if(personaje->recursoPendiente == recurso){

		quitarSolicitudesDeRecurso(idPersonaje, recurso);
	}

	if(personaje!=NULL){
		t_recursos * auxList ;
		auxList = personaje->recursos;

		//busco el recurso
		while( (auxList->sig != NULL) && (auxList->idRecurso != recurso) ){
			auxList = auxList->sig;
		}if( auxList->idRecurso == recurso ){
			//si lo encontro, le suma 1 a la cant que ya tenia.
			((t_recursos*)(personaje->recursos))->cant ++ ;

		}else if((auxList->sig == NULL)){
			//si no lo encontro, lo agrega

			t_recursos * temp;
			temp = malloc(sizeof(t_recursos));

			temp->idRecurso = recurso;
			temp->cant = 1;
			temp->sig = NULL;

			personaje->recursos = temp;
			listaPersonajes = personaje;

			//free(temp);
		}
	}
}

PersonajeEnNivel* buscarPersonaje(char idPersonaje){
	/*@NAME: buscarPersonaje
	* @DESC: devuelve un PersonajeEnNivel que tenga ese id
	*/
	PersonajeEnNivel * personaje;
	personaje = listaPersonajes;

	//busco el personaje
	while ((personaje != NULL) && (personaje->id != idPersonaje)) {
		personaje = personaje->sig;
	}if ((personaje != NULL) && (personaje->id == idPersonaje)){
		return personaje;
	}return NULL;
}

void borrarPersonajeEnNivel(char idPersonaje){
	/*@NAME: borrarPersonaje
	* @DESC: cuando un personaje termina el nivel, lo borra de listaPersonajes
	*/

	PersonajeEnNivel * personaje = listaPersonajes;
	PersonajeEnNivel * personajeAnterior;

    if ((personaje != NULL) && (personaje->id == idPersonaje)) {
    	listaPersonajes = ((PersonajeEnNivel*)listaPersonajes)->sig;
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


t_recursos* liberarRecursos(char idPersonaje ){
	/*@NAME: liberarRecursos
	 * @DESC: me devuelve la lista de recursos del personaje que termino el nivel
	*/

	PersonajeEnNivel * personaje;
	personaje = buscarPersonaje(idPersonaje);

 	if((personaje != NULL)){
 		return personaje->recursos;
	}return NULL;

}

void aumentarRecursos(t_recursos* recursosALiberar){
	/*@NAME: aumentarRecursos
	 * @DESC: actualiza el estado de los recursos en ListaItems, dependiendo de los recursos liberados
	*/

	t_recursos * aux;
	aux = recursosALiberar;

	 while(aux!= NULL){

		 //me fijo de que recurso se trata y sumo cantidades a liberar donde corresponda

		 //ITEM_NIVEL * recurso = buscarItem(aux.idRecurso);
		 agregarRecursosAListaItems(aux->idRecurso, aux->cant);

		 aux= aux->sig;
	 }

}

void agregarRecursosAListaItems(char idRecurso, int cant){
	/*@NAME: agregarRecursosAListaItems
	 * @DESC: dado un id de recurso, lo busco en la lista Items y le sumo la cantidad que libero
	*/

	ITEM_NIVEL * temp;
	temp = ListaItems;

	while( (temp!= NULL) && (temp->id != idRecurso)){
		temp= temp->next;
	}if((temp != NULL) && (temp->id == idRecurso)){
		temp->quantity = temp->quantity + cant;
	}
}

void modificarPosPersonaje(int fdPersonaje, int posx, int posy){
	/*@NAME: modificarPosPersonaje
	* @DESC: actualiza la pos del personaje
	*/
	log_info(logger, string_from_format("Modifico posicion personaje de fd %d en posicion (%d,%d)", fdPersonaje, posx, posy));

	PersonajeEnNivel * personaje;
	personaje = buscarPersonaje_byfd(fdPersonaje);

	if (personaje != NULL){
		personaje->pos = Pos(posx,posy);
	}

}

PersonajeEnNivel *buscarPersonaje_byfd(int fd){
	/*@NAME: buscarPersonaje
	* @DESC: devuelve un PersonajeEnNivel que tenga ese id
	*/
	PersonajeEnNivel * personaje;
	personaje = listaPersonajes;

	//busco el personaje
	while ((personaje != NULL) && (personaje->fd != fd)) {
		personaje = personaje->sig;
	}if ((personaje != NULL) && (personaje->fd == fd)){
		return personaje;
	}
	return NULL;

}
