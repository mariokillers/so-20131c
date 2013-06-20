#include <stdbool.h>
#include <stdlib.h>
#include "interbloqueo.h"
#include "nivel.h"

void *interbloqueo(void* a){
	/*@NAME: interbloqueo
	* @DESC: hilo que se encarga de detectar interbloqueo
	*/

	while(1) {
		char *referenciaPersonaje, *referenciaRecursos;
		int *recursosTotales, *recursosDisponibles, *aux, **recursosAsignados, **recursosSolicitados;
		bool *marcados;
		if (!pthread_mutex_trylock(&deadlock_mutex)) { //esta terminando nivel
			pthread_mutex_unlock(&deadlock_mutex);
			free(aux);
			free(recursosAsignados);
			free(recursosSolicitados);
			free(recursosTotales);
			free(recursosDisponibles);
			free(referenciaPersonaje);
			free(referenciaRecursos);
			free(marcados);
			return NULL;
		}
		log_info(logger, "Empieza a ejecutar el hilo interbloqueo");

			//entro en la region critica
			pthread_mutex_lock(&mutex);

			log_info(logger, "El hilo interbloqueo entra en la region critica");

			int cantPersonajes = cantidadPersonajes();
			int cantRecursos = cantidadRecursos();

			log_info(logger, string_from_format("La cantidad de personajes es: %d y de recursos es: %d", cantPersonajes, cantRecursos));

			//vector para saber que procesos estan interbloqueados
			marcados = malloc(cantPersonajes * sizeof(char));

			inicializarMarcados(marcados, cantPersonajes);

			//vectores que referencian en la posicion de matrices y vectores para detectar interbloqueo
			referenciaPersonaje = malloc(cantPersonajes * sizeof(char));
			referenciaRecursos = malloc(cantRecursos * sizeof(char));

			inicializarReferenciaRecurso(cantRecursos, referenciaRecursos);
			inicializarReferenciaPersonaje(cantPersonajes, referenciaPersonaje);

			//vectores para interbloqueo
			recursosTotales = malloc(cantRecursos * sizeof(int));
			recursosDisponibles = malloc(cantRecursos * sizeof(int));

			//matrices para interbloqueo
			recursosAsignados = malloc(cantPersonajes * cantRecursos * sizeof(int));
			recursosSolicitados = malloc(cantPersonajes * cantRecursos * sizeof(int));

			aux = malloc(cantRecursos * sizeof(int));

			//inicializo los vectores-matrices
			cargarRecursosTotales(recursosTotales, cantRecursos, referenciaRecursos);
			cargarRecursosDisponibles(aux, referenciaRecursos);
			cargarRecursosSolicitados(recursosSolicitados, referenciaRecursos, referenciaPersonaje);
			cargarRecursosAsignados(recursosAsignados, referenciaRecursos, referenciaPersonaje);

			pthread_mutex_unlock(&mutex);
			//salgo de la region critica

			log_info(logger, "El hilo interbloqueo sale de la region critica");


			marcarPersonajesSinRecursos(recursosAsignados,referenciaPersonaje,marcados,cantPersonajes, cantRecursos);
			marcarPersonajesConRecursos(recursosAsignados, recursosSolicitados, recursosDisponibles, marcados,cantPersonajes, cantRecursos);
			comprobarDeadlock(marcados,cantPersonajes, referenciaPersonaje);

			usleep(recovery_time);
	}
	return NULL;
}
