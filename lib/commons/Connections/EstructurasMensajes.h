#define HANDSHAKE 1
#define REQUEST_DATA_NIVEL 2
#define REQUEST_POS_RECURSO 3
#define POSICION_RECURSO 4
#define REQUEST_RECURSO 5
#define CONFIRMAR_RECURSO 6
#define TERMINE_NIVEL 7
#define RECURSOS_LIBERADOS 8
#define REQUEST_INTERBLOQUEO 10
#define REQUEST_MOVIMIENTO 11
#define PERSONAJE_BLOQUEADO 12
#define TERMINE_TURNO 13
#define MORISTE_PERSONAJE 14
#define DATANIVEL 16
#define MOVIMIENTO_PERMITIDO 17
#define RECURSOS_REASIGNADOS 18
#define NOMBRE_VICTIMA 18 //CHAR DE LA VICTIMA DE INTERBLOQUEO

typedef struct  c{
	char ID[3]; //es un string, para usar strcpy
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
}__attribute__ ((__packed__)) Recursos;

/*Recursos CantRecursos (int flores,int hongos,int monedas){
	Recursos aux;
	aux.cant_flor = flores;
	aux.cant_hongo = hongos;
	aux.cant_moneda = monedas;
	return(aux);

}*/

Posicion Pos (int x, int y){
	Posicion aux;
	aux.POS_X  = x;
	aux.POS_Y = y;
	return(aux);
}


int obtenerPosX(Posicion pos){
	int aux = pos.POS_X;
	return(aux);
}

int obtenerPosY(Posicion pos){
	int aux = pos.POS_Y;
	return(aux);
}

char* tomarPuerto(char* direct){
	char** direct_sep = string_split(direct, ":");
	int puerto = atoi(direct_sep[1]);
	return puerto;
}

char* tomarIP(char* direct){
	char** direct_sep = string_split(direct, ":");
	char *IP = direct_sep[0];
	return IP;
}

