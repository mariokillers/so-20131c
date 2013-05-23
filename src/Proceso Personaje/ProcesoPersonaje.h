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

#include <Connections/Client.h>
#include </../personaje/personaje_library.h>
#include <signal.h>

int _is_next_level(t_personaje_nivel *p);
char *proximoNivel(t_list niveles);
char *transformNivel_to_send(char *nivel);
int _is_next_obj(t_personaje_objetivo *o);
char proximoRecurso(t_list *niveles, char *nivActual);
Posicion proximaPosicion(Posicion posActual, Posicion posProxRec);
void reiniciarNivel(t_list *niveles, char *nivActual, char proxRec);
Posicion realizarMovimiento(Posicion *posActual, Posicion posProxRec, CBB clientCCB_niv, CBB clientCBB_pln, char *state, char proxRec);
bool recursoAlcanzado(Posicion pos1, Posicion pos2);
void agregarRecurso(t_list *niveles, char *nivActual);
bool nivelTerminado(t_list *niveles, char *nivActual);





#endif /* PROCESOPERSONAJE_H_ */
