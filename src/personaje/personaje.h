#ifndef PERSONAJE_H_
#define PERSONAJE_H_

enum ESTADO {STANDBY, WAIT_DATA_LEVEL, WAIT_POS_REC, WAIT_REC, NUEVO_NIVEL, WIN};

/*---------------------INCLUDES DEL TESTER, COMENTAR ANTES DE HACER PUSH--------------------------------*/
/*
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "EstructurasMensajes.h"
#include "Client.h"
#include "Server.h"
#include "Mensajes.h"
#include "commons/config.h"
#include "commons/string.h"
#include "commons/collections/list.h"
*/
/*--------------------------------------------------------------------------------------------*/


#include <commons/Connections/Client.h>
#include <commons/Connections/Mensajes.h>
#include <commons/Connections/EstructurasMensajes.h>
#include <commons/log.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>


/*define el tipo t_personaje_objetivo, que representa un objetivo de los que tiene que conseguir el personaje
en un nivel y si lo tiene o no
	*/
typedef struct t_personaje_objetivo{
	char objetivo;
	bool tieneObjetivo;
	struct t_personaje_objetivo *sig;
} t_personaje_objetivo;

/*define el tipo t_personaje_nivel, que representa un nivel que tiene que ganar un personaje y sus objetivos
	*/
typedef struct t_personaje_nivel{
	char *nivel;
	t_list *objetivos;
	bool terminoNivel;
	struct t_personaje_nivel *sig;
} t_personaje_nivel;

/*define t_personaje, que representa un personaje creado en base a un archivo de configuracion dado
y su estructura de datos
	*/
typedef struct t_personaje {
	char *nombre;
	char simbolo;
	t_list *niveles;
	int vidas;
	int vidasRestantes;
	Direccion *orquestador;
	Posicion *posActual;
} t_personaje;

t_personaje *read_personaje_archivo_configuracion(char* path);
t_personaje *create_personaje(t_config *p);
t_personaje_nivel *create_personaje_nivel(char *nivel, char **objetivos);
void add_list_personaje_niveles(char **arr, char *buffer_nivel, t_list *list);
t_list *create_lista_niveles(t_personaje *personaje, t_config *p);
t_personaje_objetivo *create_nivel_objetivo(char objetivo_char);
void add_list_nivel_objetivos(char *objetivo, t_list *list_objetivos);

int _is_next_level(t_personaje_nivel *p);
char *proximoNivel(t_list *niveles);
char *transformNivel_to_send(char *nivel, char **miNivAux);
int _is_next_obj(t_personaje_objetivo *o);
char proximoRecurso(t_list *niveles);
Posicion *proximaPosicion();
void reiniciarNivel(t_list *niveles);
void analizarRecurso();
bool recursoAlcanzado(Posicion *pos1, Posicion *pos2);
void agregarRecurso(t_list *niveles);
bool nivelTerminado(t_list *niveles, char *nivActual);
bool ganado(t_list *niveles);
Posicion *realizarMovimiento();
void morir();
void rutinaSignal(int n);
int conectarPlanificador(char *IP, int PORT);
int conectarNivel(char *IP, int PORT);
Personaje *hacerHandshake();
void inicializarPersonaje();
void solicitarDataNivel();

void imprimirObjetivos(t_list *niveles);


#endif /* PROCESOPERSONAJE_H_ */
