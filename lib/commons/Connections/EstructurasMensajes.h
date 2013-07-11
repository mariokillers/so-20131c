#ifndef ESTRUCTURAS_MENSAJES_H_
#define ESTRUCTURAS_MENSAJES_H_

enum MENSAJE {HANDSHAKE=1, REQUEST_DATA_NIVEL, REQUEST_POS_RECURSO, POSICION_RECURSO,
		REQUEST_RECURSO, CONFIRMAR_RECURSO, TERMINE_NIVEL, RECURSOS_LIBERADOS,
		REQUEST_INTERBLOQUEO, REQUEST_MOVIMIENTO, PERSONAJE_BLOQUEADO,
		TERMINE_TURNO, MORISTE_PERSONAJE, DATANIVEL, MOVIMIENTO_PERMITIDO,
		RECURSOS_REASIGNADOS, NOMBRE_VICTIMA, GANE, REASIGNACION_FINALIZADA, NODATANIVEL, CERRANDO_NIVEL, REINICIAR_NIVEL, DESCONEXION};

typedef struct  c{
	char ID[20]; //es un string, para usar strcpy
	char IP[20];
	int PORT;
	int FD;
} __attribute__ ((__packed__)) Nivel;

typedef struct  b{
	char ID[3]; //es un string, para usar strcpy
	char IP[20];
	int PORT;
	int FD;
}__attribute__ ((__packed__)) Planificador;

typedef struct a {
	char ID[3]; //es un string, para usar strcpy
	char IP[20];
	int PORT;
	int FD;
}__attribute__ ((__packed__)) Personaje;

typedef struct {
	int POS_X;
	int POS_Y;
}__attribute__ ((__packed__)) Posicion;

typedef struct {
	char IP[20];
	int PORT;
}__attribute__ ((__packed__)) Direccion;

typedef struct {
	Nivel miNivel;
	Planificador miPlanificador;
}__attribute__ ((__packed__)) Data_Nivel;


typedef struct {
	char idRecurso;
	int cant;
	char idPersonaje;
}__attribute__ ((__packed__)) Recursos;

Posicion Pos (int x, int y);
int obtenerPosX(Posicion pos);
int obtenerPosY(Posicion pos);
int tomarPuerto(char *direct);
char *tomarIP(char *direct);

#endif
