/*
 * Proceso Nivel.h
 *
 *  Created on: 30/04/2013
 *      Author: petauriel
 */

#ifndef PROCESO_NIVEL_H_
#define PROCESO_NIVEL_H_

Posicion obtenerPosRecurso(char recurso);
t_recursos liberarRecursos(char idPersonaje);
int validarPosYRecursos(char idPersonaje, char idRecurso);
void cargarPersonajeEnNivel(char id);
void cargarPersonajeEnPendiente(char id);
void agregarRecursoAPersonaje(char idPersonaje,char recurso);
void borrarPersonajeEnNivel(char idPersonaje);
void borrarPersonajeEnPendiente(char idPersonaje);
void aumentarRecursos(t_recursos recursosALiberar);
void modificarPosPersonaje(char idPersonaje, int posx, int posy);
PersonajeEnNivel buscarPersonaje(char idPersonaje);
ITEM_NIVEL buscarItem(char idRecurso);
void actualizarListaRecursosPendientes(char idPersonaje, char recurso);
void mandarRecursosLiberados(t_recursos recursosALiberar, int fd);
void agregarRecursosAListaItems(ITEM_NIVEL recurso, int cant);



//creo estructura de datos para yo:Nivel poder tener el seguimiento del personaje en mi nivel

typedef struct PersonajeEnNivel{
	char id;
	Posicion pos;
	t_recursos *recursos;
	struct PersonajeEnNivel *sig;
} PersonajeEnNivel;

typedef struct t_recursos{
	char idRecurso;
	int cant;
	t_recursos *sig;
}t_recursos;

typedef struct RecursoPendientePersonaje{
	char idPersonaje;
	char recursoPendiente;
	RecursoPendientePersonaje *sig;
}RecursoPendientePersonaje;


#endif /* PROCESO_NIVEL_H_ */


//HACER UNA FUNCION QUE RECORRA PersonajeEnNivel y que me devuelva, por recurso, la cantidad que tiene ese personaje
