/*
 * ProcesoPersonaje.c
 *
 *  Created on: 20/05/2013
 *      Author: utnso
 */


#include "ProcesoPersonaje.h"

int ProcesoPersonaje(char *PATH){

	//inicializo el personaje desde el archivo config

	t_personaje *personaje= read_personaje_archivo_configuracion(PATH);

	//variables locales
	char state = NUEVO_NIVEL;
	Posicion *posProxRec = malloc(sizeof(Posicion));
	char proxRec;
	char* proxNiv;
	char *nivActual;
	strcpy(nivActual,"");
	Posicion *nuevaPos = malloc(sizeof(Posicion));

	CCB clientCCB_orq;
	CCB clientCCB_niv;
	CCB clientCCB_pln;

	//inicializo la cola de mensajes

	t_queue* colaDeMensajes;
	colaDeMensajes = queue_create();
	Mensaje* mensaje;

	//mientras tenga algun mensaje del Server

		while(1){
			//analiza el estado actual del proceso y en base a eso, actua
			switch (state){
				case NUEVO_NIVEL:
					clientCCB_orq = connectServer(personaje->personaje_orquestador->IP, personaje->personaje_orquestador->PORT);

					//verifico si gane
					if(ganado(personaje->personaje_niveles, nivActual)){
						mandarMensaje(clientCCB_orq.sockfd,GANE,sizeof(personaje->personaje_simbolo),personaje->personaje_simbolo);

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
								Data_Nivel *data_new = mensaje->data;
								Nivel nivel_new = data_new->miNivel;
								Planificador planificador_new = data_new->miPlanificador;

								char *nivel_IP = nivel_new.IP;
								int nivel_port = nivel_new.PORT;
								char *planificador_IP = planificador_new.IP;
								int planificador_port = planificador_new.PORT;

								//desconecta del orquestador y conecta a nivel y planificador
								close(clientCCB_orq.sockfd);

								clientCCB_niv = connectServer(nivel_IP, nivel_port);
								clientCCB_pln = connectServer(planificador_IP, planificador_port);

								state = STANDBY;

								break;
						}
					}
				break;

		case STANDBY:
					if (mensajes(colaDeMensajes, clientCCB_pln)){
						mensaje = queue_pop(colaDeMensajes);

						switch(mensaje->type){
							case MOVIMIENTO_PERMITIDO:
								if(posProxRec != NULL){
									nuevaPos = realizarMovimiento(personaje->personaje_posicion_actual, posProxRec, clientCCB_niv);
									personaje->personaje_posicion_actual = nuevaPos;
									analizarRecurso(personaje->personaje_posicion_actual, posProxRec, clientCCB_niv, clientCCB_pln, &state, proxRec);
								} else{
									//solicitar posicion proximo recurso
									proxRec = proximoRecurso(personaje->personaje_niveles, nivActual);

									mandarMensaje(clientCCB_niv.sockfd,REQUEST_POS_RECURSO, sizeof(proxRec),proxRec);

									state = WAIT_POS_REC;
								}
								break;

							case MORISTE_PERSONAJE:
								mandarMensaje(clientCCB_niv.sockfd,TERMINE_NIVEL,sizeof(NULL),NULL);
								personaje->personaje_vidas_restantes --;

								//si no le quedan vidas, reinicia todo de nuevo => comienza Personaje de nuevo
								if(personaje->personaje_vidas_restantes == 0){
									printf("muerte del personaje %c por interbloqueo", personaje->personaje_simbolo);
									ProcesoPersonaje(PATH);
								} else{
									reiniciarNivel(personaje->personaje_niveles, nivActual);
									proxRec = '';

									state = STANDBY;
								}
								break;
						}
				case WAIT_POS_REC:
					if (mensajes(colaDeMensajes, clientCCB_niv)){
						mensaje = queue_pop(colaDeMensajes);

						switch(mensaje->type){
							case POSICION_RECURSO:
								posProxRec = mensaje->data;
								nuevaPos = realizarMovimiento(personaje->personaje_posicion_actual, posProxRec, clientCCB_niv);
								personaje->personaje_posicion_actual = nuevaPos;
								analizarRecurso(personaje->personaje_posicion_actual, posProxRec, clientCCB_niv, clientCCB_pln, &state, proxRec);
						}
						break;
					}

				case WAIT_REC:
					if (mensajes(colaDeMensajes, clientCCB_niv)){
						mensaje = queue_pop(colaDeMensajes);

						switch(mensaje->type){
							case CONFIRMAR_RECURSO:
								//si le otorgaron el recurso
								if(mensaje->data){
									mandarMensaje(clientCCB_pln.sockfd,TERMINE_TURNO,sizeof(NULL),NULL);
									agregarRecurso(personaje->personaje_niveles, nivActual);
									proxRec = '';
									posProxRec = NULL;

									if(nivelTerminado(personaje->personaje_niveles, nivActual)){
										mandarMensaje(clientCCB_niv.sockfd,TERMINE_NIVEL,sizeof(NULL),NULL);

										close(clientCCB_niv.sockfd);
										close(clientCCB_pln.sockfd);

										state = NUEVO_NIVEL;
									}else{
										state = STANDBY;
									}

								} else{
									mandarMensaje(clientCCB_pln.sockfd,PERSONAJE_BLOQUEADO,sizeof(proxRec),proxRec);

									state = STANDBY;
								}
							}
						}
						break;
					}
			}

	return 0;
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
	if((posActual->POS_X < posProxRec->POS_X) && (posActual->POS_X != posProxRec->POS_X)){
		posActual->POS_X ++;
	} else if((posActual->POS_X > posProxRec->POS_X) && (posActual->POS_X != posProxRec->POS_X)){
		posActual->POS_X --;
	} else if((posActual->POS_Y > posProxRec->POS_Y) && (posActual->POS_Y != posProxRec->POS_Y)){
		posActual->POS_Y --;
	} else if((posActual->POS_Y < posProxRec->POS_Y) && (posActual->POS_Y != posProxRec->POS_Y)){
		posActual->POS_Y ++;
	}

	return posActual;
}

void reiniciarNivel(t_list *niveles, char *nivActual){
	t_personaje_nivel *nn;
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
}

bool recursoAlcanzado(Posicion *pos1, Posicion *pos2){
	return ((pos1->POS_X==pos2->POS_X)&&(pos1->POS_Y==pos2->POS_Y));
}


Posicion *realizarMovimiento(Posicion *posActual, Posicion *posProxRec, CBB clientCCB_niv){
	Posicion *nuevaPos = proximaPosicion(posActual, posProxRec);

	mandarMensaje(clientCCB_niv.sockfd,REQUEST_MOVIMIENTO,sizeof(nuevaPos),nuevaPos);

	return nuevaPos;
}

void analizarRecurso(Posicion *posActual, Posicion *posProxRec, CBB clientCCB_niv, CBB clientCBB_pln, char *state, char proxRec){
	if(recursoAlcanzado(posActual, posProxRec)){
		mandarMensaje(clientCCB_niv.sockfd,REQUEST_RECURSO,sizeof(proxRec),proxRec);

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
	t_personaje_objetivo *nn;
	while(i<len){
		auxObj = list_get(auxListObj, i);
		if(auxObj->objetivo == proxRec){
			auxObj->tiene_objetivo = true;
			nn = list_replace(niveles, i, auxObj);
			break;
		} else{
			i++;
		}
	}
}

bool nivelTerminado(t_list *niveles, char *nivActual){
	t_personaje_nivel *nn;
	int i = 0;
	int j = 0;
	int lenNiv = list_size(niveles);
	t_personaje_objetivo *auxObj;
	t_personaje_nivel *auxNiv;
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
			return false;
		}
	}
	auxNiv->termino_nivel = true;
	nn = list_replace(niveles, i, auxNiv);
	return true;
}

/*bool ganado(t_list *niveles, char *nivActual){

}
*/
