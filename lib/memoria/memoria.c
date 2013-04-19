#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memoria.h"


t_memoria crear_memoria(int tamanio)
{
	extern t_list* l_particiones;
	extern int tamanio_segmento;
	t_memoria segmento;
	
	puts("CREAR");
	segmento = malloc(tamanio);
	l_particiones = list_create();
	
	if (segmento != NULL) {
		tamanio_segmento = tamanio;
	}
	else {
		tamanio_segmento = 0;
	}
	return segmento;
}
/*
Devuelve una posicion de inicio para ubicar una partición dentro del segmento
 */
int ubicar_particion(int tamanio, t_memoria segmento)
{
	extern t_list* l_particiones;
	extern int tamanio_segmento;
	int posicion = (int)segmento;
	t_particion* part;
	int i = 0;
	
	if (tamanio > tamanio_segmento) {
		return -2;
	}
	
	while ((part = list_get(l_particiones, i++)) != NULL && segmento + posicion < tamanio_segmento) {
		/* Encontrar el primer hueco que alcance para la particion */
		if (posicion > part->inicio + part->tamanio || posicion + tamanio < part->inicio) {
			return posicion - (int)segmento;
		} else {
			posicion++;
		}
	}
	
	return -1; /* No hay espacio suficientemente grande */
}

int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido)
{
	extern t_list* l_particiones;
	t_particion part;
	
	part.id = id;
	part.tamanio = tamanio;
	part.dato = contenido;
	part.libre = false; /* ??? */
	part.inicio  = ubicar_particion(part.tamanio, segmento);
	
	puts("ALMACENAR");
	switch (part.inicio) {
		/* cambiar esto a enums, es un asco asi */
		case -2:
			return -1;
		case -1:
			return 0;
	}
	
	strcpy((char*)(segmento + part.inicio), contenido);
	list_add(l_particiones, &part); /* Copia el contenido o pierdo lo que estaba en el stack? */
	
	return 1;
}

int eliminar_particion(t_memoria segmento, char id)
{
	extern t_list* l_particiones;
	
	puts("ELIMINAR");
	list_remove(l_particiones, id - 'A');
	return 0;
}

void liberar_memoria(t_memoria segmento)
{
	puts("LIBERAR");
	extern t_list* l_particiones;
	
	list_destroy(l_particiones);
	free(segmento);
	
}

t_list* particiones(t_memoria segmento)
{
	extern t_list* l_particiones;
	return l_particiones;
}
