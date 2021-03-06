/*
 * Mensajes.h
 *
 *  Created on: 27/04/2013
 *      Author: utnso
 */

#ifndef MENSAJES_H_
#define MENSAJES_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <fcntl.h>
#include "../collections/queue.h"
#include "EstructurasMensajes.h"

#define BACKLOG 20
// Cuántas conexiones pendientes se mantienen en cola

#define MAXEVENTS 20
// Cuántas conexiones atiende y supervisa.


//Variables globales



//Estructuras Mensaje y Conexion
typedef struct {
	int instancia_epoll;			// Instancia epoll
	int sockfd, masterfd; 			// Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
	char flag_desconexiones;
	struct epoll_event event;		//
	struct epoll_event *events;
	struct sockaddr_in my_addr; 	// Información sobre mi dirección
} CCB;

typedef struct {
	char name [20];
	int fd;
} Conexion;

typedef struct {
	int from;			//fileDescriptor de la conexion
	char type;
	uint16_t lenght;
	void* data;
} Mensaje;


///Prototipo de funcion
int mensajes(t_queue* , CCB );
int mandarMensaje( int , char , uint16_t , void*);
void borrarMensaje(Mensaje*);
int obtenerData(void* , Mensaje*);
int make_socket_non_blocking (int);
void Cerrar_Conexion (int fd, CCB* miCON, t_queue* mensajes_queue);

#endif /* MENSAJES_H_ */
