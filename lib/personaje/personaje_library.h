/*
 * personaje_library.h
 *
 *  Created on: 16/04/2013
 *      Author: utnso
 */

#ifndef PERSONAJE_LIBRARY_H_
#define PERSONAJE_LIBRARY_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "commons/config.h"
#include "commons/string.h"
#include "commons/collections/list.h"




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

#endif /* FPERSONAJE_LIBRARY_H_ */
