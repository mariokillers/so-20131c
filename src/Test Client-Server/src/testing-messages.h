/*
 * testing-messages.h
 *
 *  Created on: 18/05/2013
 *  Author: utnso
 */

#ifndef TESTING_MESSAGES_H_
#define TESTING_MESSAGES_H_

#include <string.h>
#include <stdlib.h>
#include "collections/queue.h"
#include "Connections/Client.h"
#include "Connections/Server.h"
#include "Connections/Mensajes.h"
#include "Connections/EstructurasMensajes.h"

void procesoPersonaje();
void procesoNivel();
void procesoPlataforma();
void showClientMessageAndResponse();
void showServerMessage();

void setClientAndServer();
void conectarPersonaje();
void responseConectarPersonaje(Mensaje* mensaje);
void requestDataNivel();
void responseDataNivel(Mensaje* mensaje);
void requestProximoRecurso();
void responsePosicionProxRecurso(Mensaje* mensaje);
void realizarMovimiento();
void responseRealizarMovimiento(Mensaje* mensaje);
void quieroRecurso();
void responseQuieroRecurso(Mensaje* mensaje);
void bloquear();
void responseBloquear(Mensaje* mensaje);
void terminarTurno();
void responseTerminarTurno(Mensaje* mensaje);
void finDeNivel();
void responseFinDeNivel(Mensaje* mensaje);
void gane();
void responseGane(Mensaje* mensaje);
void conectarNivel();

#endif /* TESTING_MESSAGES_H_ */
