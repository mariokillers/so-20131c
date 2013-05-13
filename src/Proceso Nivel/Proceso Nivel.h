/*
 * Proceso Nivel.h
 *
 *  Created on: 30/04/2013
 *      Author: petauriel
 */

#ifndef PROCESO_NIVEL_H_
#define PROCESO_NIVEL_H_

int obtenerPosRecursoX(char recurso, ITEM_NIVEL *ListaItems);
int obtenerPosRecursoY(char recurso, ITEM_NIVEL *ListaItems);
void pedirDireccionRecurso();
void cambiarEstado();
Recursos liberarRecursos(char idPersonaje,PersonajeEnNivel* listaPersonajes );
int validarPosYRecursos(ITEM_NIVEL *ListaItems, char idPersonaje, char idRecurso);
void cargarPersonaje(PersonajeEnNivel** listaPersonajes, char id);
void agregarRecursoAPersonaje(PersonajeEnNivel** listaPersonajes, char idPersonaje,char recurso);
void borrarPersonaje(PersonajeEnNivel** listaPersonajes, char idPersonaje);
void aumentarRecursos(ITEM_NIVEL *,Recursos recursosALiberar);
/*int chequearMovimiento(char idPersonaje, Posicion pos);
Posicion posPersonaje(char idPersonaje);*/

//creo estructura de datos para yo:Nivel poder tener el seguimiento del personaje en mi nivel

typedef struct PersonajeEnNivel{
	char id;
	Posicion pos;
	t_recursos *recursos;
	struct PersonajeEnNivel *sig;
} PersonajeEnNivel;

typedef struct t_recursos{
	char idRecurso;
	t_recursos *sig;
}t_recursos;

#endif /* PROCESO_NIVEL_H_ */
