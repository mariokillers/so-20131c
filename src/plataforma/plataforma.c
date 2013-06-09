
#include "plataforma.h"
#include <string.h>




t_list* Gestores;
int main (){

	Gestores = list_create();

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
	pthread_create( &orquestador, NULL, orq, NULL );

	pthread_join(orquestador,NULL);

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
	//int Quantum = 500;
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

	//Creo las colas del nivel
	miGestor->queue_listos = queue_create();
	miGestor->queues_bloq = list_create();


	//Asigno ID del gestor ="GX"
	strcpy(miGestor->ID, miNivel->ID);
	miGestor->ID[0]='G';

	//Asigno ID="PXXXX"
	strcpy(miGestor->dataPlanificador.ID, miNivel->ID);
	miGestor->dataPlanificador.ID[0]='P';
	//Los puertos de los planificadores son 5501 5502 5503...
	miGestor->dataPlanificador.PORT= (7777/*miNivel->PORT*/);
	strcpy(miGestor->dataPlanificador.IP, "localhost");

	//Copio la instancia de dataNivel
	memcpy(&(miGestor->dataNivel),miNivel, sizeof(Nivel));
	free(miNivel);

	//Agrego el gestor a la lista de gestores.
	list_add (Gestores, miGestor);

	miCON = initServer(miGestor->dataPlanificador.PORT);


	while(1){


			if(mensajes(misMensajes ,miCON)){
				miMensaje = queue_pop(misMensajes);
				switch(miMensaje->type){
					case HANDSHAKE:
						//CONEXION DE P	ERSONAJE
						if(((char*)miMensaje->data)[0]=='P'){
							//Inicializo la instancia de personaje correspondiente
							Personaje* miPersonaje = malloc (sizeof(Personaje));
							strcpy(miPersonaje->ID, ((Personaje*) miMensaje->data)->ID);
							miPersonaje->FD=miMensaje->from;
							//Lo agrego a la cola de listos o le permito mover si no hay nadie en la cola
							if(queue_is_empty(miGestor->queue_listos)){
								miGestor->PersonajeEnMovimiento = miPersonaje;
								mandarMensaje(miPersonaje->FD, MOVIMIENTO_PERMITIDO, 0, NULL);
							}else{
								queue_push (miGestor->queue_listos, miPersonaje);
							}
						}
					break;
					case TERMINE_TURNO:
						if(miMensaje->from == miGestor->PersonajeEnMovimiento->FD){
							queue_push(miGestor->queue_listos, miGestor->PersonajeEnMovimiento);
							usleep(2000000);
							miGestor->PersonajeEnMovimiento = (Personaje*) (queue_pop (miGestor->queue_listos));
							mandarMensaje(miGestor->PersonajeEnMovimiento->FD, MOVIMIENTO_PERMITIDO, 0, NULL);
						}

					break;
					case PERSONAJE_BLOQUEADO:
						if(miMensaje->from == miGestor->PersonajeEnMovimiento->FD){
							Queue_bloqueados* queue_bloq;
							//busco la cola de bloqueados de ese recurso en la lista de colas de bloqueados
							queue_bloq = findBloqQueue_byidRecurso (miGestor->queues_bloq, *((char*)(miMensaje->data)));
							if (queue_bloq != NULL){
								queue_push(queue_bloq->queue, miGestor->PersonajeEnMovimiento);
							}else{
								Queue_bloqueados nueva_queue;
								nueva_queue.queue = queue_create();
								nueva_queue.idRecurso = *((char*)(miMensaje->data));
								queue_push(nueva_queue.queue, miGestor->PersonajeEnMovimiento);
								list_add (miGestor->queues_bloq, &nueva_queue);
							}
							usleep(2000000);
							miGestor->PersonajeEnMovimiento = (Personaje*) (queue_pop (miGestor->queue_listos));
							mandarMensaje(miGestor->PersonajeEnMovimiento->FD, MOVIMIENTO_PERMITIDO, 0, NULL);
						}

				}
			}

	}

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

								//CREO EL THREAD, EL PARAMETRO ES UNA ESTRUCTURA NIVEL
								pthread_create( &thr, NULL, Planif, (void*) miNivel);

							}
						break;

						case REQUEST_DATA_NIVEL:
						{
							GestorNivel* miGestor;
							Data_Nivel miDataNivel;
							miGestor=findGestor_byid(((char*)(miMensaje->data)));
							memcpy(&(miDataNivel.miNivel),&(miGestor->dataNivel),sizeof(Nivel));
							memcpy(&(miDataNivel.miPlanificador),&(miGestor->dataPlanificador),sizeof(Planificador));
							mandarMensaje(miMensaje->from,DATANIVEL,sizeof(Data_Nivel),&miDataNivel);
						}
						break;

						case RECURSOS_LIBERADOS:
						{
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
										recursoAsignado = asignarRecurso(miRecurso->idRecurso, personajeAux);
										mandarMensaje(miMensaje->from,RECURSOS_REASIGNADOS,sizeof(Recursos), &recursoAsignado);
										queue_push(miGestor->queue_listos, personajeAux);
									}else{
										//LA COLA ESTA VACIA
										mandarMensaje(miMensaje->from,REASIGNACION_FINALIZADA,0,NULL);
										miRecurso->cant=0;
									}
								}
							//NO HAY COLA DE BLOQUEADOS ESPERANDO ESE RECURSO
							}else{
								mandarMensaje(miMensaje->from,REASIGNACION_FINALIZADA,0,NULL);
							}
						}
						break;
						case REQUEST_INTERBLOQUEO:
						{
							char PersonajesInterbloqueados[10];
							GestorNivel* miGestor;
							Personaje* Victima;
							strcpy(PersonajesInterbloqueados, miMensaje->data);
							miGestor = findGestor_byfd(miMensaje->from);
							Victima = findUltimoEnLlegar (miGestor->personajes_en_nivel, PersonajesInterbloqueados);
							mandarMensaje(Victima->FD,MORISTE_PERSONAJE,0,NULL);
							mandarMensaje(miMensaje->from, NOMBRE_VICTIMA ,1,&(Victima->ID[1]));
						}
						break;



					}
				}

	}
	return 0;
}

//CHEKEADA
GestorNivel* findGestor_byid (char* nivel){
	bool _eselGestor (GestorNivel* comparador){
		return(strcmp(comparador->ID, nivel)==0);
	}
	return (list_find(Gestores,(void*)_eselGestor));

}

Queue_bloqueados* findBloqQueue_byidRecurso (t_list* lista, char idRecurso){
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
