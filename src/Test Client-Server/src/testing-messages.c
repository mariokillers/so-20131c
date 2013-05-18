#include <stdio.h>
#include "testing-messages.h"

CCB Server;
CCB Client;
t_queue* mensajes_Client;
t_queue* mensajes_Server;

int main() {
	mensajes_Client = queue_create();
	mensajes_Server = queue_create();
	int option;
	while (option != 0) {
		printf("Seleccione un proceso: \n");
		printf("1. Personaje\n");
		printf("2. Nivel\n");
		printf("3. Plataforma\n");
		printf("0. Salir\n");

		scanf("%d", &option);

		switch (option) {
			case 1:
				procesoPersonaje();
				break;
			case 2:
				procesoNivel();
				break;
			case 3:
				procesoPlataforma();
				break;
		}
	}
	return 0;
}

void procesoPersonaje() {
	int option;
	while (option != 0) {

		printf("Seleccione un mensaje: \n");
		printf("1. Conectar\n");
		printf("2. Pedir Info Nivel\n");
		printf("3. Posición próximo recurso\n");
		printf("4. Realizar movimiento\n");
		printf("5. Quiero recurso\n");
		printf("6. Bloqueo (Avisa tambien que termino el turno)\n");
		printf("7. Termino turno (solo si termino el turno OK)\n");
		printf("8. Fin de nivel (mandar tanto a nivel como a planificador)\n");
		printf("9. Mori (no existe, manda termine nivel)\n");
		printf("10. Reiniciar nivel (conecta con el nivel que necesite)\n");
		printf("11. Gane \n");
		printf("0. Volver al menu anterior\n");

		scanf("%d", &option);

		Server = initServer(5000);
		Client = connectServer("localhost",5000);

		switch (option) {
			case 1:
				break;
			case 2:
				requestDataNivel();
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				break;
			case 6:
				break;
			case 7:
				break;
			case 8:
				break;
			case 9:
				break;
			case 10:
				break;
			case 11:
				break;
		}

		close(Client.sockfd);
	}
}

void procesoNivel() {
	int option;
	while (option != 0) {
		printf("Seleccione un mensaje: \n");
		printf("1. Entregar posición recurso\n");
		printf("2. Confirmar recurso\n");
		printf("3. Enviar victimas\n");
		printf("4. Liberar recursos\n");
		printf("5. Conectar\n");
		printf("0. Volver al menu anterior\n");

		scanf("%d", &option);

		switch (option) {
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				break;
		}
	}
}

void procesoPlataforma() {
	int option;
	while (option != 0) {
		printf("Seleccione un mensaje: \n");
		printf("1. Enviar dirección (ip+puerto)\n");
		printf("2. Movimiento permitido\n");
		printf("3. Matar personaje\n");
		printf("4. Notificar victima (el personaje le informa al nivel que termino)\n");
		printf("5. Recursos reasignados \n");
		printf("0. Volver al menu anterior\n");

		scanf("%d", &option);

		switch (option) {
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				break;
		}
	}
}

void showClientMessageAndResponse() {
	Mensaje* miMensaje;
	while (!mensajes(mensajes_Server, Server)) {
		miMensaje = queue_pop(mensajes_Server);
		switch (miMensaje->type) {
			case REQUEST_DATA_NIVEL:
				responsePosicionRecurso(miMensaje);
				break;
		}
		borrarMensaje(miMensaje);
	}
}

void showServerMessage() {
	Mensaje* mensaje;
	Posicion* pos;
	while (!mensajes(mensajes_Client, Client)) {
		mensaje = queue_pop(mensajes_Client);
		switch (mensaje->type) {
			case POSICION_RECURSO:
				pos = (Posicion*)mensaje->data;
				printf("Llego a Cliente el mensaje del proceso Nivel con la posicion (%d,%d)\n", pos->POS_X, pos->POS_Y);
				break;
		}
		borrarMensaje(mensaje);
	}
}

void requestDataNivel() {
	printf("Enviando mensaje de Personaje a Nivel pidiendo la posicion del proximo recurso...\n");
	mandarMensaje(Client.sockfd, REQUEST_DATA_NIVEL, 0, NULL);
	showClientMessageAndResponse();
}

void responsePosicionRecurso(Mensaje* mensaje) {
	printf("Llego a Server el mensaje del proceso Personaje pidiendo al nivel la posicion\n");
	Posicion pos = Pos(2, 3);
	printf("Enviando respuesta del Proceso Nivel con la posicion (2,3)");
	mandarMensaje(mensaje->from, POSICION_RECURSO, sizeof(pos), &pos);
	showServerMessage();
}
