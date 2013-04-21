/*
 * Proceso Cliente.h
 *
 *  Created on: 17/04/2013
 *      Author: utnso
 */

#ifndef PROCESO_CLIENTE_H_
#define PROCESO_CLIENTE_H_


#include <stdio.h>
#include <stdlib.h>
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

#define MYPORT 3490
// Puerto al que conectarán los usuarios

#define BACKLOG 20
// Cuántas conexiones pendientes se mantienen en cola

#define MAXEVENTS 20
// Cuántas conexiones atiende y supervisa.

typedef struct _Connection {
	char client [20];
	int client_FD;
	struct sockaddr client_addr;
	socklen_t in_len;
} Connection;

typedef struct _Mensaje {
	char from [20];
	char data[128];
} Mensaje;


////PROTOTIPOS DE FUNCIONES
int connectServer(char *);
void closeConnection (int);


#endif /* PROCESO_CLIENTE_H_ */
