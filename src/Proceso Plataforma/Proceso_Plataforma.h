/*
 * Proceso_Plataforma.h
 *
 *  Created on: 11/04/2013
 *      Author: utnso
 */

#ifndef PROCESO_PLATAFORMA_H_
#define PROCESO_PLATAFORMA_H_

#define STANDBY 0

#define REQUESTDATANIVEL 01

#include <collections/list.h>
#include <collections/queue.h>
#include <collections/stack.h>
#include <log.h>
#include <pthread.h>
#include "string.h"
#include <Connections/Server.h>
#include <Connections/Client.h>
#include <Connections/Mensajes.h>
#include <Connections/EstructurasMensajes.h>
/*
typedef struct  b{
	char ID[3]; //es un string, para usar strcpy
	char IP[20];
	int PORT;
} Planificador;

typedef struct a {
	char ID[3]; //es un string, para usar strcpy
	char IP[20];
	int PORT;
	int FD;
	int NIVEL;
} Personaje;

typedef struct t_personaje {
	char nombre[30];
	char estado;
} Elemento_personaje;
*/
Elemento_personaje* personaje_crear(char* nombre);
void* initPanif(void* Nivel);



#endif /* PROCESO_PLATAFORMA_H_ */
