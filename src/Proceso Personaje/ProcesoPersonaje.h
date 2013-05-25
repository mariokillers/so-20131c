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



#include "../../lib/commons/Connections/Client.h"
#include "../../lib/commons/Connections/Mensajes.h"
#include "../../lib/commons/Connections/EstructurasMensajes.h"
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../lib/commons/config.h"
#include "../../lib/commons/string.h"
#include "../../lib/commons/collections/list.h"





/*define el tipo t_personaje_objetivo, que representa un objetivo de los que tiene que conseguir el personaje
en un nivel y si lo tiene o no
	*/
typedef struct t_personaje_objetivo{
	char objetivo;
	bool tiene_objetivo;
	struct t_personaje_objetivo *sig;
} t_personaje_objetivo;

/*define el tipo t_personaje_nivel, que representa un nivel que tiene que ganar un personaje y sus objetivos
	*/
typedef struct t_personaje_nivel{
	char *personaje_nivel;
	t_list *personaje_objetivos;
	bool termino_nivel;
	struct t_personaje_nivel *sig;
} t_personaje_nivel;

/*define t_personaje, que representa un personaje creado en base a un archivo de configuracion dado
y su estructura de datos
	*/
typedef struct t_personaje {
	char *personaje_nombre;
	char personaje_simbolo;
	t_list *personaje_niveles;
	int personaje_vidas;
	int personaje_vidas_restantes;
	Direccion *personaje_orquestador;
	Posicion *personaje_posicion_actual;
} t_personaje;

t_personaje *read_personaje_archivo_configuracion(char* path);
t_personaje *create_personaje(t_config *p);
t_personaje_nivel *create_personaje_nivel(char *nivel, char **objetivos);
void add_list_personaje_niveles(char **arr, char *buffer_nivel, t_list *list);
void imprimir_lista_niveles(t_list *list);
void liberar_memoria_personaje(t_personaje *personaje);
t_list *create_lista_niveles(t_personaje *personaje, t_config *p);
void imprimir_personaje(t_personaje *personaje);
t_personaje_objetivo *create_nivel_objetivo(char objetivo_char);
void add_list_nivel_objetivos(char *objetivo, t_list *list_objetivos);

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
bool ganado(t_list *niveles);
Posicion *realizarMovimiento(Posicion *posActual, Posicion *posProxRec, CCB clientCCB_niv);
void morir(t_personaje *personaje, t_personaje *personaje_init, CCB clientCCB_niv, Posicion *posProxRec, char state, char *nivActual);
void rutinaSignal(int n);




#endif /* PROCESOPERSONAJE_H_ */
