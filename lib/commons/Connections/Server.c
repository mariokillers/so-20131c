/*
 * Server.c
 *
 *  Created on: 27/04/2013
 *      Author: utnso
 */
#include "Server.h"
#define MAX_EVENTOS 200


/*
 * @NAME: initServer
 * @DESC: Inicializa un servidor en el puerto indicado como parametro. Devuelve 0
 */
CCB initServer(int MYPORT){
		CCB myServer;

		////PIDO EL SOCKET
		if ((myServer.masterfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("socket");
			exit(1);
		}

		////SETEO CONFIGURACIONES DE IP + PUERTO
		myServer.my_addr.sin_family = AF_INET; 		  // Ordenación de bytes de la máquina
		myServer.my_addr.sin_port = htons(MYPORT); 	  // short, Ordenación de bytes de la	red
		myServer.my_addr.sin_addr.s_addr = INADDR_ANY; // Rellenar con mi dirección IP
		memset(&(myServer.my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

		////ASIGNO AL SOCKET LAS CONFIGURACIONES DE IP + PUERTO
		if (bind(myServer.masterfd, (struct sockaddr *)&myServer.my_addr, sizeof(struct sockaddr)) == -1) {
			perror("bind");
			exit(1);
		}

		////HAGO EL SOCKET NO BLOQUEANTE

		  if ((make_socket_non_blocking (myServer.masterfd)) == -1)
		    abort ();

		////LISTEN (BACKLOG => MAX CANTIDAD DE CLIENTES EN COLA)
		if (listen(myServer.masterfd, BACKLOG) == -1) {
			perror("listen");
			exit(1);
		}

		////CREO LA INSTANCIA DE EPOLL
		if ((myServer.instancia_epoll = epoll_create1 (0)) == -1) {
			perror ("epoll_create");
			exit(1);
		}

		////ASOCIO LOS FD DE LA CONEXION MAESTRA
		myServer.event.data.fd = myServer.masterfd;
		myServer.event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;

		////RESERVO ESPACIO PARA LAS CONEXIONES
		myServer.events = calloc (MAXEVENTS, sizeof myServer.event);

		////ASIGNO EL CCONTROL DE EPOLL
		if ((epoll_ctl (myServer.instancia_epoll, EPOLL_CTL_ADD, myServer.masterfd, &(myServer.event))) == -1) {
		    perror ("epoll_ctl");
		    exit(1);
		}


		return myServer;
}

//////////////////////////PRIVATE///////////////////////////




