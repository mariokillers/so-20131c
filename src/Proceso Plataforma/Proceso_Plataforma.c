#include <stdio.h>
#include <stdlib.h>
#include "Proceso_Plataforma.h"

int main (){


t_list* estados_personajes;
estados_personajes = list_create();
Elemento_personaje mi_personaje;
printf("Ingrese el nombre de mi_personaje");
scanf("%c", &(mi_personaje.nombre));
list_add (estados_personajes, &mi_personaje);
printf("El tamaño de la lista es %d\n", list_size(estados_personajes) );

return 0;
}