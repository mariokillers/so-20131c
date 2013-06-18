#include "nivel.h"
#include <stdio.h>
#include <string.h>

//inicializo la lista de personajes para controlar, dibujar e interbloqueo (en orden)
PersonajeEnNivel* listaPersonajes;
ITEM_NIVEL* ListaItems;
ITEM_NIVEL* recursosIniciales; //lista para tratar interbloqueo

//inicio al proceso como servidor y me conecto como cliente
CCB serverCCB;
CCB clientCCB;

int recovery;

//inicializo el semaforo MUTEX (VER PORQUE NO FUNCA EL INIT)
pthread_mutex_t mutex;
//pthread_mutex_init(&mutex, NULL);

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

	Direccion * dir = nivel.nivel_orquestador;
	char ip[20];
	strcpy(ip,"localhost");
	clientCCB = connectServer("localhost",dir->PORT);

	log_info(logger, string_from_format("El nivel se conecto al puerto: %d", dir->PORT));

	//le mando handshake al orquestador
	strcpy(yoNivel.ID,nivel->nivel_nombre);
	strcpy(yoNivel.IP,"localhost");
	yoNivel.PORT = puerto;
	mandarMensaje(clientCCB.sockfd, HANDSHAKE,sizeof(Nivel),&yoNivel); 
	log_info(logger, string_from_format("El nivel: %s le hizo HANDSHAKE al socket: %d", yoNivel.ID, clientCCB.sockfd));

	//inicializo el hilo que maneja interbloqueo VER ACA!
	pthread_t thread_interbloqueo;
	pthread_create( thread_interbloqueo, NULL, &interbloqueo, NULL );

	//inicializo el recovery
	recovery = nivel->nivel_recovery;
	if(recovery)
		log_info(logger, "El recovery esta activado");


	//inicializo la cola de mensajes
	t_queue* colaDeMensajes;
	colaDeMensajes = queue_create();
	Mensaje* mensaje;

	//entro en la region critica
	pthread_mutex_lock(mutex);

	//inicializo la lista de items a dibujar y controlar
	ListaItems = nivel->nivel_items;
	recursosIniciales = nivel->nivel_items; //esta lista es para saber la cantidad total de recursos que hay en el nivel (interbloqueo)

	nivel_gui_dibujar(ListaItems);

	pthread_mutex_unlock(mutex);
	//salgo de la region critica


	log_info(logger, "Terminado dibujar nivel");

	while(1){
		while(!mensajes(colaDeMensajes,serverCCB)); 
		log_info(logger, "Buscando mensaje en cola");
		//mientras tenga algun mensaje, ya sea de server o cliente, agarra de la cola de mensajes un mensaje
		mensaje = queue_pop(colaDeMensajes);
		log_info(logger, string_from_format("Recibi mensaje de tipo: %d", mensaje->type));

		//analiza los mensajes recibidos y en base a eso, actua
		switch(mensaje->type){

			case HANDSHAKE:

				Personaje* personaje;
				personaje = mensaje->data;
				personaje->FD = mensaje->from;

				log_info(logger, string_from_format("Recibi HANDSHAKE del personaje: %s", personaje->ID));

				//entro en la region critica
				pthread_mutex_lock(mutex);

				//cargo el personaje en las listas que voy a manejar para validar recursos e interbloqueo
				PersonajeEnNivel* miPersonaje = cargarPersonajeEnNivel(personaje);

				//creo el personaje en el nivel. Le pongo como pos inicial la (0,0) y lo dibujo
				CrearPersonaje(&ListaItems,miPersonaje->id,0,0);
				nivel_gui_dibujar(ListaItems);

				pthread_mutex_unlock(mutex);
				//salgo de la region critica

				log_info(logger, string_from_format("Se dibujo el personaje: %s en la pos (0,0)", personaje->ID));

				break;

			case REQUEST_POS_RECURSO:
			{
				//le envia al personaje la pos del nuevo recurso

				log_info(logger, "Recibi REQUEST_POS_RECURSO");

				char recurso = ((char)mensaje->data);

				//entro en la region critica
				pthread_mutex_lock(mutex);

				//obtengo la posicion del recurso
				Posicion pos =  obtenerPosRecurso(recurso);

				log_info(logger, string_from_format("Mando la posicion del recurso %s: (%d,%d)",recurso , pos.POS_X, pos.POS_Y ));

				pthread_mutex_unlock(mutex);
				//salgo de la region critica

				mandarMensaje(mensaje->from, POSICION_RECURSO,sizeof(Posicion),&pos);

				log_info(logger, string_from_format("Mande la posicion del recurso: %s al personaje: %s ",recurso , personaje->ID));
			}
				break;

			case REQUEST_MOVIMIENTO:
			{
				log_info(logger, "Recibi REQUEST_MOVIMIENTO");

				//entro en la region critica
				pthread_mutex_lock(mutex);

				//tomo del mensaje la posicion donde se va a mover el personaje
				Posicion* pos = ((Posicion*)mensaje->data);
				
				int posx = pos->POS_X;
				int posy = pos->POS_Y;

				PersonajeEnNivel* personaje = buscarPersonaje_byfd(mensaje->from);

				log_info(logger, string_from_format("Tomo la posicion del recurso:(%d,%d) que me solicito personaje: %s", posx,posy, personaje->id));

				//muevo el personaje y lo dibujo
				MoverPersonaje(ListaItems,personaje->id,posx,posy);
				nivel_gui_dibujar(ListaItems);

				//modifico la posicion en la listaPersonajes
				modificarPosPersonaje(personaje, posx, posy);

				pthread_mutex_unlock(mutex);
				//salgo de la region critica

				log_info(logger, string_from_format("Movi el personaje: %s a la posicion:(%d,%d)", personaje->id,posx,posy));

			}
				break;

			case REQUEST_RECURSO:
			{

				log_info(logger, "Recibi REQUEST_RECURSO");

				char recurso = (char*)mensaje->data;

				//entro en la region critica
				pthread_mutex_lock(mutex);

				PersonajeEnNivel* personaje = buscarPersonaje_byfd(mensaje->from);

				log_info(logger, string_from_format("El personaje: %s me pidio el recurso:%s", personaje->id,recurso));

				//le confirma al personaje que puede tomar ese recurso y lo resta de listaItems
				if(validarPosYRecursos( personaje, recurso)){

					log_info(logger, string_from_format("El personaje:%s pudo obtener el recurso:%s",personaje->id, recurso));

					//le manda 1/TRUE porque lo puede tomar
					mandarMensaje(mensaje->from, CONFIRMAR_RECURSO,sizeof(bool),1);
					restarRecurso(ListaItems, recurso);
					agregarRecursoAPersonaje(personaje,recurso);

					nivel_gui_dibujar(ListaItems);

				}else{

					log_info(logger, string_from_format("El personaje:%s no pudo obtener el recurso:%s",personaje->id, recurso));

					//si no pudo, le manda 0/FALSE porque no lo puede tomar
					mandarMensaje(mensaje->from, CONFIRMAR_RECURSO,sizeof(0),0);

					//actualiza la lista de recursos pendientes
					agregarARecursosPendientes(personaje, recurso);

					log_info(logger, string_from_format("Se agrego la solicitud del recurso:%s del personaje:%s a solicitudes pendientes", recurso, personaje->id));
				}

				pthread_mutex_unlock(mutex);
				//salgo de la region critica


				break;
			}

			//es avisado de que libere recursos y llama al orquestador para avisarle de liberarlos
			// y que este le diga cuales fueron re-asignados.
			case TERMINE_NIVEL:
			{
				log_info(logger, "Recibi TERMINE_NIVEL");

				//entro en la region critica
				pthread_mutex_lock(mutex);

				PersonajeEnNivel* personaje = buscarPersonaje_byfd(mensaje->from);

				log_info(logger, string_from_format("El personaje: %s me avisa que termino el nivel", personaje->id));

				//se fija que recursos tenia asignado el personaje para liberarlos
				t_recursos* recursosALiberar = liberarRecursos(personaje);

				//le manda los recursos liberados, de a 1, al orquestador
				mandarRecursosLiberados(recursosALiberar,clientCCB.sockfd);

				log_info(logger, "Mando recursos liberados");

				//por cada recurso que libera tengo que sumarlo a la cantidad en listaItems
				aumentarRecursos(recursosALiberar);

				log_info(logger, "Aumento los recursos re-asignados");

				//borra el personaje del nivel y libera al personaje de listaPersonajes
				BorrarItem(&ListaItems,personaje->id);
				borrarPersonajeEnNivel(personaje->id);

				log_info(logger, string_from_format("El personaje: %s ha sido borrado del nivel", personaje->id));

				//re-dibuja el nivel ya sin el personaje y con la cantidad de recursos nueva
				nivel_gui_dibujar(ListaItems);

				pthread_mutex_unlock(mutex);
				//salgo de la region critica
			}
				break;


			case NOMBRE_VICTIMA:
			{
				log_info(logger, "Recibi NOMBRE_VICTIMA");

				//loggeo la victima de interbloqueo

				char idVictima = (char*)mensaje->data;

				log_info(logger, string_from_format("El personaje: %s ha sido elegido como victima del interbloqueo", idVictima));

				//entro en la region critica
				pthread_mutex_lock(mutex);

				//TENO QUE LLAMAR A LO QUE TIENE EL CASE TERMINE_NIVEL!

				pthread_mutex_unlock(mutex);
				//salgo de la region critica

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
	Recursos* recurso = listaRecursos;

	restarRecurso(ListaItems, recurso->idRecurso); //resta de la lista items el recurso que re-asigno

	PersonajeEnNivel* personaje = buscarPersonaje_byfd(recurso->idPersonaje);
	agregarRecursoAPersonaje (personaje, recurso->idRecurso); // le agrega a listaPersonajes el recurso que obtuvo el personaje

}

void mandarRecursosLiberados(t_recursos* recursosALiberar, int fdOrquestador){
	/** @NAME: mandarRecursosLiberados
	 * @DESC: le manda al orquestador la cantidad de recursos que se liberaron POR RECURSO
	 */
	t_recursos* aux = recursosALiberar;

	t_queue* colaDeMensajes;
	colaDeMensajes = queue_create();
	Mensaje* mensaje;

//ACA PINCHA SEGURO

	while(aux != NULL){
		//paso a la struct a la que voy a mandar los mensajes
		Recursos  recurso;
		recurso.idRecurso = aux->idRecurso;
		recurso.idPersonaje = '\0';
		recurso.cant = aux->cant;

		//le mando al orquestador los recursos liberados para que re-asigne
		mandarMensaje(fdOrquestador, RECURSOS_LIBERADOS,sizeof(Recursos),&recurso);

		//escucho al orquestador que me va a mandar los que re-asigno
		while((!mensajes(colaDeMensajes,serverCCB)));
			mensaje = queue_pop(colaDeMensajes);

			//mientras no sea el mensaje REASIGNACION_FINALIZADA... quiere decir que me esta mandando re-asignaciones

			while(mensaje->type !=REASIGNACION_FINALIZADA ){
				if(mensaje->type == RECURSOS_REASIGNADOS){

					//llamo a reasignar con la data que me envio
					Recursos* listaRecursos = mensaje->data;
					reasignarRecursos(listaRecursos);

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

	ITEM_NIVEL* itemRecurso = buscarItem(recurso);

	Posicion posicion = Pos(itemRecurso->posx, itemRecurso->posy);
	return posicion;

}

int validarPosYRecursos(PersonajeEnNivel* personaje, char idRecurso){
	/** @NAME: validarPosYRecursos
	 * @DESC: Verifico que el personaje este sobre el recurso que dice estar y a su vez veo si hay instancias del mismo para
	 * asignarle al personaje.
	 */

	//busco la posicion del personaje en el mapa

	int personajePosx = (personaje->pos).POS_X;
	int personajePosy = (personaje->pos).POS_Y;

	//busco la posicion del recurso en el mapa y la cantidad de recursos que tiene
	ITEM_NIVEL* recurso = buscarItem(idRecurso);

	log_info(logger, string_from_format("La posicion del personaje: %s es:(%d,%d) y la posicion del recurso: %s es:(%d,%d) y su cantidad: %d",personaje->id,personajePosx,personajePosy ,idRecurso, recurso->posx, recurso->posy, recurso->quantity));

	//comparo
	if( ((personajePosx == recurso->posx) && (personajePosy == recurso->posy)) && ( (recurso->quantity)>0) ){
		log_info(logger, "El personaje puede tomar el recurso");
		return 1;
	}
	log_info(logger, "El personaje no puede tomar el recurso");
	return 0;
}

ITEM_NIVEL* buscarItem(char id){

	ITEM_NIVEL* temp;
	temp = ListaItems;

	while((temp != NULL) && (temp->id != id)){
		temp = temp->next;
	}if((temp != NULL) && (temp->id == id)){
		return temp;
	}return NULL;
}


PersonajeEnNivel* cargarPersonajeEnNivel(Personaje* miPersonaje){
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

	personaje->recursoPendiente = '\0';

	personaje->marcado = false;

	personaje->sig = listaPersonajes;
	listaPersonajes = personaje;

	return personaje;

	//free(personaje);


}


void agregarARecursosPendientes(PersonajeEnNivel* personaje, char recurso){
	/*@NAME: agregarAListaRecursosPendientes
	 * @DESC: actualiza la lista de recursos pendientes del personaje para luego utilizarla en el interbloqueo
	 */

	if( personaje != NULL){
		personaje->recursoPendiente = recurso;
	}
}

void quitarSolicitudesDeRecurso(PersonajeEnNivel* personaje, char idRecurso){
	/*@NAME: quitarSolicitudesDeRecurso
	 * @DESC: quita de la lista de recursos pendientes el recurs que obtuvo el personaje tras la re-asignacion
	*/

	if(personaje->recursoPendiente == idRecurso){
		personaje->recursoPendiente = '\0';
	}
}

void agregarRecursoAPersonaje(PersonajeEnNivel* personaje, char recurso){
	/*@NAME: agregarRecursoAPersonaje
	 * @DESC: cuando le confirman al personaje que puede tomar el recurso, el nivel agrega a la listaPersonajes, en el
	 * personaje, el recurso que obtuvo
	*/

	//PersonajeEnNivel* aux = personaje;

	//si es el recurso que tenia pendiente, lo borra
	if(personaje->recursoPendiente == recurso){

		quitarSolicitudesDeRecurso(personaje, recurso);
	}

	t_recursos* auxList = personaje->recursos;

	//busco el recurso
	while( (auxList->sig != NULL) && (auxList->idRecurso != recurso) ){
		auxList = auxList->sig;
	}if( auxList->idRecurso == recurso ){
		//si lo encontro, le suma el recurso a la cant que ya tenia
		personaje->recursos->cant++;

	}else if((auxList->sig == NULL)){

		//si no lo encontro, lo agrega en la lista de recursos del personaje

		t_recursos* temp;
		temp = malloc(sizeof(t_recursos));

		temp->idRecurso = recurso;
		temp->cant = 1;
		temp->sig = personaje->recursos;

		personaje->recursos = temp;

		//free(temp);
	}
}

void borrarPersonajeEnNivel(char idPersonaje){
	/*@NAME: borrarPersonaje
	* @DESC: cuando un personaje termina el nivel, lo borra de listaPersonajes
	*/

	PersonajeEnNivel* personaje = listaPersonajes;
	PersonajeEnNivel* personajeAnterior;



    if ((personaje != NULL) && (personaje->id == idPersonaje)) {
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


t_recursos* liberarRecursos(PersonajeEnNivel* personaje ){
	/*@NAME: liberarRecursos
	 * @DESC: me devuelve la lista de recursos del personaje que termino el nivel
	*/

 	if((personaje != NULL)){
 		return personaje->recursos;
	}return NULL;

}

void aumentarRecursos(t_recursos* recursosALiberar){
	/*@NAME: aumentarRecursos
	 * @DESC: actualiza el estado de los recursos en ListaItems, dependiendo de los recursos liberados
	*/

	t_recursos* aux = recursosALiberar;

	 while(aux!= NULL){

		 //me fijo de que recurso se trata y sumo cantidades a liberar donde corresponda

		 agregarRecursosAListaItems(aux->idRecurso, aux->cant);

		 aux = aux->sig;
	 }

}

void agregarRecursosAListaItems(char idRecurso, int cant){
	/*@NAME: agregarRecursosAListaItems
	 * @DESC: dado un id de recurso, lo busco en la lista Items y le sumo la cantidad que libero
	*/

	ITEM_NIVEL* temp = ListaItems;

	while( (temp!= NULL) && (temp->id != idRecurso)){
		temp= temp->next;
	}if((temp != NULL) && (temp->id == idRecurso)){
		temp->quantity = temp->quantity + cant;
	}
}

void modificarPosPersonaje(PersonajeEnNivel* personaje, int posx, int posy){
	/*@NAME: modificarPosPersonaje
	* @DESC: actualiza la pos del personaje
	*/

	log_info(logger, string_from_format("Modifico posicion personaje: %s en posicion (%d,%d)", personaje->id, posx, posy));

	if (personaje != NULL){
		personaje->pos = Pos(posx,posy);
	}

}

PersonajeEnNivel* buscarPersonaje_byfd(int fd){
	/*@NAME: buscarPersonaje
	* @DESC: devuelve un PersonajeEnNivel que tenga ese id
	*/
	PersonajeEnNivel* personaje = listaPersonajes;

	//busco el personaje
	while ((personaje != NULL) && (personaje->fd != fd)) {
		personaje = personaje->sig;
	}if ((personaje != NULL) && (personaje->fd == fd)){
		return personaje;
	}
	return NULL;

}

void* interbloqueo(void* a){
	/*@NAME: interbloqueo
	* @DESC: hilo que se encarga de detectar interbloqueo
	*/

	//VER TEMA DE MALLOC ACA

	log_info(logger, "Empieza a ejecutar el hilo interbloqueo");

	//entro en la region critica
	pthread_mutex_lock(mutex);

	log_info(logger, "El hilo interbloqueo entra en la region critica");

	int cantPersonajes = cantidadPersonajes();
	int cantRecursos = cantidadRecursos();

	log_info(logger, string_from_format("La cantidad de personajes es: %d y de recursos es: %d", cantPersonajes, cantRecursos));

	//vector para saber que procesos estan interbloqueados
	bool marcados[cantPersonajes];

	inicializarMarcados (marcados, cantPersonajes);

	//vectores que referencian en la posicion de matrices y vectores para detectar interbloqueo
	char referenciaPersonaje[cantPersonajes];
	char referenciaRecursos[cantRecursos];

	inicializarReferenciaRecurso(cantidadRecursos, referenciaRecursos);
	inicializarReferenciaPersonaje(cantPersonajes, referenciaPersonaje);

	//vectores para interbloqueo
	int recursosTotales[cantRecursos];
	int recursosDisponibles[cantRecursos];

	//matrices para interbloqueo
	int recursosAsignados[cantPersonajes][cantRecursos];
	int recursosSolicitados[cantPersonajes][cantRecursos];

	int aux[cantRecursos] = recursosDisponibles;

	//inicializo los vectores-matrices
	cargarRecursosTotales(recursosTotales, cantRecursos, referenciaRecursos);
	cargarRecursosDisponibles(aux, referenciaRecursos);
	cargarRecursosSolicitados(recursosSolicitados, referenciaRecursos, referenciaPersonaje);
	cargarRecursosAsignados(recursosAsignados, referenciaRecursos, referenciaPersonaje);

	pthread_mutex_unlock(mutex);
	//salgo de la region critica

	log_info(logger, "El hilo interbloqueo sale de la region critica");


	marcarPersonajesSinRecursos(recursosAsignados,referenciaPersonaje,marcados,cantPersonajes, cantRecursos);
	marcarPersonajesConRecursos(recursosAsignados, recursosSolicitados, recursosDisponibles, marcados,cantPersonajes, cantRecursos);
	comprobarDeadlock(marcados,cantPersonajes, referenciaPersonaje);

	return 0;
}

void inicializarMarcados (bool marcados[], int cantidadPersonajes){
	/*@NAME: inicializarMarcados
	* @DESC: inicializo  el vector en false
	*/

	int i;
	for(i=0;i<= cantidadPersonajes; i++){
		marcados[i]= false;
	}
}

void inicializarReferenciaRecurso(int cantidadRecursos, char referenciaRecurso[]){
	/*@NAME: inicializarMarcados
	* @DESC: inicializo  el vector de referencia de recursos con los recursos
	*/

	int i;
	ITEM_NIVEL* recurso = recursosIniciales;

	for(i=0;i<= cantidadRecursos; i++){
			referenciaRecurso[i]= recurso->id;
			recurso = recurso->next;
	}
}

void inicializarReferenciaPersonaje(int cantidadPersonajes, char referenciaPersonaje[]){
	/*@NAME: inicializarReferenciaPersonaje
	* @DESC: inicializo  el vector de referencia de personaje con los personajes que hay
	*/

	int i;
	PersonajeEnNivel* personaje = listaPersonajes;

	for(i=0;i<= cantidadPersonajes; i++){
		referenciaPersonaje[i]= personaje->id;
		personaje = personaje->sig;
	}
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
		}else{
			i++;
		}
	}return i;

}

int buscarEnReferenciaPersonaje(char idPersonaje, char referenciaPersonaje[]){
	/*@NAME: buscarEnReferenciaProceso
	* @DESC: busca en el vector que hace referencia a los personajes la pos de ese personaje en las matrices
	*/
	int i=0;
	bool encontrado = false;
	while(!encontrado){
		if(referenciaPersonaje[i] == idPersonaje){
			encontrado = true;
		}else{
			i++;
		}
	}return i;
}


int cantidadPersonajes(){
	/*@NAME: cantidadProcesos
	* @DESC: devuelve la cantidad de personajes conectados al nivel
	*/
	int i =0;
	PersonajeEnNivel* personaje = listaPersonajes;

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
	ITEM_NIVEL* recurso = recursosIniciales;

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

	ITEM_NIVEL* recurso = recursosIniciales;

	for(i=0; i<= cantRecursos; i++){
		//busco en el vector referencia la pos de ese recurso
		pos = buscarEnReferenciaRecurso(recurso->id,referenciaRecurso);
		if(pos != -1){
			//le asigno a esa pos la cantidad de recursos que hay
			recursosTotales[pos] = recurso->quantity;
		}
		recurso = recurso->next;
	}

}

void cargarRecursosDisponibles(int recursosDisponibles[], char referenciaRecurso[]){
	/*@NAME: cargarRecursosDisponibles
	* @DESC: completa el vector con la cantidad de recursos que quedan sin asignar
	*/

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

void cargarRecursosSolicitados(int recursosSolicitados[][], char referenciaRecurso[], char referenciaPersonaje[]){
	/*@NAME: cargarRecursosSolicitados
	* @DESC: carga la matriz dependiendo de el recurso solicitado que tuvo cada personaje
	*/

	PersonajeEnNivel* personaje = listaPersonajes;
	char recurso;

	int posPersonaje = -1;
	int posRecurso = -1;

	while(personaje != NULL){
		//cargo el recurso que solicito el personaje
		recurso = personaje->recursoPendiente;

		//busco la posicion del personaje y el recurso en el vector de referencia
		posPersonaje = buscarEnReferenciaPersonaje(personaje->id,referenciaPersonaje );
		posRecurso = buscarEnReferenciaRecurso(recurso, referenciaRecurso);

		if((posRecurso != -1) && (posPersonaje != -1)){

			//en la fila del personaje, en la columna del recurso, pongo un 1 que es el recurso que solicito
			recursosSolicitados[posPersonaje][posRecurso]= 1;
		}
		personaje = personaje->sig;
	}
}

void cargarRecursosAsignados(int recursosAsignados[][], char referenciaRecurso[], char referenciaPersonaje[]){
	/*@NAME: cargarRecursosAsignados
	* @DESC: carga la matriz dependiendo de los recursos que tiene asignado cada personaje
	*/

	int posPersonaje = -1;
	int posRecurso = -1;

	PersonajeEnNivel* personaje;
	personaje = listaPersonajes;

	while(personaje != NULL){
		posPersonaje = buscarEnReferenciaPersonaje(personaje->id,referenciaPersonaje );
		//recorro la lista de recursos de ese personaje
		t_recursos* recurso;
		recurso = personaje->recursos;

		while(recurso != NULL){
			//busca la posicion en la matriz del char de ese recurso
			posRecurso = buscarEnReferenciaRecurso(recurso->idRecurso, referenciaRecurso);
			//en la fila del personaje, la columna del recurso, le asigna la cantidad que tiene asignado ese personaje
			recursosAsignados[posPersonaje][posRecurso] = recurso->cant;
			//paso al siguiente recurso del personaje
			recurso = recurso->sig;
		}
		//paso al otro personaje
		personaje = personaje->sig;
	}
}

void marcarPersonajesSinRecursos (int recursosAsignados[][], char referenciaPersonaje[], bool marcados[], int cantPersonajes, int cantRecursos){
	/*@NAME: marcarPersonajesSinRecursos
	* @DESC: marca a los personajes que no tienen recursos asignados
	*/

	int i,j;
	for(i=0;i<cantPersonajes;i++){
		int flag=0;
		for(j=0;j<cantRecursos;j++){
			if(recursosAsignados[i][j]!=0){
				flag=1;
			}
		}
		if (flag==1){
			marcados[i]=true;
		}
	}

}


void marcarPersonajesConRecursos (int recursosAsignados[][], int recursosSolicitados[][], int recursosDisponibles[], bool marcados[], int cantPersonajes, int cantRecursos){
	/*@NAME: marcarPersonajesConRecursos
	* @DESC: marca a los personajes que pueden ejecutar
	*/
	int i,j,asignacionImposible, flagTerminar;
	do{
		flagTerminar=0;
		//recorremos personajes
		for(i=0;i<cantPersonajes;i++){
			asignacionImposible=0;

			//recorremos recursos del personaje actual
			for(j=0;j<cantRecursos;j++){
				//verifico que haya recursos susficientes para satisfacer el pedido
				if(marcados[i]==false && recursosSolicitados[i][j]<=recursosDisponibles[j]){
					asignacionImposible=1;
				}
			}

			//es posible ejecutar el personaje
			if(!asignacionImposible){
				//SI ENCUENTRA UNO QUE PUEDA EJECUTAR, SETEA PARA CONTINUAR EL ALGORTIMO
				flagTerminar=1;
				marcados[i]=true;
				//si se puede ejecutar, actualizo el disponible
				for(j=0;j<cantRecursos;j++){
					recursosDisponibles[j]+=recursosAsignados[i][j];
				}
			}

		}
	//si se encontro, termina el algoritmo
	}while(flagTerminar);

}


void comprobarDeadlock (bool marcados[],int cantPersonajes, char referenciaPersonaje[]){
	//CHEQUEAR INOTIFY

	int i,j;
	j=0;
	char personajesInterbloqueados[cantPersonajes+1];
	//recorremos el vector de marcados
	for(i=0;i<cantPersonajes;i++){
		if(marcados[i]==false){
			//Si el personaje no esta marcado, esta comprometido en un deadlock.
			personajesInterbloqueados[j]=referenciaPersonaje[i];
			j++;

		}
	}
	personajesInterbloqueados[j]='\0';

	if(recovery && personajesInterbloqueados[0]!='\0'){
		mandarMensaje(clientCCB.sockfd,REQUEST_INTERBLOQUEO,strlen(personajesInterbloqueados)+1,personajesInterbloqueados);
	}
}

