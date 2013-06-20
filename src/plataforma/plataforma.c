#include "plataforma.h"
#include <string.h>

t_list* Gestores;
t_log* Logger;
int quantum_inicial;
t_list* personajes_jugando;
int main (){

	Gestores = list_create();
	personajes_jugando = list_create();
	Logger = log_create("ProcesoPlataforma.log", "ProcesoPlataforma", true, LOG_LEVEL_INFO);
	log_info(Logger, "************************************************************************");
	log_info(Logger, "Se inicia Proceso Plataforma.");
	
	//TEST 1
	/*
	GestorNivel* test;
	GestorNivel* test2;
	char buf[10];
	for (i=0; i<10; i++){
	test = malloc (sizeof(GestorNivel));
	sprintf(buf,"%d",i);
	strcat(buf, "hola");
	strcpy(test->ID , buf);
	printf("%s",test->ID);
	list_add(Gestores, test);
	}

	test2 = findGestor_byid("4hola");
	printf("%s",test2->ID);
	 */
	pthread_t orquestador;
	quantum_inicial=3;
	pthread_create( &orquestador, NULL, orq, NULL );
	pthread_join(orquestador,NULL);
	log_info(Logger, "Crea Thread Orquestador.");

	return 0;
}
/*
Elemento_personaje* personaje_crear(char* nombre) {
	Elemento_personaje* personaje = mallot_list* c(sizeof(Elemento_personaje));
	strncpy(personaje->nombre, nombre, 24);
	return personaje;
}
 */


void* Planif(void* nivel){

	log_info(Logger, "Inicia Planificador.");

	GestorNivel* miGestor;
	Nivel* miNivel = (Nivel*)nivel;
	t_queue* misMensajes;
	misMensajes = queue_create();
	Mensaje* miMensaje;
	CCB miCON;
	
	//CREO LA INSTANCIA DEL GESTOR
	if((miGestor= malloc(sizeof(GestorNivel)))==NULL){
		exit(1);
	}
	
	//Inicializo flag de turno y quantum
	miGestor->turno_entregado=0;
		miGestor->quantum = quantum_inicial;

	//Creo las colas del nivel
	log_info(Logger, "Crea cola de Listos y lista de bloqueados.");
	miGestor->queue_listos = queue_create();
	miGestor->queues_bloq = list_create();
	
	//Creo personajes en nivel
	miGestor->personajes_en_nivel = list_create();

	//Asigno ID del gestor ="GX"
	strcpy(miGestor->ID, miNivel->ID);
	//miGestor->ID[0]='G';

	//Asigno ID="PXXXX"
	strcpy(miGestor->dataPlanificador.ID, miNivel->ID);
	miGestor->dataPlanificador.ID[0]='P';
	//Los puertos de los planificadores son 5501 5502 5503...
	miGestor->dataPlanificador.PORT= miNivel->PORT+1000;
	strcpy(miGestor->dataPlanificador.IP, "localhost");

	//Copio la instancia de dataNivel
	memcpy(&(miGestor->dataNivel),miNivel, sizeof(Nivel));
	free(miNivel);

	//Agrego el gestor a la lista de gestores.
	list_add (Gestores, miGestor);

	miCON = initServer(miGestor->dataPlanificador.PORT);

	log_info(Logger, "Crea e inicializa las variables y colecciones necesarias del Planificador.");
	
	
	while(1){
		
		if(mensajes(misMensajes ,miCON)){
			miMensaje = queue_pop(misMensajes);
			switch(miMensaje->type){
			case HANDSHAKE:
				//CONEXION DE P	ERSONAJE
				log_info(Logger, "Recibe mensaje de conexion de Proceso Personaje.");
				if(((char*)miMensaje->data)[0]=='P'){
					//Inicializo la instancia de personaje correspondiente
					Personaje* miPersonaje = malloc (sizeof(Personaje));
					strcpy(miPersonaje->ID, ((Personaje*) miMensaje->data)->ID);
					miPersonaje->FD=miMensaje->from;
					
					//AGREGO EL PERSONAJE A LA LISTA DE PERSONAJES JUGANDO
					if(findPersonaje_byid(personajes_jugando,miPersonaje->ID)== NULL)
						list_add(personajes_jugando, miPersonaje);
					
					//Lo agrego a la lista de personajes en nivel
					list_add(miGestor->personajes_en_nivel, miPersonaje);
					
					//Lo agrego a la cola de listos
					queue_push (miGestor->queue_listos, miPersonaje);
					
					imprimirListos(miGestor->queue_listos, string_from_format("Agrega personaje '%s' a cola de listos.", miPersonaje->ID));
				}
				break;
			case TERMINE_TURNO:
				log_info(Logger, "Recibe mensaje de finalizacion de turno.");
				if(miMensaje->from == miGestor->PersonajeEnMovimiento->FD){
					miGestor->turno_entregado=0;
					miGestor->quantum--;
					if(miGestor->quantum > 0){
						miGestor->turno_entregado=1;
						log_info(Logger, string_from_format("Envia mensaje indicando movimiento permitido (ID PERSONAJE: %s).", miGestor->PersonajeEnMovimiento->ID));
						mandarMensaje(miGestor->PersonajeEnMovimiento->FD, MOVIMIENTO_PERMITIDO, 0, NULL);
					}else{
						miGestor->quantum = quantum_inicial;
						queue_push(miGestor->queue_listos, miGestor->PersonajeEnMovimiento);
						imprimirListos(miGestor->queue_listos, string_from_format("Agrega personaje '%s' a cola de listos.", miGestor->PersonajeEnMovimiento->ID));
						entregarTurno(miGestor);
					}
					
				}

				break;
			case PERSONAJE_BLOQUEADO:
				log_info(Logger, "Recibe mensaje de personaje bloqueado.");
				if(miMensaje->from == miGestor->PersonajeEnMovimiento->FD){
					miGestor->turno_entregado=0;
					Queue_bloqueados* queue_bloq;
					//busco la cola de bloqueados de ese recurso en la lista de colas de bloqueados
					queue_bloq = findBloqQueue_byidRecurso (miGestor->queues_bloq, *((char*)(miMensaje->data)));
					
					if (queue_bloq != NULL){
						queue_push(queue_bloq->queue, miGestor->PersonajeEnMovimiento);
						imprimirBloqueados(miGestor->queues_bloq, string_from_format("Agrega personaje '%s' asignado al recurso '%c' a lista de bloqueados", miGestor->PersonajeEnMovimiento->ID, queue_bloq->idRecurso));
					}else{
						Queue_bloqueados nueva_queue;
						nueva_queue.queue = queue_create();
						nueva_queue.idRecurso = *((char*)(miMensaje->data));
						queue_push(nueva_queue.queue, miGestor->PersonajeEnMovimiento);
						list_add (miGestor->queues_bloq, &nueva_queue);
						imprimirBloqueados(miGestor->queues_bloq, string_from_format("Agrega personaje '%s' asignado al recurso '%c' a lista de bloqueados.", miGestor->PersonajeEnMovimiento->ID, nueva_queue.idRecurso));
					}
					miGestor->quantum=quantum_inicial;
					entregarTurno(miGestor);
				}
				break;
			case TERMINE_NIVEL:
				log_info(Logger,"Personaje termino nivel");
				miGestor->turno_entregado=0;
				removePersonaje_byfd (miGestor->personajes_en_nivel, miMensaje->from);
				break;
			}
			borrarMensaje(miMensaje);//fin de antencion a los mensajes
			
		}else{ //NO HAY MENSAJES, ME FIJO SI DESBLOQUEO ALGUNO
			if(!(miGestor->turno_entregado)){//ESTOY EN STANDBY PORQUE NO HABIA PERSONAJES EN COLA DE LISTOS
				entregarTurno(miGestor);
			}
			
		}//fin de CHEKEO DE MENSAJES
	}//FIN DEL LOOP
	return 0;
}



void* orq (void* a){

	Mensaje* miMensaje;
	t_queue* queue_mensajes = queue_create();
	CCB CCB_Orquestador;
	CCB_Orquestador= initServer(5000);

	while (1){
		//LOOP INFINITO A LA ESPERA DE MENSAJES
		if(mensajes(queue_mensajes,CCB_Orquestador)){
			miMensaje = queue_pop(queue_mensajes);
			switch(miMensaje->type){
			case HANDSHAKE:
				log_info(Logger, "Recibe mensaje de conexion de nivel");
				//CONEXION DE NIVEL
				if (((char*)miMensaje->data)[0]!='P'){
					//CREO LA INSTANCIA DEL THREAD PLANIFICADOR
					pthread_t thr;
					//CREO LA INSTANCIA NIVEL, COPIO LOS PARAMETROS, Y LA AGREGO A LA LISTA
					Nivel* miNivel = malloc(sizeof(Nivel));
					miNivel->FD = miMensaje->from;
					strcpy((miNivel->ID), (((Nivel*) miMensaje->data)->ID));
					strcpy(miNivel->IP, ((Nivel*) miMensaje->data)->IP);
					miNivel->PORT= ((Nivel*) miMensaje->data)->PORT;
					miNivel->FD= miMensaje->from;
					//CREO LA INSTANCIA PLANIFICADOR CORRESPONDIENTE A ESE NIVEL

					log_info(Logger, string_from_format("Crea el Thread con la estructura de nivel (ID: %s - PUERTO NIVEL: %d - IP NIVEL: %s).", miNivel->ID, miNivel->PORT, miNivel->IP));
					//CREO EL THREAD, EL PARAMETRO ES UNA ESTRUCTURA NIVEL
					pthread_create( &thr, NULL, Planif, (void*) miNivel);
				}
				break;

			case REQUEST_DATA_NIVEL:
			{
				log_info(Logger, "Recibe mensaje pidiendo informacion del nivel.");
				GestorNivel* miGestor;
				Data_Nivel miDataNivel;
				miGestor=findGestor_byid(((char*)(miMensaje->data)));
				if(miGestor==NULL){
					log_info(Logger, string_from_format("El nivel %s no esta conectado a plataforma.", (char*)miMensaje->data));
					mandarMensaje(miMensaje->from,NODATANIVEL,0,NULL);
				}else{
					log_info(Logger, string_from_format("Envia mensaje con la informacion del nivel (NIVEL ID: %s - PLANIFICADOR ID: %s).", (miGestor->dataNivel.ID), (miGestor->dataPlanificador.ID)));
					mandarMensaje(miMensaje->from,DATANIVEL,sizeof(Data_Nivel),&(miGestor->dataNivel));
				}
			}
			break;

			case RECURSOS_LIBERADOS:
			{
				log_info(Logger, "Recibe mensaje de recursos liberados.");
				GestorNivel* miGestor;
				Personaje* personajeAux;
				miGestor = findGestor_byfd(miMensaje->from);
				Recursos* miRecurso;
				Recursos recursoAsignado;
				miRecurso = (Recursos*)(miMensaje->data);
				Queue_bloqueados* queue_bloq;
				//busco la cola de bloqueados de ese recurso en la lista de colas de bloqueados
				queue_bloq = findBloqQueue_byidRecurso (miGestor->queues_bloq, miRecurso->idRecurso);
				if (queue_bloq != NULL){
					while(miRecurso->cant){
						//SI LA COLA DE BLOQUEADOS NO ESTA VACIA
						if(!queue_is_empty(queue_bloq->queue)){
							personajeAux = queue_pop(queue_bloq->queue);
							imprimirBloqueados(miGestor->queues_bloq, string_from_format("Quita personaje '%s' asignado al recurso '%c' de la lista de bloqueados.", personajeAux->ID, queue_bloq->idRecurso));
							recursoAsignado = asignarRecurso(miRecurso->idRecurso, personajeAux);
							log_info(Logger, string_from_format("Envia mensaje con la cantidad de recursos asignados (ID PERSONAJE: %s - ID RECURSO: %s - CANT: %d).", (recursoAsignado.idPersonaje), (recursoAsignado.idRecurso), (recursoAsignado.cant)));
							mandarMensaje(miMensaje->from,RECURSOS_REASIGNADOS,sizeof(Recursos), &recursoAsignado);
							queue_push(miGestor->queue_listos, personajeAux);
							imprimirListos(miGestor->queue_listos, string_from_format("Agrega personaje '%s' a cola de listos.", personajeAux->ID));
						}else{
							//LA COLA ESTA VACIAfrom
							log_info(Logger, "Envia mensaje informando reasignacion finalizada debido a que la cola de bloqueados esta vacia para el recurso.");
							mandarMensaje(miMensaje->from, REASIGNACION_FINALIZADA,sizeof(NULL),NULL);
							log_info(Logger, "b;ba");
							miRecurso->cant=0;
						}
					}
					//NO HAY COLA DE BLOQUEADOS ESPERANDO ESE RECURSO
				}else{
					log_info(Logger, "Envia mensaje informando reasignacion finalizada debido a que no hay cola de bloqueados esperando ese recurso.");
					mandarMensaje(miMensaje->from,REASIGNACION_FINALIZADA,0,NULL);
				}
			}
			break;
			case REQUEST_INTERBLOQUEO:
			{
				log_info(Logger, "Recibe mensaje de pedido de interbloqueo.");
				char PersonajesInterbloqueados[10];
				GestorNivel* miGestor;
				Personaje* Victima;
				strcpy(PersonajesInterbloqueados, miMensaje->data);
				miGestor = findGestor_byfd(miMensaje->from);
				Victima = findUltimoEnLlegar (miGestor->personajes_en_nivel, PersonajesInterbloqueados);
				log_info(Logger, "Envia mensaje indicando que murio el personaje");
				mandarMensaje(Victima->FD,MORISTE_PERSONAJE,0,NULL);
				log_info(Logger, string_from_format("Envia mensaje con el nombre de la victima (NOMBRE VICTIMA: %s)", Victima->ID[1]));
				mandarMensaje(miMensaje->from, NOMBRE_VICTIMA ,1,&(Victima->ID[1]));
			}
			break;
			case GANE:
			{
				Personaje* miPersonaje;
				log_info(Logger, string_from_format("Recibe mensaje de que gano, hay %d personajes jugando",list_size(personajes_jugando)) );
				miPersonaje = removePersonaje_byid(personajes_jugando, (Personaje *) miMensaje->data);
				if(miPersonaje!=NULL) log_info(Logger, string_from_format("Removi %s", miPersonaje->ID)); else log_info(Logger,"NADA");
				if(list_is_empty(personajes_jugando)) finalizarProceso();
			}
				break;
			}
			borrarMensaje(miMensaje);
		}

	}
	return 0;
}

void finalizarProceso() {
	log_info(Logger, "Ganaron todos. NIVEL FINAL -- KOOPA");
	char *argumentos[] = {"./koopa", "archivo.lst", NULL};
	log_destroy(Logger);
	execv("./koopa", argumentos);
}

void imprimirListos(t_queue* listos, char* mensaje) {
	if (!string_is_empty(mensaje)) {
		log_info(Logger, mensaje);
	}
	log_info(Logger, "---LISTOS---");
	int index = 0;
	while (index < queue_size(listos)) {
		Personaje* personaje = (Personaje*)list_get(listos->elements, index);
		log_info(Logger, string_from_format("Personaje '%s'", personaje->ID));
		index++;
	}
	log_info(Logger, "-----------");
}

void imprimirBloqueados(t_list* bloqueados, char* mensaje) {
	if (!string_is_empty(mensaje)) {
		log_info(Logger, mensaje);
	}
	log_info(Logger, "---BLOQUEADOS---");
	int index = 0;
	while (index < list_size(bloqueados)) {
		Queue_bloqueados* queueBloqueados = (Queue_bloqueados*)list_get(bloqueados, index);
		log_info(Logger, string_from_format("Recurso: '%c'", queueBloqueados->idRecurso));
		int queueIndex = 0;
		while (queueIndex < queue_size(queueBloqueados->queue)) {
			Personaje* personaje = (Personaje*)list_get(queueBloqueados->queue->elements, queueIndex);
			log_info(Logger, string_from_format("-> Personaje '%s'", personaje->ID));
			queueIndex++;
		}
		index++;
	}
	log_info(Logger, "----------------");
}

void entregarTurno (GestorNivel* miGestor){
	usleep(200000);
	miGestor->PersonajeEnMovimiento = (Personaje*) (queue_pop (miGestor->queue_listos));
	if(miGestor->PersonajeEnMovimiento != NULL){ //Si hay personaje en cola de listos, entrega turno
		miGestor->turno_entregado=1;// SINO, SIGO.
		imprimirListos(miGestor->queue_listos, string_from_format("Quita personaje '%s' de cola de listos.", miGestor->PersonajeEnMovimiento->ID));
		log_info(Logger, string_from_format("Envia mensaje indicando movimiento permitido (ID PERSONAJE: %s).", miGestor->PersonajeEnMovimiento->ID));
		mandarMensaje(miGestor->PersonajeEnMovimiento->FD, MOVIMIENTO_PERMITIDO, 0, NULL);
	}
}

//CHEKEADA
GestorNivel* findGestor_byid (char* nivel){
	bool _eselGestor (GestorNivel* comparador){
		return(string_equals_ignore_case(comparador->ID, nivel));
	}
	return (list_find(Gestores,(void*)_eselGestor));
}

Personaje* removePersonaje_byid (t_list* personajes_en_nivel, Personaje * miPersonaje){
	log_info(Logger, string_from_format ("recibi fd %d", fd));
	bool _eselPersonaje (Personaje* comparador){
		log_info(Logger, string_from_format ("recibi fd %d", id));
		return(string_equals_ignore_case(comparador->ID, miPersonaje->ID));
	}
	return (list_remove_by_condition(personajes_en_nivel,(void*)_eselPersonaje));
}

Personaje* findPersonaje_byid (t_list* personajes_en_nivel, char* id){
	bool _eselPersonaje (Personaje* comparador){
		return(string_equals_ignore_case(comparador->ID, id));
	}
	return (list_find(personajes_en_nivel,(void*)_eselPersonaje));
}


Queue_bloqueados* findBloqQueue_byidRecurso (t_list* lista, char idRecurso){
	log_info(Logger, string_from_format("Busco lista de bloqueados de recurs: %c",idRecurso));
	bool _eselGestor (Queue_bloqueados* comparador){
		log_info(Logger, string_from_format("LLamada a la funcion. %c",comparador->idRecurso));
		return(comparador->idRecurso==idRecurso);
	}
	return (list_find(lista,(void*)_eselGestor));
}

GestorNivel* findGestor_byfd (int fd){
	bool _eselGestor (GestorNivel* comparador){
		return(comparador->dataNivel.FD==fd);
	}
	return (list_find(Gestores,(void*)_eselGestor));
}

//CHEKEADA
Personaje* findUltimoEnLlegar (t_list* ListaDePersonajes, char PersonajesInterbloqueados[10]){
	char PersonajesABuscar [10];
	strcpy(PersonajesABuscar,PersonajesInterbloqueados);
	bool _esElUltimoEnEncontrarse (Personaje* comparador){
		int i;
		for (i=0; i<(strlen(PersonajesABuscar)); i++){
			if(comparador->ID[1]==PersonajesABuscar[i]) {
				memmove((&PersonajesABuscar[i]), (&PersonajesABuscar[i+1]), strlen(PersonajesABuscar)-i);
				if (!strcmp(PersonajesABuscar,"")){
					return true;
				}
			}
		}
		return false;
	}
	return (list_find(ListaDePersonajes,(void*)_esElUltimoEnEncontrarse));

}

Recursos asignarRecurso (char recurso, Personaje* miPersonaje){
	Recursos Aux;
	Aux.idRecurso = recurso;
	Aux.idPersonaje = miPersonaje->ID[1];
	return Aux;
}
