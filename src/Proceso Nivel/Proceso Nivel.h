/*
 * Proceso Nivel.h
 *
 *  Created on: 30/04/2013
 *      Author: petauriel
 */

#ifndef PROCESO_NIVEL_H_
#define PROCESO_NIVEL_H_

Posicion obtenerPosRecurso(char recurso);
void pedirDireccionRecurso();
void cambiarEstado();
Recursos liberarRecursos(char idPersonaje,PersonajeEnNivel* listaPersonajes );
int validarPosYRecursos(char idPersonaje, char idRecurso);
void cargarPersonaje(PersonajeEnNivel** listaPersonajes, char id);
void agregarRecursoAPersonaje(PersonajeEnNivel** listaPersonajes, char idPersonaje,char recurso);
void borrarPersonaje(PersonajeEnNivel** listaPersonajes, char idPersonaje);
void aumentarRecursos(Recursos recursosALiberar);
void modificarPosPersonaje(PersonajeEnNivel** listaPersonajes,char idPersonaje, int posx, int posy);
t_recursos actualizarListaPersonaje(t_recursos** listaRecursos, char idRecurso);


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

typedef struct recursos{
	char idRecurso;
	int cant;
	t_recursos *sig;
}t_recursos;


#endif /* PROCESO_NIVEL_H_ */


//HACER UNA FUNCION QUE RECORRA PersonajeEnNivel y que me devuelva, por recurso, la cantidad que tiene ese personaje
