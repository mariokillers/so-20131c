/*
 * Proceso Cliente.c
 *
 *  Created on: 17/04/2013
 *      Author: utnso
 */
#include "Proceso Cliente.h"

int instancia_epoll;			// Instancia epoll
int sockfd, new_fd; 			// Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
struct epoll_event event;		//
struct epoll_event *events;

int main(void){
	char Server[20];
	char msg []="Mensaje 1";
	int sockfd;
	int numbytes=0;
	char buf[128];
	//strcpy(Server,"localhost");
	printf("Ingrese IP del host\n");
	scanf("%s",Server);
	sockfd = connectServer(Server);
	printf("Listo para mandar\n");
	send(sockfd, msg, 18, 0);
	printf("Mensaje enviado\n");

	//ESPERO RESPUESTA
	if ((numbytes=recv(sockfd, buf, sizeof(buf)-1, 0)) == -1) {
	perror("recv");
	exit(1);
	}

	//MUESTRO RESPUESTA
	buf[numbytes] = '\0';
	printf("Received: %s \n",buf);

	//CIERRO CONEXION
	close(sockfd);
	return 0;

}

int connectServer(char *Server){
	int sockfd; 					// Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
	struct sockaddr_in their_addr; 	// Información sobre mi dirección
	struct hostent *server; 		// Información sobre el server

	////OBTENER INFORMACIÓN DEL SERVER
		if ((server = gethostbyname(Server)) == NULL) { // obtener información de máquina
		perror("gethostbyname");
		exit(1);
		}

		////SETEO CONFIGURACIONES DE IP + PUERTO
		their_addr.sin_family = AF_INET;  // Ordenación de bytes de la máquina
		their_addr.sin_port = htons(MYPORT); // short, Ordenación de bytes de la	red
		their_addr.sin_addr = *((struct in_addr *)server->h_addr);
		memset(&(their_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura


		////PIDO EL SOCKET
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("socket");
			exit(1);
		}


	////INTENTO CONECTAR
		if (connect(sockfd, (struct sockaddr *)&their_addr,	sizeof(struct sockaddr)) == -1) {
			perror("connect");
			exit(1);
		}
		printf("Conectado con host \n");
	return sockfd;

}


void closeConnection (int fd){

	//CIERRO CONEXION
	close (fd);
	printf ("Desconectado del host\n");

}


