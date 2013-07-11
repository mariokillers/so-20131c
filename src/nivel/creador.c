#include "nivel.h"
#include <string.h>

/*NAME: read_nivel_a rchivo_configuracion
PARAM: char* path -> direccion del archivo de configuracion
RETURN: t_nivel * -> un nivel creado en base a un archivo de configuracion
DESC: instancia un t_config (struct de commons/config.h) tomando valores del archivo de configuracion y devuelve 
	un nivel creado en base a este t_config
	*/

t_nivel *read_nivel_archivo_configuracion(char* path){

	t_nivel *nivel;
	t_config * n;

	n = config_create(path);
	if (n == NULL) {
		return NULL;
	}

	nivel = create_nivel(n);

	return nivel;
}

/*NAME: create_nivel
PARAM: t_config *n -> una instancia de t_config con los valores de un archivo de ocnfiguracion
RETURN: t_nivel * -> el nivel creado
DESC: con las funciones de commons/config.h va tomando los valores del t_config dependiendo de la key pasada
	como parametro a cada función
	*/

t_nivel *create_nivel(t_config *n){

	t_nivel *nivel;
	nivel = (t_nivel*)malloc(sizeof(t_nivel));
	nivel->orquestador = (Direccion*) malloc(sizeof(Direccion));
	nivel->miDireccion = (Direccion*)malloc(sizeof(Direccion));

	nivel->nivel_items = create_lista_cajas(n);

	strcpy(nivel->orquestador->IP , tomarIP(config_get_string_value(n, "orquestador")));

	strcpy(nivel->miDireccion->IP , tomarIP(config_get_string_value(n, "miDireccion")));

	nivel->nombre = config_get_string_value(n, "Nombre");

	nivel->orquestador->PORT = tomarPuerto(config_get_string_value(n, "orquestador"));

	nivel->miDireccion->PORT = tomarPuerto(config_get_string_value(n, "miDireccion"));

	nivel->tiempo_deadlcok = config_get_double_value(n, "tiempoChequeoDeadlock");

	nivel->recovery = config_get_int_value(n, "recovery");

	return nivel;

}

/*NAME: create_lista_cajas
PARAM: t_config *n -> una instancia de t_config con los valores de un archivo de configuracion
RETURN: ITEM_NIVEL * -> una instancia de ITEM_NIVEL * (so-nivel-gui-library-master/nivel-gui/nivel.h) como una lista nueva
DESC: crea una lista de ITEM_NIVEL con sus respectivos atributos
	*/

ITEM_NIVEL *create_lista_cajas(t_config *n){

	ITEM_NIVEL *ListaItems = NULL;

	char buffer_caja[8];
	char buffer_num[5];
	char buffer_caja_num[8];
	int i = 1;

	strcpy(buffer_caja, "caja");
	sprintf(buffer_num, "%d", i);
	strcpy(buffer_caja_num, (strcat(buffer_caja, buffer_num)));

	while(config_has_property(n, buffer_caja_num)){

		ListItems_add_caja(n, buffer_caja_num, &ListaItems);

		i++;

		strcpy(buffer_num, "");
		strcpy(buffer_caja, "");
		strcpy(buffer_caja_num, "");

		strcpy(buffer_caja, "caja");
		sprintf(buffer_num, "%d", i);
		strcpy(buffer_caja_num, (strcat(buffer_caja, buffer_num)));
	}

	return ListaItems;
}

/*NAME: create_caja
PARAM: t_config *n, char *buffer_caja_num -> una instancia de t_config y el key de una caja ("caja1", "caja2", etc..)
RETURN: ITEM_NIVEL * -> devuelve una instancia de ITEM_NIVEL * con los valores tomados del t_config utilizando la key
pasada como argumento
DESC: crea una nueva instancia de ITEM_NIVEL * utilizando las funciones de commons/config.h para tomar los valores
	de t_config *n.
EXPLICACION: el arch viene en formato "x,y,z,w", para poder tomar cada valor por separado, hay que convertirlo
	a formato ["x","y","z","w"], utilizando las funciones de commons/string.h  string_from_format\2 y
	string_get_string_as_array\1
	*/

void ListItems_add_caja(t_config *n, char *buffer_caja_num, ITEM_NIVEL **list){
	ITEM_NIVEL *new = malloc(sizeof(ITEM_NIVEL));
	
	char* aux_string = string_from_format("[%s]", config_get_string_value(n, buffer_caja_num));

	char **aux = string_get_string_as_array(aux_string);

	new->id = aux[1][0];

	new->quantity = atoi(aux[2]);
	
	new->item_type = RECURSO_ITEM_TYPE;

	new->posx = atoi(aux[4]);

	new->posy = atoi(aux[3]);

	new->next = *list;
	
	*list = new;
}
