#include <stdio.h>
#include <stdlib.h>
#include "Proceso_Plataforma.h"

int main (){

	t_log* logger = log_create("testing.log", "ProcesoPlataforma", true, LOG_LEVEL_INFO);
	char message[500];

	log_info(logger, "************************************************************");

	t_stack* stack_personajes = stack_create();
	t_queue* queue_personajes = queue_create();

	char nombre[30];

	log_info(logger, "Ingrese los personajes. Para finalizar ingrese 'fin':");

	scanf("%s", nombre);

	while (strcmp(nombre, "fin") != 0) {
		stack_push(stack_personajes, personaje_crear(nombre));
		queue_push(queue_personajes, personaje_crear(nombre));
		scanf("%s", nombre);
	}

	sprintf(message, "El tamaño de la lista es %d", stack_size(stack_personajes));
	log_info(logger, message);

	log_info(logger, "************************************************************");
	log_info(logger, "Extrayendo elementos de la pila:");

	while (stack_is_empty(stack_personajes) == false) {
		Elemento_personaje* personaje_recibido = stack_pop(stack_personajes);
		log_info(logger, personaje_recibido->nombre);
	}

	log_info(logger, "************************************************************");
	log_info(logger, "Extrayendo elementos de la cola:");

	while (queue_is_empty(queue_personajes) == false) {
		Elemento_personaje* personaje_recibido = queue_pop(queue_personajes);
		log_info(logger, personaje_recibido->nombre);
	}

	while (stack_is_empty(stack_personajes) == false) {
		Elemento_personaje* personaje_recibido = stack_pop(stack_personajes);
		log_info(logger, personaje_recibido->nombre);
	}

	log_destroy(logger);

	return 0;
}

Elemento_personaje* personaje_crear(char* nombre) {
	Elemento_personaje* personaje = malloc(sizeof(Elemento_personaje));
	strncpy(personaje->nombre, nombre, 24);
	return personaje;
}
