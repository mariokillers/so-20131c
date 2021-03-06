#include <stdbool.h>
#include <stdlib.h>
#include "interbloqueo.h"
#include "nivel.h"

void *interbloqueo(void* a){
	/*@NAME: interbloqueo
	* @DESC: hilo que se encarga de detectar interbloqueo
	*/
	char *referenciaPersonaje, *referenciaRecursos;
	int *recursosTotales, *recursosDisponibles, *recursosAsignados, *recursosSolicitados;
	bool *marcados;

	while(1) {
		if (!pthread_mutex_trylock(&deadlock_mutex)) { //esta terminando nivel
			pthread_mutex_unlock(&deadlock_mutex);
			return NULL;
		}

			//entro en la region critica
			pthread_mutex_lock(&mutex);

			int cantPersonajes = cantidadPersonajes();
			int cantRecursos = cantidadRecursos();

			log_info(loggerInterbloqueo, string_from_format("La cantidad de personajes es: %d y de recursos es: %d", cantPersonajes, cantRecursos));

			//vector para saber que procesos estan interbloqueados
			marcados = malloc(cantPersonajes * sizeof(bool));

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
			recursosAsignados = calloc(cantPersonajes * cantRecursos, sizeof(int));
			recursosSolicitados = calloc(cantPersonajes * cantRecursos, sizeof(int));


			//inicializo log vectores-matrices
			cargarRecursosTotales(recursosTotales, cantRecursos, referenciaRecursos);
			cargarRecursosDisponibles(recursosDisponibles, referenciaRecursos);
			cargarRecursosSolicitados(cantRecursos,recursosSolicitados, referenciaRecursos, referenciaPersonaje);
			cargarRecursosAsignados(cantRecursos,recursosAsignados, referenciaRecursos, referenciaPersonaje);


			marcarPersonajesSinRecursos(recursosAsignados,referenciaPersonaje,marcados,cantPersonajes, cantRecursos);
			marcarPersonajesConRecursos(recursosAsignados, recursosSolicitados, recursosDisponibles, marcados,cantPersonajes, cantRecursos, referenciaPersonaje);
			comprobarDeadlock(marcados,cantPersonajes, referenciaPersonaje);

			free(recursosAsignados);
			free(recursosSolicitados);
			free(recursosTotales);
			free(recursosDisponibles);
			free(referenciaPersonaje);
			free(referenciaRecursos);
			free(marcados);

			pthread_mutex_unlock(&mutex);
			//salgo de la region critica

			sleep(recovery_time);
			log_info(loggerInterbloqueo, "----------------------------------------------");
	}
	return NULL;
}

int cantidadPersonajes(){
	/*@NAME: cantidadProcesos
	* @DESC: devuelve la cantidad de personajes conectados al nivel
	*/
	int i =0;
	PersonajeEnNivel *personaje = listaPersonajes;

	while(personaje != NULL){
		i++;
		personaje= personaje->sig;
	}return i;
}

int cantidadRecursos(){
	/*@NAME: cantidadRecursos
	* @DESC: devuelve la cantidad de recursos que hay en el nivel
	* NOTA: como no hay personajes en esta lista no tengo que diferenciar recursos de personajes
	*/
	int i =0;
	ITEM_NIVEL *recurso = recursosIniciales;

	while(recurso != NULL){
		i++;
		recurso= recurso->next;
	}return i;
}

void inicializarMarcados(bool marcados[], int cantidadPersonajes) {
	/*@NAME: inicializarMarcados
	 * @DESC: inicializo el vector en false
	 */

	int i;
	for (i = 0; i < cantidadPersonajes; i++) {
		marcados[i] = false;
	}
}

void inicializarReferenciaPersonaje(int cantidadPersonajes,
		char referenciaPersonaje[]) {
	/*@NAME: inicializarReferenciaPersonaje
	 * @DESC: inicializo el vector de referencia de personaje con los personajes que hay
	 */

	int i;
	PersonajeEnNivel *personaje = listaPersonajes;

	for (i = 0; i < cantidadPersonajes; i++) {
		referenciaPersonaje[i] = personaje->id;
		personaje = personaje->sig;
	}
}

void inicializarReferenciaRecurso(int cantidadRecursos,
		char referenciaRecurso[]) {
	/*@NAME: inicializarMarcados
	 * @DESC: inicializo el vector de referencia de recursos con los recursos
	 */

	int i;
	ITEM_NIVEL *recurso = recursosIniciales;

	for (i = 0; i < cantidadRecursos; i++) {
		referenciaRecurso[i] = recurso->id;
		recurso = recurso->next;
	}
}

int buscarEnReferenciaRecurso(char idRecurso, char referenciaRecurso[]) {
	/*@NAME: buscarEnReferenciaRecurso
	 * @DESC: busca en el vector que hace referencia a los recursos la pos de ese recurso en las matrices/vectores
	 */
	int i = 0;
	bool encontrado = false;
	while (!encontrado) {
		if (referenciaRecurso[i] == idRecurso) {
			encontrado = true;
		} else {
			i++;
		}
	}
	return i;
}

int buscarEnReferenciaPersonaje(char idPersonaje, char referenciaPersonaje[]) {
	/*@NAME: buscarEnReferenciaProceso
	 * @DESC: busca en el vector que hace referencia a los personajes la pos de ese personaje en las matrices
	 */
	int i = 0;
	bool encontrado = false;
	while (!encontrado) {
		if (referenciaPersonaje[i] == idPersonaje) {
			encontrado = true;
		} else {
			i++;
		}
	}
	return i;
}

void cargarRecursosTotales(int *recursosTotales, int cantRecursos , char *referenciaRecurso){
	/*@NAME: cargarRecursosTotales
	* @DESC: completa el vector con la cantidad de recursos que hay en total
	*/

	int i;
	int pos =-1;

	ITEM_NIVEL *recurso = recursosIniciales;

	for(i=0; i< cantRecursos; i++){
		//busco en el vector referencia la pos de ese recurso
		pos = buscarEnReferenciaRecurso(recurso->id,referenciaRecurso);
		if(pos != -1){
			//le asigno a esa pos la cantidad de recursos que hay
			recursosTotales[pos] = recurso->quantity;
		}
		recurso = recurso->next;
	}

}

void cargarRecursosDisponibles(int recursosDisponibles[], char referenciaRecurso[]){
	/*@NAME: cargarRecursosDisponibles
	* @DESC: completa el vector con la cantidad de recursos que quedan sin asignar
	*/

	int pos =-1;

	ITEM_NIVEL *recurso = ListaItems;

	while(recurso!= NULL){
		//me fijo antes que sea recurso y NO personaje
		if ( recurso->item_type == 1){
			//busco en el vector referencia la pos de ese recurso
			pos = buscarEnReferenciaRecurso(recurso->id,referenciaRecurso);
			if(pos != -1){
				//le asigno a esa pos la cantidad de recursos que hay
				recursosDisponibles[pos] = recurso->quantity;
			}
		}
		recurso = recurso->next;
	}
}

void cargarRecursosSolicitados(int cantRecursos, int *recursosSolicitados, char *referenciaRecurso, char *referenciaPersonaje){
	/*@NAME: cargarRecursosSolicitados
	* @DESC: carga la matriz dependiendo de el recurso solicitado que tuvo cada personaje
	*/

	PersonajeEnNivel *personaje = listaPersonajes;
	char recurso;

	int posPersonaje = -1;
	int posRecurso = -1;

	while(personaje != NULL){

		//cargo el recurso que solicito el personaje
		recurso = personaje->recursoPendiente;

		if(recurso != NULL){

					//busco la posicion del personaje y el recurso en el vector de referencia
					posPersonaje = buscarEnReferenciaPersonaje(personaje->id,referenciaPersonaje );

					posRecurso = buscarEnReferenciaRecurso(recurso, referenciaRecurso);

					if((posRecurso != -1) && (posPersonaje != -1)){
						//en la fila del personaje, en la columna del recurso, pongo un 1 que es el recurso que solicito
						recursosSolicitados[posPersonaje*cantRecursos + posRecurso]= 1;
					}

		}
		personaje = personaje->sig;
	}
}

void cargarRecursosAsignados(int cantRecursos,int *recursosAsignados, char *referenciaRecurso, char *referenciaPersonaje){
	/*@NAME: cargarRecursosAsignados
	* @DESC: carga la matriz dependiendo de los recursos que tiene asignado cada personaje
	*/

	int posPersonaje = -1;
	int posRecurso = -1;

	PersonajeEnNivel *personaje = listaPersonajes;

	while(personaje != NULL){
		posPersonaje = buscarEnReferenciaPersonaje(personaje->id,referenciaPersonaje );
		//recorro la lista de recursos de ese personaje
		t_recursos *recurso = personaje->recursos;

		while(recurso != NULL){
			//busca la posicion en la matriz del char de ese recurso
			posRecurso = buscarEnReferenciaRecurso(recurso->idRecurso, referenciaRecurso);
			//en la fila del personaje, la columna del recurso, le asigna la cantidad que tiene asignado ese personaje
			recursosAsignados[posPersonaje*cantRecursos + posRecurso] = recurso->cant;
			//paso al siguiente recurso del personaje
			recurso = recurso->sig;
		}
		//paso al otro personaje
		personaje = personaje->sig;
	}
}

void marcarPersonajesSinRecursos (int *recursosAsignados, char *referenciaPersonaje, bool *marcados, int cantPersonajes, int cantRecursos){
	/*@NAME: marcarPersonajesSinRecursos
	* @DESC: marca a los personajes que no tienen recursos asignados
	*/

	int i,j, flag;
	for(i=0;i<cantPersonajes;i++){
		flag=0;
		for(j=0;j<cantRecursos;j++){
			//si tiene asignado algo... seteo el flag en 1
			if(recursosAsignados[i*cantRecursos +j]!= 0){
				flag=1;
			}
		}
		if (flag==0){
			marcados[i]=true;
		}
	}

}

void marcarPersonajesConRecursos (int *recursosAsignados, int *recursosSolicitados, int *recursosDisponibles, bool *marcados, int cantPersonajes, int cantRecursos, char *referenciaPersonaje){
	/*@NAME: marcarPersonajesConRecursos
	* @DESC: marca a los personajes que pueden ejecutar
	*/
	int i,j,asignacionImposible, flagTerminar;
	do{
		flagTerminar=0;
		//recorremos personajes
		for(i=0;i<cantPersonajes;i++){
			asignacionImposible=0;

			if(marcados[i]==false){

			//recorremos recursos del personaje actual
			for(j=0;j<cantRecursos;j++){

				//verifico que haya recursos susficientes para satisfacer el pedido
				if(marcados[i]==false && recursosSolicitados[i*cantRecursos +j]>recursosDisponibles[j]){
					asignacionImposible=1;
				}
			}

			//es posible ejecutar el personaje
			if(!asignacionImposible){
				//SI ENCUENTRA UNO QUE PUEDA EJECUTAR, SETEA PARA CONTINUAR EL ALGORTIMO
				flagTerminar=1;
				marcados[i]=true;

				//si se puede ejecutar, actualizo el disponible
				for(j=0;j<cantRecursos;j++){
					recursosDisponibles[j]+=recursosAsignados[i* cantRecursos +j];
				}
			}
			}
		}

	//si se encontro, termina el algoritmo
	}while(flagTerminar);

}

void comprobarDeadlock (bool marcados[],int cantPersonajes, char referenciaPersonaje[]){

	int i,j;
	j=0;
	char personajesInterbloqueados[cantPersonajes+1];
	//recorremos el vector de marcados
	for(i=0;i < cantPersonajes;i++){
		if(marcados[i]==false){
			//Si el personaje no esta marcado, esta comprometido en un deadlock.
			personajesInterbloqueados[j]=referenciaPersonaje[i];
			log_info(loggerInterbloqueo, string_from_format("El personaje: %c esta en deadlock", referenciaPersonaje[i]));
			j++;

		}
	}
	personajesInterbloqueados[j]='\0';

	if(recovery && personajesInterbloqueados[0]!='\0'){
		log_info(loggerInterbloqueo, string_from_format("Los personajes que estan en deadlock son: %s ", personajesInterbloqueados));
		mandarMensaje(clientCCB.sockfd,REQUEST_INTERBLOQUEO,strlen(personajesInterbloqueados)+1,personajesInterbloqueados);
	}
}
