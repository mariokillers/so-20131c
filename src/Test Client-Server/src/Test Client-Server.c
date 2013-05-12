/*
 ============================================================================
 Name        : Test.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <Connections/Client.h>
#include <Connections/Server.h>
#include <Connections/Mensajes.h>
#include <collections/queue.h>


int main(void) {
	//INICIALIZO LA COLA DE MENSAJES
	CCB Server;
	CCB Client;
	t_queue* mensajes_Client;
	t_queue* mensajes_Server;
	Mensaje* miMensaje;
	mensajes_Server = queue_create();
	mensajes_Client = queue_create();

	//INICIALIZO EL SERVIDOR EN EL PUERTO 5000
	Server = initServer(5000);

	//INICIALIZO EL CLIENTE EN EL SOCKETCLIENTE
	Client = connectServer("localhost",5000);

	//MANDO MENSAJE DEL CLIENTE AL SERVIDOR
	mandarMensaje(Client.sockfd,'a',sizeof("Mensaje de Cliente a Server\n"),"Mensaje de Cliente a Server\n");

	//LEVANTO EL MENSAJE DEL CLIENTE
	while(!mensajes(mensajes_Server, Server));


				miMensaje = queue_pop(mensajes_Server);
				char*msg = (char*) miMensaje->data;
				printf("%s",msg);
				fflush(stdout);

				//ENVIO UNA RESPUESTA DEL SERVER AL CLIENTE
				mandarMensaje(miMensaje->from,'b',sizeof("Mensaje de Server a Cliente\n"),"Mensaje de Server a Cliente\n");

				//BORRO EL MENSAJE
				borrarMensaje(miMensaje);


	//LEVANTO EL MENSAJE DEL SERVIDOR (LLEGAN A LA MISMA COLA, QUE ES UNICA DEL PROCESO)
	while(!mensajes(mensajes_Client, Client));


				miMensaje = queue_pop(mensajes_Client);
				msg = (char*) miMensaje->data;
				printf("%s",msg);
				fflush(stdout);


				//BORRO EL MENSAJE
				borrarMensaje(miMensaje);



	//CIERRO EL SOCKET DEL CLIENTE
	close(Client.sockfd);

	return EXIT_SUCCESS;
}
