/*
 * ProcesoPersonaje.h
 *
 *  Created on: 20/05/2013
 *      Author: utnso
 */

#ifndef PROCESOPERSONAJE_H_
#define PROCESOPERSONAJE_H_

#define STANDBY 0
#define WAIT_DATA_LEVEL 1
#define WAIT_POS_REC 2
#define WAIT_REC 3
#define NUEVO_NIVEL 4
#define WIN 5

#include "personaje_library.h"
#include <signal.h>
#include "Client.h"
#include "EstructurasMensajes.h"
#include "Mensajes.h"

typedef struct {
	int instancia_epoll;			// Instancia epoll
	int sockfd, masterfd; 			// Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
	struct epoll_event event;		//
	struct epoll_event *events;
	struct sockaddr_in my_addr; 	// Información sobre mi dirección
} CBB;

int _is_next_level(t_personaje_nivel *p);
char *proximoNivel(t_list *niveles);
char *transformNivel_to_send(char *nivel, char **miNivAux);
int _is_next_obj(t_personaje_objetivo *o);
char proximoRecurso(t_list *niveles, char *nivActual);
Posicion *proximaPosicion(Posicion *posActual, Posicion *posProxRec);
void reiniciarNivel(t_list *niveles, char *nivActual);
void analizarRecurso(Posicion *posActual, Posicion *posProxRec, CCB clientCCB_niv, CCB clientCBB_pln, char *state, char proxRec);
bool recursoAlcanzado(Posicion *pos1, Posicion *pos2);
void agregarRecurso(t_list *niveles, char *nivActual, char proxRec);
bool nivelTerminado(t_list *niveles, char *nivActual);
bool ganado(t_list *niveles, char *nivActual);
Posicion *realizarMovimiento(Posicion *posActual, Posicion *posProxRec, CCB clientCCB_niv);





#endif /* PROCESOPERSONAJE_H_ */
