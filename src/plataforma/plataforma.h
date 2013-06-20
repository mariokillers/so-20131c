#ifndef PLATAFORMA_H_
#define PLATAFORMA_H_

#define STANDBY 0

#define REQUESTDATANIVEL 01
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/collections/stack.h>
#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>

#include <commons/Connections/Server.h>
#include <commons/Connections/Client.h>
#include <commons/Connections/Mensajes.h>
#include <commons/Connections/EstructurasMensajes.h>
/*
typedef struct  b{
	char ID[3]; //es un string, para usar strcpy
	char IP[20];
	int PORT;﻿﻿
} Planificador;
*/


typedef struct {
	char ID[20];
	Nivel dataNivel;
	Planificador dataPlanificador;
	t_list* personajes_en_nivel;
	t_queue* queue_listos;
	t_list* queues_bloq;
	Personaje* PersonajeEnMovimiento;

}GestorNivel;

typedef struct{
	char idRecurso;
	t_queue* queue;
} Queue_bloqueados;

/*
typedef struct t_personaje {
	char nombre[30];
	char estado;
} Elemento_personaje;
*/
//Elemento_personaje* personaje_crear(char* nombre);
void* Planif(void*);
void initOrq(void);
void* orq (void*);
void finalizarProceso();
void imprimirListos();
void imprimirBloqueados();
GestorNivel* findGestor_byid (char* );
GestorNivel* findGestor_byfd (int);
Queue_bloqueados* findBloqQueue_byidRecurso (t_list*, char);
Personaje* findUltimoEnLlegar (t_list*, char*);
Recursos asignarRecurso (char , Personaje* );
#endif /* PROCESO_PLATAFORMA_H_ */
