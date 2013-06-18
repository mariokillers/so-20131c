
#include "memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "commons/string.h"

#define SUPERA_SEGMENTO -2
#define NO_ENTRA -3

int buscarSegmentoLibre(t_memoria segmento, int tamanio);
bool repiteID(char id);
bool analizarProximasParticiones(int x, int tamanio, int tamanio_anterior);
void particion_destroy(t_particion* part);
int cantidad_particiones();

void imprimirParticiones(t_memoria segmento);
void imprimirPrueba(t_memoria segmento);

//VARIABLES GLOBALES ADMINISTRATIVAS
t_list* list_particiones;
int tamanio_segmento;
int ultimo_ingreso;

t_memoria crear_memoria(int tamanio) {
	list_particiones = list_create();
	t_memoria segmento;
	tamanio_segmento = tamanio;
	ultimo_ingreso = 0;

	//inicio una particion inicial vacia que ocupe toodo el segmento y la agrego a la lista de particiones
	t_particion *part = malloc(sizeof(t_particion));
	part->id = '#';
	part->inicio = 0;
	part->tamanio = tamanio;
	part->libre = true;

	list_add(list_particiones, part);

	segmento = malloc(tamanio);

//	printf("SEGMENTO CREADO DE TAMANIO %d\n", tamanio_segmento);

	return segmento;
}

int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido) {

	t_particion *particion = malloc(sizeof(t_particion));
	t_particion *auxParticion;
	t_particion *part_useless;
	int validacion = buscarSegmentoLibre(segmento, tamanio);//devuelve una posicion en la lista, no la posicion inicial

	//validaciones de tamanio e id
	switch(validacion){
		case NO_ENTRA:
			return 0;
		case SUPERA_SEGMENTO:
			return -1;
	}
	if(repiteID(id)){
		return -1;
	}

	auxParticion = list_get(list_particiones, validacion);

	particion->inicio = auxParticion->inicio;
	particion->id = id;
	particion->tamanio = tamanio;
	particion->libre = false;
	particion->dato = (char*)(segmento + particion->inicio);


	//modifico los elementos de la lista de particiones para agregar la nueva particion

	//si el tamanio de la particion nueva es igual al de la vieja, la reemplazo
	if(tamanio == auxParticion->tamanio){
		part_useless = list_replace(list_particiones, ultimo_ingreso, particion);
	//si el tamanio es menor, agrego la particion nueva en esa posicion y modifico la vieja
	}else{
		auxParticion->inicio = (particion->inicio + particion->tamanio);
		auxParticion->libre = true;
		auxParticion->tamanio = (auxParticion->tamanio - particion->tamanio);
		auxParticion->id = '#';

		//agrego la nueva particion
		list_add_in_index(list_particiones, ultimo_ingreso, particion);

		//reemplazo la particion vieja libre con la nueva actualizada
		part_useless = list_replace(list_particiones, (ultimo_ingreso + 1), auxParticion);
	}

	//copio el contenido al segmento
	strcpy(((char*)(segmento+particion->inicio)), contenido);


	return 1;
}

int buscarSegmentoLibre(t_memoria segmento, int tamanio){

	int lenList = list_size(list_particiones);
	int x = ultimo_ingreso;
	int y = 0;
	t_particion *part;

	//validacion que la particion no sea mayor al segmento
	if(tamanio>tamanio_segmento){
//		printf("EL TAMANIO %d ES MAYOR AL TAMANIO_SEGMENTO %d\n", tamanio, tamanio_segmento);
		return SUPERA_SEGMENTO;
	}

	//busco espacio entre el ultimo_ingreso y el final de la lista de particiones
	while(x<lenList){
		part = list_get(list_particiones, x);
		if(part->libre){
			if(tamanio <= part->tamanio){
				ultimo_ingreso = x;
				return x;
			} else{
				if(analizarProximasParticiones(x, tamanio, part->tamanio)){
					ultimo_ingreso = x;
					return x;
				}
			}
		}
		x++;
	}

	//busco espacio entre el primer elemento de la lista de particiones y el ultimo_ingreso
	while(y < ultimo_ingreso){
		part = list_get(list_particiones, y);
		if(part->libre){
			if(tamanio <= part->tamanio){
				ultimo_ingreso = y;
				return y;
			} else{
				if(analizarProximasParticiones(y, tamanio, part->tamanio)){
					ultimo_ingreso = y;
					return y;
				}
			}
		}
		y++;
	}

	return NO_ENTRA;
}

int eliminar_particion(t_memoria segmento, char id) {
	int i = 0;
	int len = list_size(list_particiones);
	t_particion *aux;
	t_particion *part_useless;
	while(i<len){
		aux = list_get(list_particiones, i);
		if(aux->id == id){
			aux->libre = true;
			aux->id = '#';
			part_useless = list_replace(list_particiones, i, aux);
			return 1;
		} else{
			i++;
		}
	}
	return 0;
}

void liberar_memoria(t_memoria segmento) {
	list_destroy(list_particiones);
	free(segmento);
}

t_list* particiones(t_memoria segmento) {
	t_list* list_imprimir = list_create();
	list_add_all(list_imprimir, list_particiones);
	return list_imprimir;
}

bool analizarProximasParticiones(int x, int tamanio, int tamanio_anterior){
	int i = x + 1;
	int tamanio_disponible;
	t_particion *part;
	if(list_size(list_particiones) > i){
		part = list_get(list_particiones, i);
		tamanio_disponible = tamanio_anterior + part->tamanio;
		if(part->libre){
			if(tamanio <= (tamanio_disponible)){
				return true;
			} else{
				return analizarProximasParticiones(i, tamanio, tamanio_disponible);
			}
		} else{
			return false;
		}
	} else{
		return false;
	}


}


/*-------------FUNCIONES PRIVADAS-------------------*/

void imprimirParticiones(t_memoria segmento){
	int i = 0;
	t_particion *part;
	char *datos;
	int len = list_size(list_particiones);

	printf("el segmento tiene %d particiones ocupadas y %d libres\n", cantidad_particiones(), (list_size(list_particiones) - cantidad_particiones()));

	while(i < len){
		part = list_get(list_particiones, i);
		datos = malloc(sizeof(char)*part->tamanio);
		if(part->libre){
			printf("particion libre, empieza en %d, de tamanio %d\n",part->inicio, part->tamanio);
		}else{
			strcpy(datos, (string_substring(part->dato, 0, part->tamanio)));
			printf("la particion \'%c\' esta ocupada, empieza en %d, de tamanio %d y contiene %s\n", part->id, part->inicio, part->tamanio, datos);
		}
		free(datos);
		i++;
	}
}

bool repiteID(char id){
	int i = 0;
	t_particion *aux;
	while(i<list_size(list_particiones)){
		aux = list_get(list_particiones, i);
		if((!(aux->libre)) && (id == aux->id)){
			return true;
		} else{
			i++;
		}
	}
	return false;
}

void particion_destroy(t_particion* part){
	free(part->dato);
	free(part);
}

int cantidad_particiones(){
	int i = 0;
	int cont = 0;
	t_particion *aux;
	while(i<list_size(list_particiones)){
		aux = list_get(list_particiones, i);
		if(aux->libre == false){
			cont ++;
		}
		i++;
	}
	return cont;
}

void imprimirPrueba(t_memoria segmento){
	t_particion *aux;
	int h = 0;
	while(h < (list_size(list_particiones))){
		aux = list_get(list_particiones, h);
		printf("inicio: %d, tamanio: %d, libre: %d, id: \'%c\'\n", aux->inicio, aux->tamanio, aux->libre, aux->id);
		h++;
	}
}
