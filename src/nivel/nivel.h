#ifndef NIVEL_H_
#define NIVEL_H_

#include "tad_items.h"
#include <stdlib.h>
#include <curses.h>
#include <commons/Connections/Server.h>
#include <commons/log.h>
#include <commons/Connections/EstructurasMensajes.h>
#include <commons/Connections/Client.h>
#include <commons/config.h>
//#include "personaje/personaje_library.h"

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

	char recursoPendiente;

	bool marcado;

	struct PersonajeEnNivel *sig;

	int fd;

} PersonajeEnNivel;



/*typedef struct RecursoPendientePersonaje{

	char idPersonaje;

	char recursoPendiente;

	struct RecursoPendientePersonaje *sig;

}RecursoPendientePersonaje;*/





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

int validarPosYRecursos(int fdPersonaje, char *mensaje);

void cargarPersonajeEnNivel(Personaje* miPersonaje, int fd);

void cargarPersonajeEnPendiente(char id);

void agregarRecursoAPersonaje(char idPersonaje,char recurso);

void borrarPersonajeEnNivel(char idPersonaje);

void aumentarRecursos(t_recursos* recursosALiberar);

void modificarPosPersonaje(int fdPersonaje, int posx, int posy);

PersonajeEnNivel* buscarPersonaje(char idPersonaje);

ITEM_NIVEL* buscarItem(char idRecurso);

void agregarARecursosPendientes(char idPersonaje, char recurso);

void mandarRecursosLiberados(t_recursos* recursosALiberar, int fd);

void agregarRecursosAListaItems(char idRecurso, int cant);

void reasignarRecursos(Recursos* listaRecursos);

void quitarSolicitudesDeRecurso(char idPersonaje, char idRecurso);

PersonajeEnNivel *buscarPersonaje_byfd(int fd);

char* tomarIP(char* direct);

int tomarPuerto(char* direct);



#endif /* NIVEL_H_ */
