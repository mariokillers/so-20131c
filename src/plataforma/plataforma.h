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
#include <commons/config.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/Connections/Server.h>
#include <commons/Connections/Client.h>
#include <commons/Connections/Mensajes.h>
#include <commons/Connections/EstructurasMensajes.h>

t_list* Gestores;
t_log* Logger;
int quantum_inicial;
int quantum_delay;
t_list* personajes_jugando;
pthread_t orquestador;
pthread_t quantum_monitor;
pthread_mutex_t mutex_plataforma;
t_config *plataforma_config;
char *mi_ip;
char *path_koopa_lst;
char *path_archivo_config;



typedef struct {
	char ID[20];
	Nivel dataNivel;
	Planificador dataPlanificador;
	t_list* personajes_en_nivel;
	t_queue* queue_listos;
	t_list* queues_bloq;
	Personaje* PersonajeEnMovimiento;
	char turno_entregado;
	int quantum;
	pthread_mutex_t * miMutex;

}GestorNivel;

typedef struct{
	char idRecurso;
	t_queue* queue;
} Queue_bloqueados;


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
Personaje* removePersonaje_byid (t_list* personajes_en_nivel, Personaje* miPersonaje);
Personaje* removePersonaje_byfd (t_list* personajes_en_nivel, int fd);
Personaje* findPersonaje_byid (t_list* personajes_en_nivel, char* id);
void removePersonaje_fromBloq (t_list* bloqueados, Personaje* miPersonaje);
void entregarTurno (GestorNivel* miGestor);
Personaje* findUltimoEnLlegar (t_list*, char*);
void asignarDatos (Recursos* Aux, char recurso, Personaje* miPersonaje);
void leerArchivoConfig(void);
GestorNivel* removeGestor_byid (char* nivel);
#endif /* PROCESO_PLATAFORMA_H_ */
