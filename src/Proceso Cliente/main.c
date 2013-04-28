/*
 * Main.c
 *
 *  Created on: 27/04/2013
 *      Author: utnso
 */
#include "Proceso Cliente.h"

int main(void){
	char Server[20];
	char* msg ="Hola como te va?";
	int sockfd;
	void *mipuntero;
	mipuntero=msg;
	//strcpy(Server,"localhost");
	printf("Ingrese IP del host\n");
	scanf("%s",Server);
	sockfd = connectServer(Server, 5000);
	printf("Listo para mandar\n");
	mandarMensaje(sockfd,'a', atoi("20") ,mipuntero);
	printf("Mensaje enviado\n");




	//CIERRO CONEXION
	close(sockfd);
	return 0;

}

