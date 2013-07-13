#include "plataforma.h"
#include "quantum_inotify.h"
#include <string.h>

int main (int argc, char *argv[]){
	
	if (argc < 2) {
		fprintf(stderr, "Falta el archivo de configuracion por parametro!\n");
		return EXIT_FAILURE;
	}
	
	path_archivo_config = argv[1];
	leerArchivoConfig();
	
	
	pthread_mutex_lock(&mutex_plataforma);
	Gestores = list_create();
	personajes_jugando = list_create();
	Logger = log_create("ProcesoPlataforma.log", "ProcesoPlataforma", true, LOG_LEVEL_INFO);
	log_info(Logger, "************************************************************************");
	log_info(Logger, "Se inicia Proceso Plataforma.");
	
	
	quantum_inicial=3;
	pthread_create(&orquestador, NULL, orq, NULL);
	log_info(Logger, "Crea Thread Orquestador.");
	pthread_create(&quantum_monitor, NULL, monitorear_quantum, path_archivo_config); // TODO: Pasar por parametro

	pthread_join(orquestador, NULL);

	pthread_mutex_unlock(&mutex_plataforma);
	pthread_join(quantum_monitor, NULL);

	return 0;
}



void* Planif(void* nivel){

	log_info(Logger, "Inicia Planificador.");

	GestorNivel* miGestor;
	Nivel* miNivel = (Nivel*)nivel;
	t_queue* misMensajes;
	misMensajes = queue_create();
	Mensaje* miMensaje;
	int desconexion_autorizada = 0;
	
	
	
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
	strcpy(miGestor->dataPlanificador.IP, mi_ip);

	//Copio la instancia de dataNivel
	memcpy(&(miGestor->dataNivel),miNivel, sizeof(Nivel));
	free(miNivel);
	
	//Creo la instancia del mutex del gestor
	miGestor->miMutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(miGestor->miMutex, NULL );
	
	//Agrego el gestor a la lista de gestores.
	list_add (Gestores, miGestor);
	

	miGestor->miCON = initServer(miGestor->dataPlanificador.PORT);
	miGestor->miCON.flag_desconexiones = 1;
	miGestor->PersonajeEnMovimiento = NULL;

	log_info(Logger, "Crea e inicializa las variables y colecciones necesarias del Planificador.");
	
	
	while(1){
		
		if(mensajes(misMensajes , miGestor->miCON)){
			miMensaje = queue_pop(misMensajes);
			switch(miMensaje->type){
			case HANDSHAKE:
				//CONEXION DE PERSONAJE
				log_info(Logger, "Recibe mensaje de conexion de Proceso Personaje.");
				if(((char*)miMensaje->data)[0]=='P'){
					//Inicializo la instancia de personaje correspondiente
					Personaje* auxPersonaje = malloc (sizeof(Personaje));
					Personaje* miPersonaje;
					strcpy(auxPersonaje->ID, ((Personaje*) miMensaje->data)->ID);
					
					
					if((miPersonaje = findPersonaje_byid(personajes_jugando,auxPersonaje->ID))== NULL){

						//USO LA MEMORIA AUXILIAR Y SE LA ASIGNO AL PERSONAJE
						miPersonaje = auxPersonaje;

						//AGREGO EL PERSONAJE A LA LISTA DE PERSONAJES JUGANDO
						list_add(personajes_jugando, miPersonaje);

					}else{

						//SI YA EXISTIA SIGO TRABAJANDO CON LA MISMA MEMORIA, Y LIBERO LA AUXILIAR
						free(auxPersonaje);
					}
					
					miPersonaje->FD=miMensaje->from;

					//ENTRO EN ZONA CRITICA
					pthread_mutex_lock(miGestor->miMutex);

					//Lo agrego a la lista de personajes en nivel
					list_add(miGestor->personajes_en_nivel, miPersonaje);
					
					//Lo agrego a la cola de listos
					queue_push (miGestor->queue_listos, miPersonaje);
					
					imprimirListos(miGestor->queue_listos, string_from_format("Agrega personaje '%s' a cola de listos.", miPersonaje->ID));
					
					//SALGO DE ZONA CRITICA
					pthread_mutex_unlock(miGestor->miMutex);

				}
				break;
			case TERMINE_TURNO:
				log_info(Logger, "Recibe mensaje de finalizacion de turno.");
				
				//ENTRO EN ZONA CRITICA
				pthread_mutex_lock(miGestor->miMutex);

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
				//SALGO DE ZONA CRITICA
				pthread_mutex_unlock(miGestor->miMutex);

				break;

			case PERSONAJE_BLOQUEADO:
				log_info(Logger, "Recibe mensaje de personaje bloqueado.");

				//ENTRO EN ZONA CRITICA
					pthread_mutex_lock(miGestor->miMutex);

				if(miMensaje->from == miGestor->PersonajeEnMovimiento->FD){
					miGestor->turno_entregado=0;
					Queue_bloqueados* queue_bloq;
					//busco la cola de bloqueados de ese recurso en la lista de colas de bloqueados
					queue_bloq = findBloqQueue_byidRecurso (miGestor->queues_bloq, *((char*)(miMensaje->data)));
					
					if (queue_bloq != NULL){
						queue_push(queue_bloq->queue, miGestor->PersonajeEnMovimiento);
						imprimirBloqueados(miGestor->queues_bloq, string_from_format("Agrega personaje '%s' asignado al recurso '%c' a lista de bloqueados", miGestor->PersonajeEnMovimiento->ID, queue_bloq->idRecurso));
					}else{
						Queue_bloqueados* nueva_queue= malloc(sizeof(Queue_bloqueados));
						nueva_queue->queue = queue_create();
						nueva_queue->idRecurso = *((char*)(miMensaje->data));
						queue_push(nueva_queue->queue, miGestor->PersonajeEnMovimiento);
						list_add (miGestor->queues_bloq, nueva_queue);
						imprimirBloqueados(miGestor->queues_bloq, string_from_format("Crea lista de bloqueados y agrega personaje '%s' asignado al recurso '%c' a lista de bloqueados.", miGestor->PersonajeEnMovimiento->ID, nueva_queue->idRecurso));
					}
					miGestor->quantum=quantum_inicial;
					entregarTurno(miGestor);
				}
				//SALGO DE ZONA CRITICA
					pthread_mutex_unlock(miGestor->miMutex);

				break;
			case TERMINE_NIVEL:
			{
				Personaje* Person;
				log_info(Logger,"Personaje termino nivel");

				//ENTRO EN ZONA CRITICA
					pthread_mutex_lock(miGestor->miMutex);

				miGestor->turno_entregado=0;
				Person = removePersonaje_byfd (miGestor->personajes_en_nivel, miMensaje->from);
				log_info(Logger, string_from_format("termino nivel%x",Person));
				removePersonaje_byfd (miGestor->queue_listos->elements, miMensaje->from);
				removePersonaje_fromBloq(miGestor->queues_bloq, Person);
				desconexion_autorizada=1;

				//SALGO DE ZONA CRITICA
					pthread_mutex_unlock(miGestor->miMutex);
			}
				break;
			case DESCONEXION:
			{
				if(desconexion_autorizada == 0){
							Personaje* Person;
							log_info(Logger,"Se desconecto personaje");

							//ENTRO EN ZONA CRITICA
								pthread_mutex_lock(miGestor->miMutex);

							
							Person = removePersonaje_byfd (miGestor->personajes_en_nivel, *((int*)miMensaje->data));
							//log_info(Logger, string_from_format("termino nivel%x",Person));
							if(miGestor->PersonajeEnMovimiento){
								miGestor->turno_entregado=0;
							}
							removePersonaje_byfd (miGestor->queue_listos->elements, *((int*)miMensaje->data));
							if (Person != NULL) removePersonaje_fromBloq(miGestor->queues_bloq, Person);
							removePersonaje_byfd(personajes_jugando, *((int*)miMensaje->data));
							//SALGO DE ZONA CRITICA
								pthread_mutex_unlock(miGestor->miMutex);
				}else{
					desconexion_autorizada=0;
				}
			}
			break;
			}
			borrarMensaje(miMensaje);//fin de antencion a los mensajes
			
		}else{ //NO HAY MENSAJES, ME FIJO SI DESBLOQUEO ALGUNO
			//ENTRO EN ZONA CRITICA
					pthread_mutex_lock(miGestor->miMutex);
			if(!(miGestor->turno_entregado)){//ESTOY EN STANDBY PORQUE NO HABIA PERSONAJES EN COLA DE LISTOS
				entregarTurno(miGestor);
			}
			//SALGO DE ZONA CRITICA
					pthread_mutex_unlock(miGestor->miMutex);
			
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
							//ENTRO EN ZONA CRITICA
							pthread_mutex_lock(miGestor->miMutex);	
						
							personajeAux = queue_pop(queue_bloq->queue);
							imprimirBloqueados(miGestor->queues_bloq, string_from_format("Quita personaje '%s' asignado al recurso '%c' de la lista de bloqueados.", personajeAux->ID, queue_bloq->idRecurso));
							asignarDatos(&recursoAsignado, miRecurso->idRecurso, personajeAux);
							log_info(Logger, string_from_format("Envia mensaje de recursos asignado (ID PERSONAJE: %c - ID RECURSO: %c).", (recursoAsignado.idPersonaje), (recursoAsignado.idRecurso)));
							mandarMensaje(miMensaje->from,RECURSOS_REASIGNADOS,sizeof(Recursos), &recursoAsignado);
							queue_push(miGestor->queue_listos, personajeAux);
							imprimirListos(miGestor->queue_listos, string_from_format("Agrega personaje '%s' a cola de listos.", personajeAux->ID));
							(miRecurso->cant)--;
							
							//SALGO DE ZONA CRITICA
							pthread_mutex_unlock(miGestor->miMutex);									
						}else{
							//LA COLA ESTA VACIA
							miRecurso->cant=0;
						}
					}

					//EXISTIA COLA, ESTABA VACIA, O SE ACABARON LOS RECURSOS DISPONIBLES
					log_info(Logger, "Envia mensaje informando reasignacion finalizada.");
					mandarMensaje(miMensaje->from, REASIGNACION_FINALIZADA,0,NULL);
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
				mandarMensaje(miMensaje->from, NOMBRE_VICTIMA ,1,&(Victima->ID[1]));
				log_info(Logger, "Envia mensaje indicando que murio el personaje");
				mandarMensaje(Victima->FD,MORISTE_PERSONAJE,0,NULL);
				log_info(Logger, string_from_format("Mata al personaje (NOMBRE VICTIMA: %s)", Victima->ID));
				

				//ENTRO EN ZONA CRITICA
				//	pthread_mutex_lock(miGestor->miMutex);

				//removePersonaje_fromBloq(miGestor->queues_bloq, Victima);
				//imprimirBloqueados(miGestor->queues_bloq,"Borro victima de lista de Bloqueados");
				
				//ENTRO EN ZONA CRITICA
				//	pthread_mutex_unlock(miGestor->miMutex);
				
			}
			break;
			case GANE:
			{
				Personaje* miPersonaje;
				log_info(Logger, string_from_format("Recibe mensaje de que gano, hay %d personajes jugando",list_size(personajes_jugando)) );
				miPersonaje = removePersonaje_byid(personajes_jugando, (Personaje *) miMensaje->data);
				if(miPersonaje!=NULL) log_info(Logger, string_from_format("Removi %s de los personajes jugando", miPersonaje->ID)); else log_info(Logger,"NADA");
				free(miPersonaje);
				if(list_is_empty(personajes_jugando)) finalizarProceso();
				break;
			}
			case CERRANDO_NIVEL:
				{
					
					GestorNivel* miGestor;
					Personaje* AUX;
					int index = 0;			
					//log_info(Logger, "0");
					miGestor=findGestor_byfd(miMensaje->from);
					log_info(Logger, string_from_format("Recibe mensaje avisando desconexion de nivel: %s.",miGestor->ID));
					pthread_mutex_lock(miGestor->miMutex);
					if(miGestor->PersonajeEnMovimiento) mandarMensaje(miGestor->PersonajeEnMovimiento->FD,REINICIAR_NIVEL,0,NULL);
					//log_info(Logger, "1");
					while(queue_size(miGestor->queue_listos)){
						
						AUX = queue_pop(miGestor->queue_listos);
						mandarMensaje(AUX->FD,REINICIAR_NIVEL,0,NULL);
						//log_info(Logger, "2");
					}
					
					while (index < list_size(miGestor->queues_bloq)) {
							Queue_bloqueados* queueBloqueados = (Queue_bloqueados*)list_get(miGestor->queues_bloq, index);
							while(queue_size(queueBloqueados->queue)){
								
								AUX = queue_pop(queueBloqueados->queue);
								mandarMensaje(AUX->FD,REINICIAR_NIVEL,0,NULL);
							}
							index++;
					}
					close(miGestor->miCON.masterfd);
					removeGestor_byfd(miMensaje->from);
					pthread_mutex_unlock(miGestor->miMutex);
					
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
	char *argumentos[] = {"./koopa", path_koopa_lst, NULL};
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
	
	usleep(quantum_delay);
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
GestorNivel* removeGestor_byfd (int fd){
	bool _eselGestor (GestorNivel* comparador){
		return(comparador->dataNivel.FD==fd);
	}
	return (list_remove_by_condition(Gestores,(void*)_eselGestor));
	
}


Personaje* removePersonaje_byfd (t_list* personajes_en_nivel, int fd){
	
	bool _eselPersonaje (Personaje* comparador){
			return((comparador->FD)==fd);
		}
		return (list_remove_by_condition(personajes_en_nivel,(void*)_eselPersonaje));
}


Personaje* removePersonaje_byid (t_list* personajes_en_nivel, Personaje * miPersonaje){
	bool _eselPersonaje (Personaje* comparador){
		return(string_equals_ignore_case(comparador->ID, miPersonaje->ID));
	}
	return (list_remove_by_condition(personajes_en_nivel,(void*)_eselPersonaje));
}


void removePersonaje_fromBloq (t_list* bloqueados, Personaje* miPersonaje){
	int index = 0;
	while (index < list_size(bloqueados)) {
		Queue_bloqueados* queueBloqueados = (Queue_bloqueados*)list_get(bloqueados, index);
		removePersonaje_byid(queueBloqueados->queue->elements, miPersonaje);
		index++;
	}

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
				return true;
			}
		}
		return false;
	}
	return (list_find(ListaDePersonajes,(void*)_esElUltimoEnEncontrarse));

}

void asignarDatos (Recursos* Aux, char recurso, Personaje* miPersonaje){
	Aux->idRecurso = recurso;
	Aux->idPersonaje = miPersonaje->ID[1];
}

void leerArchivoConfig(void){
	plataforma_config = config_create(path_archivo_config);
		if (plataforma_config == NULL) {
			fprintf(stderr, "Archivo de configuracion invalido!\n");
			exit(EXIT_FAILURE);
		}
			
		mi_ip = config_get_string_value(plataforma_config, "IPlocal");
		quantum_delay = config_get_long_value(plataforma_config, "quantum_delay");
		path_koopa_lst = config_get_string_value(plataforma_config, "KoopaLst");
}
