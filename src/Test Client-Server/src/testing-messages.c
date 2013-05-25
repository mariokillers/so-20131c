#include <stdio.h>
#include "testing-messages.h"
#include <string.h>

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
		printf("9. Gane \n");
		printf("0. Volver al menu anterior\n");

		scanf("%d", &option);

		switch (option) {
			case 1:
				conectarPersonaje();
				break;
			case 2:
				requestDataNivel();
				break;
			case 3:
				requestProximoRecurso();
				break;
			case 4:
				realizarMovimiento();
				break;
			case 5:
				quieroRecurso();
				break;
			case 6:
				bloquear();
				break;
			case 7:
				terminarTurno();
				break;
			case 8:
				finDeNivel();
				break;
			case 9:
				gane();
				break;
		}
	}
}

void procesoNivel() {
	int option;
	while (option != 0) {
		printf("Seleccione un mensaje: \n");
		printf("1. Enviar victimas\n");
		printf("2. Liberar recursos\n");
		printf("3. Conectar\n");
		printf("0. Volver al menu anterior\n");

		scanf("%d", &option);

		switch (option) {
			case 1:
				break;
			case 2:
				break;
			case 3:
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
	while (!mensajes(mensajes_Server, Server));

		miMensaje = queue_pop(mensajes_Server);
		switch (miMensaje->type) {
			case HANDSHAKE:
				responseConectarPersonaje(miMensaje);
				break;
			case REQUEST_DATA_NIVEL:
				responseDataNivel(miMensaje);
				break;
			case REQUEST_POS_RECURSO:
				responsePosicionProxRecurso(miMensaje);
				break;
			case REQUEST_MOVIMIENTO:
				responseRealizarMovimiento(miMensaje);
				break;
			case REQUEST_RECURSO:
				responseQuieroRecurso(miMensaje);
				break;
			case PERSONAJE_BLOQUEADO:
				responseBloquear(miMensaje);
				break;
			case TERMINE_TURNO:
				responseTerminarTurno(miMensaje);
				break;
			case TERMINE_NIVEL:
				responseFinDeNivel(miMensaje);
				break;
			case GANE:
				responseGane(miMensaje);
				break;
		}
		borrarMensaje(miMensaje);

}

void showServerMessage() {
	Mensaje* mensaje;
	Posicion* pos;
	Data_Nivel* dataNivel;
	while (!mensajes(mensajes_Client, Client));
		mensaje = queue_pop(mensajes_Client);
		switch (mensaje->type) {
			case POSICION_RECURSO:
				pos = (Posicion*)mensaje->data;
				printf("Llego a Cliente el mensaje del proceso Nivel con la posicion (%d,%d)\n", pos->POS_X, pos->POS_Y);
				break;
			case DATANIVEL:
				dataNivel = (Data_Nivel*)mensaje->data;
				printf("Llego a Cliente el mensaje con la data del nivel. IDNivel:'%s', IDPlanificador: '%s'\n", dataNivel->miNivel.ID, dataNivel->miPlanificador.ID);
				break;
			case CONFIRMAR_RECURSO:
				printf("Llego a Cliente el mensaje confirmando recurso \n");
				break;
		}
		borrarMensaje(mensaje);
}

void setClientAndServer() {
	printf("Ingrese puerto: \n");
	int puerto;
	scanf("%d", &puerto);
	Server = initServer(puerto);
	Client = connectServer("localhost", puerto);
}

void conectarPersonaje() {
	setClientAndServer();
	printf("Enviando mensaje de Personaje para que se conecte...\n");
	Personaje personaje;
	strcpy(personaje.ID, "IDT");
	strcpy(personaje.IP, "localhost");
	personaje.PORT = 5000;
	personaje.FD = 0;
	mandarMensaje(Client.sockfd, HANDSHAKE, sizeof(personaje), &personaje);
	showClientMessageAndResponse();
}

void responseConectarPersonaje(Mensaje* mensaje) {
	Personaje* personaje = (Personaje*)mensaje->data;
	printf("LLego a Server el mensaje estableciendo conexion del proceso personaje '%s' \n", personaje->ID);
	close(Client.sockfd);
}

void requestDataNivel() {
	setClientAndServer();
	printf("Enviando mensaje de Personaje pidiendo informacion del nivel...\n");
	mandarMensaje(Client.sockfd, REQUEST_DATA_NIVEL, 0, NULL);
	showClientMessageAndResponse();
}

void responseDataNivel(Mensaje* mensaje) {
	printf("Llego a Server el mensaje del proceso Personaje pidiendo informacion del nivel\n");

	Nivel nivel;
	nivel.FD = 0;
	nivel.PORT = 0;
	strcpy(nivel.IP, "localhost");
	strcpy(nivel.ID, "IDT");

	Planificador planificador;
	planificador.FD = 0;
	planificador.PORT = 0;
	strcpy(planificador.IP, "localhost");
	strcpy(planificador.ID, "IDT");

	Data_Nivel dataNivel;
	dataNivel.miNivel = nivel;
	dataNivel.miPlanificador = planificador;

	printf("Enviando respuesta con la data del nivel \n");
	mandarMensaje(mensaje->from, DATANIVEL, sizeof(dataNivel), &dataNivel);
	showServerMessage();

	close(Client.sockfd);
}

void requestProximoRecurso() {
	setClientAndServer();
	printf("Enviando mensaje de Personaje pidiendo posicion del proximo recurso \n");
	char recurso = 'M';
	mandarMensaje(Client.sockfd, REQUEST_POS_RECURSO, sizeof(recurso), &recurso);
	showClientMessageAndResponse();
}

void responsePosicionProxRecurso(Mensaje* mensaje) {
	char* recurso = (char*)mensaje->data;
	printf("Llego a Server el mensaje del proceso Personaje pidiendo la posicion del proximo recurso de '%s' \n", recurso);
	Posicion pos = Pos(2,3);
	printf("Enviando respuesta del Proceso Nivel con la posicion (2,3) \n");
	mandarMensaje(mensaje->from, POSICION_RECURSO, sizeof(pos), &pos);
	showServerMessage();
	close(Client.sockfd);
}

void realizarMovimiento() {
	setClientAndServer();
	printf("Enviando mensaje para realizar movimiento con la posicion (3,4)\n");
	Posicion pos = Pos(3,4);
	mandarMensaje(Client.sockfd, REQUEST_MOVIMIENTO, sizeof(pos), &pos);
	showClientMessageAndResponse();
}

void responseRealizarMovimiento(Mensaje* mensaje) {
	Posicion* posicion = (Posicion*)mensaje->data;
	printf("Llegó al server el mensaje del proceso Personaje para realizar un movimiento a (%d,%d) \n", posicion->POS_X, posicion->POS_Y);
	close(Client.sockfd);
}

void quieroRecurso() {
	setClientAndServer();
	printf("Enviando mensaje de Personaje para obtener un recurso. \n");
	char recurso = 'M';
	mandarMensaje(Client.sockfd, REQUEST_RECURSO, sizeof(recurso), &recurso);
	showClientMessageAndResponse();
}

void responseQuieroRecurso(Mensaje* mensaje) {
	char* recurso = (char*)mensaje->data;
	printf("Llegó al server el mensaje del proceso Personaje pidiendo el recurso '%s'. \n", recurso);
	int teLoDi = 1;
	printf("Enviando respuesta del Proceso Nivel confirmando recurso (valor: %d) \n", teLoDi);
	mandarMensaje(mensaje->from, CONFIRMAR_RECURSO, sizeof(teLoDi), &teLoDi);
	showServerMessage();
	close(Client.sockfd);
}

void bloquear() {
	setClientAndServer();
	printf("Enviando mensaje de Personaje para bloquear \n");
	char recurso = 'M';
	mandarMensaje(Client.sockfd, PERSONAJE_BLOQUEADO, sizeof(recurso), &recurso);
	showClientMessageAndResponse();
}

void responseBloquear(Mensaje* mensaje) {
	char* recurso = (char*)mensaje->data;
	printf("Llegó al server el mensaje del proceso Personaje para bloquear el recurso '%s' \n", recurso);
	close(Client.sockfd);
}

void terminarTurno() {
	setClientAndServer();
	printf("Enviando mensaje de Personaje informando que termino el turno \n");
	mandarMensaje(Client.sockfd, TERMINE_TURNO, 0, NULL);
	showClientMessageAndResponse();
}

void responseTerminarTurno(Mensaje* mensaje) {
	printf("Llegó al server el mensaje del proceso Personaje informando que termino el turno \n");
	close(Client.sockfd);
}

void finDeNivel() {
	setClientAndServer();
	printf("Enviando mensaje de Personaje informando fin de nivel \n");
	mandarMensaje(Client.sockfd, TERMINE_NIVEL, 0, NULL);
	showClientMessageAndResponse();
}

void responseFinDeNivel(Mensaje* mensaje) {
	printf("Llegó al server el mensaje del proceso Personaje informando fin de nivel. \n");
	close(Client.sockfd);
}

void gane() {
	setClientAndServer();
	printf("Enviando mensaje de Personaje informando que gano \n");
	char personaje = 'M';
	mandarMensaje(Client.sockfd, GANE, sizeof(personaje), &personaje);
	showClientMessageAndResponse();
}

void responseGane(Mensaje* mensaje) {
	char* personaje = (char*)mensaje->data;
	printf("LLego a Server el mensaje de Cliente diciendo que el personaje '%s' gano \n", personaje);
	close(Client.sockfd);
}
