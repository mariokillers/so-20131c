/*
 * interbloqueo.h
 *
 *  Created on: 20/06/2013
 *      Author: utnso
 */

#ifndef INTERBLOQUEO_H_
#define INTERBLOQUEO_H_

int recovery, recovery_time;
pthread_mutex_t deadlock_mutex;
pthread_t thread_interbloqueo;

int cantidadPersonajes();

int cantidadRecursos();

void cargarRecursosTotales(int *recursosTotales, int cantRecursos , char *referenciaRecurso);

void cargarRecursosDisponibles(int *recursosDisponibles, char *referenciaRecurso);
void cargarRecursosSolicitados(int cantRecursos, int **recursosSolicitados, char *referenciaRecurso, char *referenciaPersonaje);
//void cargarRecursosSolicitados(int **recursosSolicitados, char *referenciaRecurso, char *referenciaPersonaje);
void cargarRecursosAsignados(int cantRecursos, int **recursosAsignados, char *referenciaRecurso, char *referenciaPersonaje);
//void cargarRecursosAsignados(int **recursosAsignados, char *referenciaRecurso, char *referenciaPersonaje);

void inicializarMarcados (bool *marcados, int cantidadPersonajes);

void inicializarReferenciaRecurso(int cantidadRecursos, char *referenciaRecurso);

void inicializarReferenciaPersonaje(int cantidadPersonajes, char *referenciaPersonaje);

void comprobarDeadlock (bool *marcados,int cantPersonajes, char *referenciaPersonaje);

void marcarPersonajesConRecursos (int **recursosAsignados, int **recursosSolicitados, int *recursosDisponibles, bool *marcados, int cantPersonajes, int cantRecursos, char *referenciaPersonaje);

void marcarPersonajesSinRecursos (int **recursosAsignados, char *referenciaPersonaje, bool *marcados, int cantPersonajes, int cantRecursos);


#endif /* INTERBLOQUEO_H_ */
