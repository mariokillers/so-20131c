/*
 * ProcesoPersonaje.c
 *
 *  Created on: 20/05/2013
 *      Author: utnso
 */



#include "ProcesoPersonaje.h"


t_personaje *personaje;
t_personaje *personaje_init;

//variables locales //VER
char state = NUEVO_NIVEL;
Posicion *posProxRec;
char proxRec;
char* proxNiv;
char *nivActual;
Posicion *nuevaPos;
Posicion *posActual;

CCB clientCCB_orq;
CCB clientCCB_niv;
CCB clientCCB_pln;


int main(void){

	//inicializo el personaje desde el archivo config

	personaje= read_personaje_archivo_configuracion("PATH");              //VER DE DONDE SALE EL PATH

	personaje_init = personaje;
	strcpy(nivActual,"");
	posActual = (Posicion*)personaje->personaje_posicion_actual;


	//declaro las señales

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
					clientCCB_orq = connectServer(((char*)((Direccion*)(personaje->personaje_orquestador))->IP), ((int)((Direccion*)(personaje->personaje_orquestador))->PORT));

					//verifico si gane
					if(ganado(personaje->personaje_niveles)){
						mandarMensaje(clientCCB_orq.sockfd,GANE,sizeof(personaje->personaje_simbolo),(char *)&personaje->personaje_simbolo);

						state = WIN;
					} else{

					//mando el primer mensaje al orquestador solicitando informacion del nivel y su planificador

					strcpy(proxNiv, proximoNivel(personaje->personaje_niveles));
					char miNiv[3];
					char *miNivAux;
					transformNivel_to_send(proxNiv, &miNivAux);
					strcpy(miNiv, miNivAux);

					mandarMensaje(clientCCB_orq.sockfd,REQUEST_DATA_NIVEL,sizeof(miNiv),miNiv);

					state = WAIT_DATA_LEVEL;
					}

					break;

				case WAIT_DATA_LEVEL:
					if (mensajes(colaDeMensajes, clientCCB_orq)){
						//agarra de la cola de mensajes un mensaje
						mensaje = queue_pop(colaDeMensajes);

						//analiza los mensajes recibidos y en base a eso, actua
						switch(mensaje->type){
							case DATANIVEL:
								strcpy(nivActual, proxNiv);
								Data_Nivel *data_new = (Data_Nivel*) mensaje->data;
								Nivel nivel_new = data_new->miNivel;
								Planificador planificador_new = data_new->miPlanificador;

								//desconecta del orquestador y conecta a nivel y planificador
								close(clientCCB_orq.sockfd);

								clientCCB_niv = connectServer((char*)nivel_new.IP, (int)nivel_new.PORT);
								clientCCB_pln = connectServer((char*)planificador_new.IP, (int)planificador_new.PORT);

								state = STANDBY;

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
								if(posProxRec != NULL){
									nuevaPos = realizarMovimiento(posActual, posProxRec, clientCCB_niv);
									posActual = nuevaPos;
									((Posicion*)(personaje->personaje_posicion_actual))->POS_X = posActual->POS_X;
									((Posicion*)(personaje->personaje_posicion_actual))->POS_Y = posActual->POS_Y;
									analizarRecurso(posActual, posProxRec, clientCCB_niv, clientCCB_pln, &state, (char) proxRec);
								} else{
									//solicitar posicion proximo recurso
									proxRec = proximoRecurso(personaje->personaje_niveles, nivActual);

									mandarMensaje(clientCCB_niv.sockfd,REQUEST_POS_RECURSO, sizeof(proxRec),(char *)&proxRec);

									state = WAIT_POS_REC;
								}
								break;

							case MORISTE_PERSONAJE:

								morir(personaje, personaje_init, clientCCB_niv, posProxRec, state, nivActual);


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
								posProxRec = (Posicion*) mensaje->data;
								nuevaPos = realizarMovimiento(posActual, posProxRec, clientCCB_niv);
								posActual = nuevaPos;
								((Posicion*)(personaje->personaje_posicion_actual))->POS_X = posActual->POS_X;
								((Posicion*)(personaje->personaje_posicion_actual))->POS_Y = posActual->POS_Y;
								analizarRecurso(posActual, posProxRec, clientCCB_niv, clientCCB_pln, &state, (char) proxRec);
						}
						break;
						borrarMensaje(mensaje);
					}
					break;
				case WAIT_REC:
					if (mensajes(colaDeMensajes, clientCCB_niv)){
						mensaje = queue_pop(colaDeMensajes);

						switch(mensaje->type){
						char *respuesta = malloc((int)mensaje->lenght);
						respuesta = ((char*) mensaje->data);
							case CONFIRMAR_RECURSO:

								//si le otorgaron el recurso
								if(*respuesta){
									mandarMensaje(clientCCB_pln.sockfd,TERMINE_TURNO,sizeof(NULL),NULL);
									agregarRecurso(personaje->personaje_niveles, nivActual, proxRec);
									posProxRec = NULL;

									if(nivelTerminado(personaje->personaje_niveles, nivActual)){
										mandarMensaje(clientCCB_niv.sockfd,TERMINE_NIVEL,sizeof(NULL),NULL);

										((Posicion*)(personaje->personaje_posicion_actual))->POS_X = 0;
										((Posicion*)(personaje->personaje_posicion_actual))->POS_Y = 0;

										close(clientCCB_niv.sockfd);
										close(clientCCB_pln.sockfd);

										state = NUEVO_NIVEL;
									}else{
										state = STANDBY;
									}

								} else{
									mandarMensaje(clientCCB_pln.sockfd,PERSONAJE_BLOQUEADO,sizeof(proxRec), (char *)&proxRec);

									state = STANDBY;
								}
							}
							borrarMensaje(mensaje);
						}
						break;

					case WIN:
						return 1;
						break;
					}
			}

	return 1;
}


int _is_next_level(t_personaje_nivel *p){
	return !(p->termino_nivel);
}

char *proximoNivel(t_list *niveles){
	t_personaje_nivel *auxNiv = list_find(niveles, (void*) _is_next_level);

	return auxNiv->personaje_nivel;
}

char* transformNivel_to_send(char *nivel, char **miNivAux){
	strcpy(*miNivAux, string_from_format("N%c", nivel[(strlen(nivel)-1)]));
	return *miNivAux;
}

int _is_next_obj(t_personaje_objetivo *o){
	return !(o->tiene_objetivo);
}

char proximoRecurso(t_list *niveles, char *nivActual){
	t_personaje_nivel *auxNiv = list_find(niveles, (void*) _is_next_level);
	t_personaje_objetivo *auxObj = list_find(auxNiv->personaje_objetivos, (void*) _is_next_obj);

	return auxObj->objetivo;
}

Posicion *proximaPosicion(Posicion *posActual, Posicion *posProxRec){
	Posicion *pos = posActual;
	if((pos->POS_X < posProxRec->POS_X) && (pos->POS_X != posProxRec->POS_X)){
		pos->POS_X ++;
		return pos;
	} else if((pos->POS_X > posProxRec->POS_X) && (pos->POS_X != posProxRec->POS_X)){
		pos->POS_X --;
		return pos;
	} else if((pos->POS_Y > posProxRec->POS_Y) && (pos->POS_Y != posProxRec->POS_Y)){
		pos->POS_Y --;
		return pos;
	} else if((pos->POS_Y < posProxRec->POS_Y) && (pos->POS_Y != posProxRec->POS_Y)){
		posActual->POS_Y ++;
		return posActual;
	}
	return posActual;
}

void reiniciarNivel(t_list *niveles, char *nivActual){
	t_personaje_nivel *nn = malloc(sizeof(t_personaje_nivel));
	int i = 0;
	int j = 0;
	int lenNiv = list_size(niveles);
	t_personaje_objetivo *auxObj;
	t_personaje_nivel *auxNiv;
	while((j<lenNiv) && (!(string_equals_ignore_case(nivActual, auxNiv->personaje_nivel)))){
		auxNiv = list_get(niveles,j);
		j++;
	}
	int lenObj = list_size(auxNiv->personaje_objetivos);
	while(i<lenObj){
		auxObj = list_get(auxNiv->personaje_objetivos,i);
		auxObj->tiene_objetivo = false;
		i++;
	}
	auxNiv->termino_nivel = true;
	nn = list_replace(niveles, j, auxNiv);
	free(nn);
}

bool recursoAlcanzado(Posicion *pos1, Posicion *pos2){
	return ((pos1->POS_X==pos2->POS_X)&&(pos1->POS_Y==pos2->POS_Y));
}


Posicion *realizarMovimiento(Posicion *posActual, Posicion *posProxRec, CCB clientCCB_niv){
	Posicion *nuevaPos = proximaPosicion(posActual, posProxRec);

	mandarMensaje(clientCCB_niv.sockfd,REQUEST_MOVIMIENTO,sizeof(&nuevaPos),(char*)&nuevaPos);

	return nuevaPos;
}

void analizarRecurso(Posicion *posActual, Posicion *posProxRec, CCB clientCCB_niv, CCB clientCBB_pln, char *state, char proxRec){
	if(recursoAlcanzado(posActual, posProxRec)){
		mandarMensaje(clientCCB_niv.sockfd,REQUEST_RECURSO,sizeof(proxRec), (char *)&proxRec);

		*state = WAIT_REC;
	} else{
		mandarMensaje(clientCBB_pln.sockfd,TERMINE_TURNO,sizeof(NULL),NULL);

		*state = STANDBY;
	}
}

void agregarRecurso(t_list *niveles, char *nivActual, char proxRec){
	t_personaje_nivel *auxNiv = list_find(niveles, (void*) _is_next_level);
	int i = 0;
	t_list *auxListObj = auxNiv->personaje_objetivos;
	int len = list_size(auxListObj);
	t_personaje_objetivo *auxObj;
	t_personaje_nivel *nn;
	while(i<len){
		auxObj = list_get(auxListObj, i);
		if(auxObj->objetivo == proxRec){
			auxObj->tiene_objetivo = true;
			nn = list_replace(auxListObj, i, auxObj);
			break;
		} else{
			i++;
		}
	}
}

bool nivelTerminado(t_list *niveles, char *nivActual){
	t_personaje_nivel *nn = malloc(sizeof(t_personaje_nivel));
	int i = 0;
	int j = 0;
	int lenNiv = list_size(niveles);
	t_personaje_objetivo *auxObj;
	t_personaje_nivel *auxNiv = list_get(niveles,0);
	while((j<lenNiv) && (!(string_equals_ignore_case(nivActual, auxNiv->personaje_nivel)))){
		auxNiv = list_get(niveles,j);
	}
	t_list *auxListObj = auxNiv->personaje_objetivos;
	int lenObj = list_size(auxListObj);
	while(i<lenObj){
		auxObj = list_get(auxListObj, i);
		if(auxObj->tiene_objetivo){
			i++;
		} else{
			free(nn);
			return false;
		}
	}
	auxNiv->termino_nivel = true;
	nn = list_replace(niveles, i, auxNiv);
	free(nn);
	return true;
}

bool ganado(t_list *niveles){
	int i = 0;
	t_personaje_nivel *auxNiv;
	int len = list_size(niveles);
	while(i<len){
		auxNiv = list_get(niveles, i);
		if(nivelTerminado(niveles, auxNiv->personaje_nivel)){
			i++;
		} else{
			return false;
		}
	}
	return true;
}

void morir(t_personaje *personaje, t_personaje *personaje_init, CCB clientCCB_niv, Posicion *posProxRec, char state, char *nivActual){

	mandarMensaje(clientCCB_niv.sockfd,TERMINE_NIVEL,sizeof(NULL),NULL);
	personaje->personaje_vidas_restantes --;

	//si no le quedan vidas, reinicia todo de nuevo -> personaje apunta al personaje iniciar
	if(personaje->personaje_vidas_restantes == 0){
		printf("muerte del personaje %c por interbloqueo", personaje->personaje_simbolo);
		personaje = personaje_init;
	} else{
		reiniciarNivel(personaje->personaje_niveles, nivActual);
		posProxRec = NULL;

		state = STANDBY;
	}
}

void rutinaSignal(int n){
	switch (n) {
		case SIGTERM:
			morir(personaje, personaje_init, clientCCB_niv, posProxRec, state, nivActual);

		break;
		case SIGUSR1:
			personaje->personaje_vidas_restantes ++;
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

	personaje = create_personaje(p);

	return personaje;
}

void imprimir_personaje(t_personaje *personaje){
	printf("%s\n%c\n", personaje->personaje_nombre, personaje->personaje_simbolo);

	imprimir_lista_niveles(personaje->personaje_niveles);

	printf("%d\n", personaje->personaje_vidas);

	printf("%s\n", personaje->personaje_orquestador->IP);

	printf("%d\n", personaje->personaje_orquestador->PORT);
}

/*NAME: create_personaje
PARAM: t_config *n -> una instancia de t_config con los valores de un archivo de ocnfiguracion
RETURN: t_personaje * -> el personaje creado
DESC: con las funciones de commons/config.h va tomando los valores del t_config dependiendo de la key pasada
	como parametro a cada función
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

void imprimir_lista_niveles(t_list *list){

	int i = 0;
	t_personaje_nivel *niv;
	t_personaje_objetivo *obj;

	while(i < (list_size(list))){
		niv = list_get(list, i);

		printf("\n%s\n", niv->personaje_nivel);

		int j = 0;
		while(j < (list_size(niv->personaje_objetivos))){
			obj = list_get(niv->personaje_objetivos,j);

			printf("%c\n", obj->objetivo);
			printf("%d\n", obj->tiene_objetivo);
			j++;
		}
		printf("\n");
		i++;
	}
}

void liberar_memoria_personaje(t_personaje *personaje){
	int i = 0;
	t_personaje_nivel *niv;

	while(i < (list_size(personaje->personaje_niveles))){
		niv = list_get(personaje->personaje_niveles, i);
		int j = 0;
		while(j < (list_size(niv->personaje_objetivos))){
			free(niv->personaje_objetivos);

			j++;
		}
		free(niv);
		i++;
	}
	list_clean(personaje->personaje_niveles);
	free(personaje);
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

