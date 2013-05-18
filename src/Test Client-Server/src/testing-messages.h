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
void requestDataNivel();
void responsePosicionRecurso();

#endif /* TESTING_MESSAGES_H_ */
