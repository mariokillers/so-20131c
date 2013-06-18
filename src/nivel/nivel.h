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

void BorrarItem(ITEM_NIVEL** i, char id);

void MoverPersonaje(ITEM_NIVEL* ListaItems, char id, int x, int y);



Posicion obtenerPosRecurso(char recurso);

t_recursos* liberarRecursos(PersonajeEnNivel* personaje );

int validarPosYRecursos(PersonajeEnNivel* personaje, char idRecurso);

PersonajeEnNivel* cargarPersonajeEnNivel(Personaje* miPersonaje);

void cargarPersonajeEnPendiente(char id);

void modificarPosPersonaje(PersonajeEnNivel* personaje, int posx, int posy);

void agregarRecursoAPersonaje(PersonajeEnNivel* personaje,char recurso);

void borrarPersonajeEnNivel(char idPersonaje);

void aumentarRecursos(t_recursos* recursosALiberar);

ITEM_NIVEL* buscarItem(char idRecurso);

void agregarARecursosPendientes(PersonajeEnNivel* personaje, char recurso);

void mandarRecursosLiberados(t_recursos* recursosALiberar, int fdOrquestador);

void agregarRecursosAListaItems(char idRecurso, int cant);

void reasignarRecursos(Recursos* listaRecursos);

void quitarSolicitudesDeRecurso(PersonajeEnNivel* personaje, char idRecurso);

PersonajeEnNivel *buscarPersonaje_byfd(int fd);

void* interbloqueo(void*);

int buscarEnReferenciaRecurso(char idRecurso, char referenciaRecurso[]);

int buscarEnReferenciaPersonaje(char idProceso, char referenciaProceso[]);

int cantidadPersonajes();

int cantidadRecursos();

void cargarRecursosTotales(int recursosTotales[], int cantRecursos , char referenciaRecurso[]);

void cargarRecursosDisponibles(int recursosDisponibles[], char referenciaRecurso[]);

void cargarRecursosSolicitados(int recursosSolicitados[][], char referenciaRecurso[], char referenciaPersonaje[]);

void cargarRecursosAsignados(int recursosAsignados[][], char referenciaRecurso[], char referenciaPersonaje[]);

void inicializarMarcados (bool marcados[], int cantidadPersonajes);

void inicializarReferenciaRecurso(int cantidadRecursos, char referenciaRecurso[]);

void inicializarReferenciaPersonaje(int cantidadPersonajes, char referenciaPersonaje[]);

void comprobarDeadlock (bool marcados[],int cantPersonajes, char referenciaPersonaje[]);

void marcarPersonajesConRecursos (int recursosAsignados[][], int recursosSolicitados[][], int recursosDisponibles[], bool marcados[], int cantPersonajes, int cantRecursos);

void marcarPersonajesSinRecursos (int recursosAsignados[][], char referenciaPersonaje[], bool marcados[], int cantPersonajes, int cantRecursos);

char* tomarIP(char* direct);

int tomarPuerto(char* direct);



#endif /* NIVEL_H_ */
