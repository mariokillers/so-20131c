/*
 * funciones_read_archivo_configuracion.c
 *
 *  Created on: 18/04/2013
 *      Author: utnso
 */


#include "personaje_library.h"

/*NAME: read_personaje_archivo_configuracion
PARAM: char* path -> direccion del archivo de configuracion
RETURN: t_personaje * -> un personaje creado en base a un archivo de configuracion
DESC: instancia un t_config (struct de commons/config.h) tomando valores del archivo de configuracion y devuelve 
	un personaje creado en base a este t_config
	*/
t_personaje *read_personaje_archivo_configuracion(char* path){

	t_personaje *personaje;
	t_config * p;

	p = config_create(path);

	personaje = create_personaje(p);

	return personaje;
}

void imprimir_personaje(t_personaje *personaje){
	printf("%s\n%c\n", personaje->personaje_nombre, personaje->personaje_simbolo);

	imprimir_lista_niveles(personaje->personaje_niveles);

	printf("%d\n", personaje->personaje_vidas);

	printf("%s\n", personaje->personaje_orquestador);
}

/*NAME: create_personaje
PARAM: t_config *n -> una instancia de t_config con los valores de un archivo de ocnfiguracion
RETURN: t_personaje * -> el personaje creado
DESC: con las funciones de commons/config.h va tomando los valores del t_config dependiendo de la key pasada
	como parametro a cada función
	*/

t_personaje *create_personaje(t_config *p){
	t_personaje *personaje;
	char *aux = config_get_string_value(p, "simbolo");

	personaje = (t_personaje*)malloc(sizeof(t_personaje));

	personaje->personaje_nombre = config_get_string_value(p, "nombre");

	personaje->personaje_simbolo = aux[0];

	personaje->personaje_niveles = create_lista_niveles(personaje, p);

	personaje->personaje_vidas = config_get_int_value(p, "vidas");
	
	personaje->personaje_vidas_restantes = personaje->personaje_vidas;

	personaje->personaje_orquestador = config_get_string_value(p, "orquestador");

	return personaje;
}

/*NAME: create_personaje_nivel
PARAM: char *nivel, char **objetivos -> un nivel y un array de strings que contienen los objetivos
	de un nivel
RETURN: t_personaje_nivel -> devuelve una instancia de t_personaje_nivel con los valores tomados del t_config utilizando la key
pasada como argumento
DESC: crea una nueva instancia de t_personaje_nivel
	*/
t_personaje_nivel *create_personaje_nivel(char *nivel, char **objetivos){
	t_personaje_nivel *new = malloc(sizeof(t_personaje_nivel));
	t_list *list_objetivos = list_create();
	int i = 0;

	strcpy(new->personaje_nivel,nivel);

	while(objetivos[i] != NULL){

		add_list_nivel_objetivos(objetivos[i], list_objetivos);
		i++;
	}

	new->personaje_objetivos = list_objetivos;

	new->sig = NULL;

	return new;
}

void add_list_nivel_objetivos(char *objetivo, t_list *list_objetivos){

	char objetivo_char = objetivo[0];

	list_add(list_objetivos, create_nivel_objetivo(objetivo_char));
}

t_personaje_objetivo *create_nivel_objetivo(char objetivo){
	t_personaje_objetivo *new = malloc(sizeof(t_personaje_objetivo));

	new->objetivo = objetivo;

	new->tiene_objetivo = false;

	new->sig = NULL;

	return new;
}

void add_list_personaje_niveles(char **arr, char *buffer_nivel, t_list *list){

		list_add(list, create_personaje_nivel(buffer_nivel, arr));
}

void imprimir_lista_niveles(t_list *list){

	int i = 0;
	t_personaje_nivel *niv;
	t_personaje_objetivo *obj;

	while(i < (list_size(list))){
		niv = list_get(list, i);

		printf("\n%s\n", niv->personaje_nivel);

		int j = 0;
		while(j < (list_size(niv->personaje_objetivos))){
			obj = list_get(niv->personaje_objetivos,j);

			printf("%c\n", obj->objetivo);
			printf("%d\n", obj->tiene_objetivo);
			j++;
		}
		printf("\n");
		i++;
	}
}

void liberar_memoria_personaje(t_personaje *personaje){
	int i = 0;
	t_personaje_nivel *niv;

	while(i < (list_size(personaje->personaje_niveles))){
		niv = list_get(personaje->personaje_niveles, i);
		int j = 0;
		while(j < (list_size(niv->personaje_objetivos))){
			free(niv->personaje_objetivos);

			j++;
		}
		free(niv);
		i++;
	}
	list_clean(personaje->personaje_niveles);
	free(personaje);
}

t_list *create_lista_niveles(t_personaje *personaje, t_config *p){

	t_list *list = list_create();
	char **aux_niveles = config_get_array_value(p, "planDeNiveles");
	int i = 0;

	char *key_obj = malloc(sizeof(char)*((strlen((aux_niveles[i])))+(sizeof(char)*6)));

	while((aux_niveles)[i] != NULL){

		strcpy(key_obj, string_from_format("obj[%s]", (aux_niveles)[i])); //key almacenada en key_obj

		add_list_personaje_niveles(config_get_array_value(p, key_obj), (aux_niveles)[i], list);

		i++;
	}

	free(key_obj);

	return list;
}
