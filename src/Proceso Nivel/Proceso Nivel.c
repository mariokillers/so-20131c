/*
 * Proceso_Nivel.c
 *
 *  Created on: 21/04/2013
 *      Author: petauriel
 */

#include "tad_items.h"
#include <stdlib.h>
#include <curses.h>
#include "Proceso Server.h"
#include "nivel.h"
#include "Proceso Cliente.h"
#include "config.c"
#include "Proceso Nivel.h"
#include "personaje_library.h"

ITEM_NIVEL* ListaItems;

int recovery;// VER SI ES GLOBAL O NO


int main(void) {

	//inicializo el nivel desde el archivo config FALTA EL PATH
	t_nivel *nivel= read_nivel_archivo_configuracion();

	//inicializo el proceso nivel
	nivel_gui_inicializar();

	//inicio al proceso como servidor y me conecto como cliente
	CCB serverCCB;
	CCB clientCCB;

	serverCCB = initServer(6000);

	clientCCB = connectServer("localhost", (nivel->nivel_orquestador).PORT); // VA EL PUERTO O EL IP?

	//inicializo el hilo que maneja interbloqueo VER ACA!
	pthread_t interbloqueo;
	pthread_create( &interbloqueo, NULL, interbloqueo, NULL );

	//inicializo la lista de items a dibujar y controlar
	extern ITEM_NIVEL* ListaItems;
	ListaItems = nivel->nivel_items;

	//inicializo la lista de personajes
	PersonajeEnNivel* listaPersonajes;

	//inicializo el recovery
	extern int recovery = nivel->nivel_recovery;

	//inicializo la cola de mensajes
	t_queue* colaDeMensajes;
	colaDeMensajes = queue_create();
	Mensaje* mensaje;


	nivel_gui_dibujar(ListaItems);

	//mientras tenga algun mensaje, ya sea de server o cliente....
	while( (mensajes(colaDeMensajes,serverCCB)) || (mensajes(colaDeMensajes, clientCCB)) ){

		//agarra de la cola de mensajes un mensaje
		mensaje = queue_pop(colaDeMensajes);


		//analiza los mensajes recibidos y en base a eso, actua
		switch(mensaje->type){

			case HANDSHAKE:

				//cargo el personaje en la lista que yo voy a manejar para validar recursos
				cargarPersonaje(&listaPersonajes, ((char*)(mensaje->from))[1]);

				//creo el personaje en el nivel. Le pongo como pos inicial la (0,0) y lo dibujo
				CrearPersonaje(&ListaItems,((char*)(mensaje->from))[1],0,0);
				nivel_gui_dibujar(ListaItems);

				//creo instancia de logeo
				t_log* logger = log_create("ProcesoNivelTestingConexiones.log", "ProcesoNivel", true, LOG_LEVEL_INFO);

				//mensajeLogeo= char del personaje que ingreso al nivel
				char mensajeLogeo = ((char*)(mensaje->from))[1];

				log_info(logger, mensajeLogeo);


				break;

			case REQUEST_POS_RECURSO:

			/**le envia al personaje la pos del nuevo recurso **/

				Posicion pos = obtenerPosRecurso(mensaje->data);

				mandarMensaje(mensaje->from, POSICION_RECURSO,sizeof(pos),pos);

				break;

			case REQUEST_MOVIMIENTO:

				//tomo del mensaje la posicion donde se va a mover el personaje
				int posx = obtenerPosX(mensaje->data);
				int posy = obtenerPosY(mensaje->data);

				//muevo el personaje y lo dibujo
				MoverPersonaje(ListaItems,((char*)(mensaje->from))[1],posx,posy);
				nivel_gui_dibujar(ListaItems);

				//modifico la posicion del personaje en listaPersonajes
				modificarPosPersonaje(&listaPersonajes,((char*)(mensaje->from))[1],posx,posy);

				sleep(1);

				break;

			case REQUEST_RECURSO:

				//le confirma al personaje que puede tomar ese recurso y lo resta de listaItems
				if(validarPosYRecursos( ((char*)(mensaje->from))[1], mensaje->data)){

					//le manda 1/TRUE porque lo puede tomar
					mandarMensaje(mensaje->from,CONFIRMAR_RECURSO,sizeof(1),1);
					restarRecurso(ListaItems, mensaje->data);
					agregarRecursoAPersonaje(&listaPersonajes, ((char*)(mensaje->from))[1],mensaje->data);
					nivel_gui_dibujar(ListaItems);

				}else{
					//si no pudo, le manda 0/FALSE porque no lo puede tomar
					mandarMensaje(mensaje->from, CONFIRMAR_RECURSO,sizeof(0),0);
				}

				break;


			/**case ENVIAR_VICTIMAS://LOGEAR ACA Y VER COMO FUNCIONA EL TEMA DEL HILO

				//...
				/**if(recovery){
					manderMensaje(orquestador, sizeof(),personajes)
				}
				break; **/

			//es avisado de que libere recursos y llama al orquestador para avisarle de liberarlos
			// y que este le diga cuales fueron re-asignados.
			case TERMINE_TURNO:

				//se fija que recursos tenia asignado el personaje para liberarlos y mandarselos al orquestador
				Recursos recursosALiberar = liberarRecursos(((char*)(mensaje->from))[1], listaPersonajes);

				//por cada recurso que libera tengo que sumarlo a la cantidad en listaItems
				aumentarRecursos(recursosALiberar); //CAMBIAR ACA

				//le mando al orquestador los recursos liberados para que re-asigne
				mandarMensaje(clientCCB.sockfd, RECURSOS_LIBERADOS,sizeof(recursosALiberar), recursosALiberar);

				//borra el personaje del nivel y libera al personaje de listaPersonajes
				BorrarItem(&ListaItems,((char*)(mensaje->from))[1]);
				borrarPersonaje(&listaPersonajes,((char*)(mensaje->from))[1]);

				//re-dibuja el nivel ya sin el personaje
				nivel_gui_dibujar(ListaItems);

				break;

			//es avisado de que reasigne los recursos que tomaron los personajes que antes estaban bloqueados
			case RECURSOS_REASIGNADOS:
				break;


			case NOMBRE_VICTIMA: //LOGEO QUE HUBO RECOVERY Y PONGO EL NOMBRE DE LA VICTIMA. TIPO DE DATO: CHAR

				//creo instancia de logeo.
				t_log* logger = log_create("ProcesoNivelTestingVictimas.log", "ProcesoNivel", true, LOG_LEVEL_INFO);

				char mensajeLogeo= mensaje->data;

				char mensajeLogeo = mensaje->data;

				log_info(logger, mensajeLogeo);


				break;

			//borro el mensaje
			borrarMensaje(mensaje);

		}
	}

	//nivel_gui_terminar();

	//cierro el socket del cliente
	close(clientCCB.sockfd);

}

Posicion obtenerPosRecurso(char recurso){

/** @NAME: obtenerPosRecurso
 * @DESC: devuelve la pos en la que se encuentra el recurso
*/
	ITEM_NIVEL * itemRecurso;
	itemRecurso = ListaItems;

	while( (itemRecurso != NULL) && (itemRecurso->id != recurso)){
		itemRecurso = itemRecurso->next;
	}if ((itemRecurso != NULL) && (itemRecurso->id == recurso)){
		Posicion posicion = Pos(itemRecurso->posx, itemRecurso->posy);
	}
	return posicion;
}

int validarPosYRecursos(char idPersonaje, char idRecurso){
	/** @NAME: validarPosYRecursos
	* @DESC: Verifico que el personaje este sobre el recurso que dice estar y a su vez veo si hay instancias del mismo para
	* asignarle al personaje.
	**/

	//busco la posicion del personaje en el mapa.
	t_list auxList = ListaItems;
	ITEM_NIVEL* personaje = list_find(auxList, (ListaItems->id == idPersonaje));
	//Posicion posPersonaje = Pos(personaje->posx, personaje->posy);

	//busco la posicion del recurso en el mapa y la cantidad de recursos que tiene
	t_list auxList = ListaItems;
	ITEM_NIVEL* recurso = list_find(auxList, (ListaItems->id == idRecurso));
	//Posicion posRecurso = Pos(recurso->posx, recurso->posy);

	//comparo
	if( ((personaje->posx == recurso->posx) && (personaje->posy == recurso->posy)) && ( (recurso->quantity)>0) ){
		return 1;
		break;
	}
	return 0;
}

void cargarPersonaje(PersonajeEnNivel** listaPersonajes, char id){
	/*@NAME: cargarPersonaje
	 * @DESC: cuando se conecta un personaje al nivel, lo agrega a la listaPersonajes que es la lista para la cual el nivel
	 * tiene actualizado los recursos que ese personaje posee en el nivel
	 */

	PersonajeEnNivel* personaje;
	personaje = malloc(sizeof(PersonajeEnNivel));

	personaje->id = id;

	Posicion pos = Pos(0,0);
	personaje->pos = pos;

	personaje->recursos = NULL;

	personaje->sig = *listaPersonajes;
	*listaPersonajes = personaje;


}

void agregarRecursoAPersonaje(PersonajeEnNivel** listaPersonajes, char idPersonaje,char recurso){
	/*@NAME: agregarRecursoAPersonaje
	 * @DESC: cuando le confirman al personaje que puede tomar el recurso, el nivel agrega a la listaPersonajes, en el
	 * personaje, el recurso que obtuvo
	*/

	PersonajeEnNivel * personaje;
	personaje = listaPersonajes;

	//busco el personaje
	while ((personaje != NULL) && (personaje->id != idPersonaje)) {
		personaje = personaje->sig;
	}if ((personaje != NULL) && (personaje->id == idPersonaje)) {
		t_recursos * auxList ;
		auxList = personaje->recursos;

		//busco el recurso
		while( (auxList->sig != NULL) && (auxList->idRecurso != recurso) ){
			auxList = auxList->sig;
		}if( auxList->idRecurso == recurso ){
			//si lo encontro, le suma 1 a la cant que ya tenia.
			personaje->recursos->cant = personaje->recursos->cant++ ;

		}else if((auxList->sig == NULL)){
			//si no lo encontro, lo agrega

			t_recursos * temp;
			temp = malloc(sizeof(t_recursos));

			temp->idRecurso = recurso;
			temp->cant = 1;
			temp->sig = NULL;

			personaje->recursos = temp;
			*listaPersonajes = personaje;
		}
	}
}

void borrarPersonaje(PersonajeEnNivel** listaPersonajes, char idPersonaje){
	/*@NAME: borrarPersonaje
	 * @DESC: cuando un personaje termina el nivel, lo borra de listaPersonajes
	*/

	PersonajeEnNivel * personaje = *listaPersonajes;
	PersonajeEnNivel * personajeAnterior;

    if ((personaje != NULL) && (personaje->id == idPersonaje)) {
    	*listaPersonajes = (*listaPersonajes)->sig;
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

//CAMBIAR LA FUNCION
Recursos liberarRecursos(char idPersonaje,PersonajeEnNivel* listaPersonajes ){
	/*@NAME: liberarRecursos
	 * @DESC: cuenta los recursos a ser liberados
	*/

	PersonajeEnNivel * personaje;
	personaje = listaPersonajes;

	//instancio los contadores
	int cantFlores = 0;
	int cantMonedas = 0;
	int cantHongos = 0;

 	//busco el personaje en la lista
	while ((personaje != NULL) && (personaje->id != idPersonaje)){
		personaje = personaje->sig;
	}if((personaje != NULL) && (personaje->id == idPersonaje)){

		//recorro todos los recursos
		while(personaje->recursos != NULL){
			char recurso = ((t_recursos*)(personaje->recursos))->idRecurso;

			//me fijo que recurso es y lo sumo al contador de ese recurso
			switch(recurso){

			case 'H':

				cantHongos++;

				break;

			case 'F':

				cantFlores++;

				break;

			case 'M':

				cantMonedas++;

				break;
			}
		}
	}

	//lo paso al tipo de datos Recursos que es lo que voy a terminar devolviendo
	Recursos recursos = CantRecursos(cantFlores,cantHongos,cantMonedas);
	return(recursos);

}

//CAMBIAR LA FUNCION
void aumentarRecursos(Recursos recursosALiberar){
	/*@NAME: aumentarRecursos
	 * @DESC: actualiza el estado de los recursos en ListaItems, dependiendo de los recursos liberados
	*/

	 ITEM_NIVEL * aux;
	 aux = ListaItems;

	 int cantFlores = recursosALiberar.cant_flor;
	 int cantHongos = recursosALiberar.cant_hongo;
	 int cantMonedas = recursosALiberar.cant_moneda;

	 while(aux!= NULL){

		 //chequeo que se trate de un recurso. Si es recurso es 1, si es personaje es 0

		 if(aux->item_type){

			 //me fijo de que recurso se trata y sumo cantidades a liberar donde corresponda

			 switch(aux->id){

			 case 'H':

				 aux->quantity = aux->quantity + cantHongos;

				break;

			 case 'F':

				 aux->quantity = aux->quantity + cantFlores;

			 	break;

			 case 'M':

				 aux->quantity = aux->quantity + cantMonedas;

			 	break;


			 }
		 }

	 }

}

void modificarPosPersonaje(PersonajeEnNivel** listaPersonajes,char idPersonaje, int posx, int posy){
	/*@NAME: modificarPosPersonaje
	* @DESC: actualiza la pos del personaje
	*/

	PersonajeEnNivel * personaje;
	personaje = listaPersonajes;

	while ((personaje != NULL) && (personaje->id != idPersonaje)) {
		personaje = personaje->sig;
	}if ((personaje != NULL) && (personaje->id == idPersonaje)) {
		personaje->pos = Pos(posx,posy);
	}

}

