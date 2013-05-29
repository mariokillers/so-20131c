/*
 * Proceso_Nivel.c
 *
 *  Created on: 21/04/2013
 *      Author: petauriel
 */


#include "Proceso Nivel.h"



//inicializo la lista de personajes para controlar, dibujar e interbloqueo (en orden)
PersonajeEnNivel* listaPersonajes;
ITEM_NIVEL* ListaItems;
ITEM_NIVEL* recursosIniciales;
RecursoPendientePersonaje* listaRecursosPendientes;

//inicio al proceso como servidor y me conecto como cliente
CCB serverCCB;
CCB clientCCB;

int recovery;


int main(void) {

	//inicializo el nivel desde el archivo config 
	t_nivel *nivel= read_nivel_archivo_configuracion("/home/utnso/Escritorio/TP/tp-20131c-mario-killers/Configs/nivel1.config");
	Nivel yoNivel;

	//instancio el logger
	t_log* logger = log_create("ProcesoNivelTestingConexiones.log", "ProcesoNivel", true, LOG_LEVEL_INFO);
	//inicializo el proceso nivel
	nivel_gui_inicializar();

	serverCCB = initServer(6000);  // chequear esto

	//Direccion * dir = (Direccion *)
	char ip[20];
	strcpy(ip,"localhost");
	clientCCB = connectServer("localhost", ((Direccion*)(nivel->nivel_orquestador))->PORT); 

	strcpy(yoNivel.ID,nivel->nivel_nombre);
	strcpy(yoNivel.IP,"localhost");
	yoNivel.PORT=6000;
	mandarMensaje(clientCCB.sockfd, HANDSHAKE,sizeof(Nivel),&yoNivel); 
	 /*
	 * tome del nmbre el ultimo caracter y lo concatene con el puerto que elija
	 * */

	//inicializo el hilo que maneja interbloqueo VER ACA!
	pthread_t interbloqueo;
	pthread_create( &interbloqueo, NULL, interbloqueo, NULL );


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

	//mientras tenga algun mensaje, ya sea de server o cliente.... NO TENGO QUE PONER WHILE(1)?
	while( (mensajes(colaDeMensajes,serverCCB)) || (mensajes(colaDeMensajes, clientCCB)) ){

		//agarra de la cola de mensajes un mensaje
		mensaje = queue_pop(colaDeMensajes);


		//analiza los mensajes recibidos y en base a eso, actua
		switch(mensaje->type){

			case HANDSHAKE:
				((Personaje*)mensaje->data)->FD=mensaje->from;
				//cargo el personaje en las listas que voy a manejar para validar recursos e interbloque
				cargarPersonajeEnNivel(((Personaje*)mensaje->data));
				cargarPersonajeEnPendiente(buscarPersonaje_byfd(mensaje->from));

				//creo el personaje en el nivel. Le pongo como pos inicial la (0,0) y lo dibujo
				CrearPersonaje(&ListaItems,(buscarPersonaje_byfd(mensaje->from)),0,0);
				nivel_gui_dibujar(ListaItems);

				//mensajeLogeo= char del personaje que ingreso al nivel
				char mensajeLogeo = (buscarPersonaje_byfd(mensaje->from));

				log_info(logger, &mensajeLogeo);

				break;

			case REQUEST_POS_RECURSO:
			{
			/**le envia al personaje la pos del nuevo recurso **/

				Posicion pos;
				pos =  obtenerPosRecurso((char)(*((char*)(mensaje->data))));

				mandarMensaje(mensaje->from, POSICION_RECURSO,sizeof(Posicion),&pos);
			}
				break;

			case REQUEST_MOVIMIENTO:
			{
				//tomo del mensaje la posicion donde se va a mover el personaje
				int posx = obtenerPosX(*((Posicion*)mensaje->data));
				int posy = obtenerPosY(*(Posicion*)mensaje->data);

				//muevo el personaje y lo dibujo
				MoverPersonaje(ListaItems,buscarPersonaje_byfd(mensaje->from),posx,posy);
				nivel_gui_dibujar(ListaItems);

				//modifico la posicion del personaje en listaPersonajes
				modificarPosPersonaje(((char*)(mensaje->from))[1],posx,posy);

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
					nivel_gui_dibujar(ListaItems);

				}else{
					//si no pudo, le manda 0/FALSE porque no lo puede tomar
					mandarMensaje(mensaje->from, CONFIRMAR_RECURSO,sizeof(0),0);

					//actualiza la lista de recursos pendientes
					agregarAListaRecursosPendientes(buscarPersonaje_byfd(mensaje->from), *((char*)mensaje->data));

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

				//borra el personaje del nivel y libera al personaje de listaPersonajes y listaRecursosPendientes
				BorrarItem(&ListaItems,buscarPersonaje_byfd(mensaje->from));
				borrarPersonajeEnNivel(buscarPersonaje_byfd(mensaje->from));
				borrarPersonajeEnPendiente(buscarPersonaje_byfd(mensaje->from));

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

void cargarPersonajeEnNivel(Personaje* miPersonaje){
	/*@NAME: cargarPersonaje
	 * @DESC: cuando se conecta un personaje al nivel, lo agrega a la listaPersonajes que es la lista para la cual el nivel
	 * tiene actualizado los recursos que ese personaje posee en el nivel
	 */

	PersonajeEnNivel* personaje;
	personaje = malloc(sizeof(PersonajeEnNivel));

	personaje->id = miPersonaje->ID[1];
	personaje->fd = miPersonaje->FD;

	Posicion pos;
	pos = Pos(0,0);
	personaje->pos = pos;

	personaje->recursos = NULL;

	personaje->sig = listaPersonajes;
	listaPersonajes = personaje;


}

void cargarPersonajeEnPendiente(char id){
	/*@NAME: cargarPersonajeEnPendiente
	 * @DESC: cuando se conecta un personaje al nivel, lo agrega a la listaRecursosPendientes que es la lista para la cual el nivel
	 * tiene actualizado las solicitudes de recurso para luego utilizarlas para chequear interbloqueo
	 */

	RecursoPendientePersonaje* personaje;
	personaje = malloc(sizeof(RecursoPendientePersonaje));

	personaje->idPersonaje = id;
	personaje->recursoPendiente = '\0'; // va NULL o va ' '?
	personaje->sig= listaRecursosPendientes;

	listaRecursosPendientes = personaje;

}

void agregarAListaRecursosPendientes(char idPersonaje, char recurso){
	/*@NAME: agregarAListaRecursosPendientes
	 * @DESC: actualiza la lista de recursos pendientes del personaje para luego utilizarla en el interbloqueo
	 */
	RecursoPendientePersonaje* personaje;
	personaje = listaRecursosPendientes;

	while( (personaje != NULL) && (personaje->idPersonaje != idPersonaje) ){
		personaje = personaje->sig;
	}if( (personaje != NULL) && (personaje->idPersonaje == idPersonaje)){
		personaje->recursoPendiente = recurso;
	}
}

void quitarSolicitudesDeRecurso(char idPersonaje, char idRecurso){
	/*@NAME: quitarSolicitudesDeRecurso
	 * @DESC: quita de la lista de recursos pendientes el recurs que obtuvo el personaje tras la re-asignacion
	*/

	RecursoPendientePersonaje* personaje;
	personaje = listaRecursosPendientes;

	while( (personaje != NULL) && (personaje->idPersonaje != idPersonaje)){
		personaje = personaje->sig;
	}if( (personaje != NULL) && (personaje->idPersonaje == idPersonaje)){
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

void borrarPersonajeEnPendiente(char idPersonaje){
	/*@NAME: borrarPersonajeEnPendiente
	 * @DESC: cuando un personaje termina el nivel, lo borra de listaRecursosPendientes
	 */

	RecursoPendientePersonaje * personaje = listaRecursosPendientes;
	RecursoPendientePersonaje * personajeAnterior;

	if ((personaje != NULL) && (personaje->idPersonaje == idPersonaje)) {
	    listaRecursosPendientes = ((RecursoPendientePersonaje*)listaRecursosPendientes)->sig;
		free(personaje);
	}else {
    	while((personaje != NULL) && (personaje->idPersonaje != idPersonaje)) {
    		personajeAnterior = personaje;
    		personaje = personaje->sig;
        }
    	if ((personaje != NULL) && (personaje->idPersonaje == idPersonaje)) {
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

void modificarPosPersonaje(char idPersonaje, int posx, int posy){
	/*@NAME: modificarPosPersonaje
	* @DESC: actualiza la pos del personaje
	*/

	PersonajeEnNivel * personaje;
	personaje = buscarPersonaje (idPersonaje);

	if (personaje != NULL){
		personaje->pos = Pos(posx,posy);
	}

}

char buscarPersonaje_byfd(int fd){
	/*@NAME: buscarPersonaje
	* @DESC: devuelve un PersonajeEnNivel que tenga ese id
	*/
	PersonajeEnNivel * personaje;
	personaje = listaPersonajes;

	//busco el personaje
	while ((personaje != NULL) && (personaje->fd != fd)) {
		personaje = personaje->sig;
	}if ((personaje != NULL) && (personaje->fd == fd)){
		return personaje->id;
	}
	return '\0';

}

//VER TEMA SEMAFOROS
void* interbloqueo(void* a){
	/*@NAME: interbloqueo	
	* @DESC: hilo que se encarga de detectar interbloqueo
	*/

	int cantidadPersonajes = cantidadPersonajes();
	int cantRecursos = cantidadRecursos();

	//vector para saber que procesos estan interbloqueados
	bool marcados[];
	//vectores que referencian en la posicion de matrices y vectores para detectar interbloqueo
	char referenciaProceso[cantidadPersonajes];
	char referenciaRecurso[cantRecursos];

	//vectores para interbloqueo
	int recursosTotales[cantRecursos];
	int recursosDisponibles[cantRecursos];

	//matrices para interbloqueo
	int recursosAsignados[cantidadPersonajes][cantRecursos];
	int recursosSolicitados[cantidadPersonajes][cantRecursos];

	cargarRecursosTotales(recursosTotales, cantRecursos);
	cargarRecursosDisponibles(recursosDisponibles, cantRecursos);
	cargarRecursosSolicitados(recursosSolicitados);
	cargarRecursosAsignados(recursosAsignados);


}

int buscarEnReferenciaRecurso(char idRecurso, char referenciaRecurso[]){
	/*@NAME: buscarEnReferenciaRecurso	
	* @DESC: busca en el vector que hace referencia a los recursos la pos de ese recurso en las matrices/vectores	
	*/	
	int i=0;
	bool encontrado = false;
	while(!encontrado){
		if(referenciaRecurso[i] == idRecurso){
			encontrado = true;
			return i;
		}else{
			i++;
		}
	}return -1;
			
}

int buscarEnReferenciaProceso(char idProceso, char referenciaProceso[]){
	/*@NAME: buscarEnReferenciaProceso
	* @DESC: busca en el vector que hace referencia a los personajes la pos de ese personaje en las matrices
	*/	
	int i=0;
	bool encontrado = false;
	while(!encontrado){
		if(referenciaProceso[i] == idProceso){
			encontrado = true;
			return i;
		}else{
			i++;
		}
	}return -1;
				
}
				

int cantidadPersonajes(){
	/*@NAME: cantidadProcesos
	* @DESC: devuelve la cantidad de personajes conectados al nivel
	*/
	int i =0;
	PersonajeEnNivel * personaje;
	personaje = listaPersonajes;

	while(personaje != NULL){
		i++;
		personaje= personaje->sig;
	}return i;
}

int cantidadRecursos(){
	/*@NAME: cantidadRecursos
	* @DESC: devuelve la cantidad de recursos que hay en el nivel
	* NOTA: como no hay personajes en esta lista no tengo que diferenciar recursos de personajes
	*/
	int i =0;
	ITEM_NIVEL* recurso;
	recurso = recursosIniciales;

	while(recurso != NULL){
		i++;
		recurso= recurso->next;
	}return i;
}

void cargarRecursosTotales(int recursosTotales[], int cantRecursos , char referenciaRecurso[]){
	/*@NAME: cargarRecursosTotales
	* @DESC: completa el vector con la cantidad de recursos que hay en total
	*/

	int i;
	int pos =-1;

	ITEM_NIVEL* recurso;
	recurso = recursosIniciales;

	for(i=0; i<= cantRecursos; i++){
		//busco en el vector referencia la pos de ese recurso
		pos = buscarEnReferenciaRecurso(recurso->id,referenciaRecurso);
		if(pos != -1){
			//le asigno a esa pos la cantidad de recursos que hay
			recursosTotales[pos] = recurso->quantity;
		}
	}

}

void cargarRecursosDisponibles(int recursosDisponibles[], int cantRecursos , char referenciaRecurso[]){
	/*@NAME: cargarRecursosDisponibles
	* @DESC: completa el vector con la cantidad de recursos que quedan sin asignar
	*/

	int i;
	int pos =-1;

	ITEM_NIVEL* recurso;
	recurso = ListaItems;

	while(recurso!= NULL){
		//me fijo antes que sea recurso y NO personaje
		if ( (recurso!= NULL) && (recurso->item_type == 1)){
			//busco en el vector referencia la pos de ese recurso
			pos = buscarEnReferenciaRecurso(recurso->id,referenciaRecurso);
			if(pos != -1){
				//le asigno a esa pos la cantidad de recursos que hay
				recursosDisponibles[pos] = recurso->quantity;
				recurso = recurso->next;
			}
		}
		recurso = recurso->next;
	}
}



