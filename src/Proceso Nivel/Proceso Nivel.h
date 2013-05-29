/*

 * Proceso Nivel.h

 *

 *  Created on: 30/04/2013

 *      Author: petauriel

 */



#ifndef PROCESO_NIVEL_H_

#define PROCESO_NIVEL_H_



#include "DibujoNivelLib/tad_items.h"

#include <stdlib.h>

#include <curses.h>

#include <commons/log.h>

#include "commons/Connections/Server.h"

#include "commons/Connections/EstructurasMensajes.h"

#include "commons/Connections/Client.h"

#include "commons/config.h"

#include "personaje/personaje_library.h"





//listaItems = Resources (al principio)

//PersonajeEnNivel = Allocation

//RecursoPendientePersonaje = Claim

// RECURSOSDISPONIBLES = Available





//creo estructura de datos para yo:Nivel poder tener el seguimiento del personaje en mi nivel







//ESTO SIRVE PARA DEADLOCK: RECURSOSDISPONIBLES (AVAILABLE)

typedef struct t_recursos{

	char idRecurso;

	int cant;

	struct t_recursos *sig;

}t_recursos;



typedef struct PersonajeEnNivel{

	char id;

	Posicion pos;

	t_recursos *recursos;

	struct PersonajeEnNivel *sig;

	int fd;

} PersonajeEnNivel;



typedef struct RecursoPendientePersonaje{

	char idPersonaje;

	char recursoPendiente;

	struct RecursoPendientePersonaje *sig;

}RecursoPendientePersonaje;





/*define el tipo t_nivel, que representa un nivel creado a partir de un archivo de configuracion dado

con toda su estructura de datos

	*/

typedef struct t_nivel{

	ITEM_NIVEL *nivel_items;

	Direccion* nivel_orquestador;

	char *nivel_nombre;

	long nivel_tiempo_deadlock;

	int nivel_recovery;

	} t_nivel;



t_nivel *read_nivel_archivo_configuracion(char* path);

t_nivel *create_nivel(t_config *n);

ITEM_NIVEL *create_lista_cajas(t_config *n);

void imprimir_lista_cajas(t_list *cajas);

void ListItems_add_caja(t_config *n, char *buffer_caja_num, ITEM_NIVEL **list);





/*funciones para usar en el dibujo de nivel

	*/

ITEM_NIVEL *crear_lista_items(t_list *cajas, t_list *personajes);

//void add_personaje_item_nivel(t_personaje_en_nivel *personaje, ITEM_NIVEL **list, char tipo);

void BorrarItem(ITEM_NIVEL** i, char id);

void MoverPersonaje(ITEM_NIVEL* ListaItems, char id, int x, int y);



Posicion obtenerPosRecurso(char recurso);

t_recursos* liberarRecursos(char idPersonaje);

int validarPosYRecursos(char idPersonaje, char idRecurso);

void cargarPersonajeEnNivel(Personaje* miPersonaje);

void cargarPersonajeEnPendiente(char id);

void agregarRecursoAPersonaje(char idPersonaje,char recurso);

void borrarPersonajeEnNivel(char idPersonaje);

void borrarPersonajeEnPendiente(char idPersonaje);

void aumentarRecursos(t_recursos* recursosALiberar);

void modificarPosPersonaje(char idPersonaje, int posx, int posy);

PersonajeEnNivel* buscarPersonaje(char idPersonaje);

ITEM_NIVEL* buscarItem(char idRecurso);

void agregarAListaRecursosPendientes(char idPersonaje, char recurso);

void mandarRecursosLiberados(t_recursos* recursosALiberar, int fd);

void agregarRecursosAListaItems(char idRecurso, int cant);

void reasignarRecursos(Recursos* listaRecursos);

void quitarSolicitudesDeRecurso(char idPersonaje, char idRecurso);

char buscarPersonaje_byfd(int fd);

//funciones para tratar interbloqueo

void* interbloqueo(void*);

int buscarEnReferenciaRecurso(char idRecurso, char referenciaRecurso[]);

int buscarEnReferenciaProceso(char idProceso, char referenciaProceso[]);

int cantidadPersonajes();

int cantidadRecursos();

void cargarRecursosTotales(int recursosTotales[], int cantRecursos , char referenciaRecurso[]);

void cargarRecursosDisponibles(int recursosDisponibles[], int cantRecursos , char referenciaRecurso[]);

void cargarRecursosSolicitados(int recursosSolicitados[][]);

void cargarRecursosAsignados(int recursosAsignados[][]);



char* tomarIP(char* direct);

int tomarPuerto(char* direct);



#endif /* PROCESO_NIVEL_H_ */
