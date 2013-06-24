/*
 * Mensajes.c
 *
 *  Created on: 27/04/2013
 *      Author: utnso
 */
#include "Mensajes.h"

/*
 * @NAME: mensajes
 * @DESC: Esta es la funcion que se va a usar para esperar mensajes.
 * 		  Recibe como parametro una cola de mensajes, y una lista de conexiones.
 * 		  Acepta automaticamente todas las conexiones nuevas, y devuelve la
 * 		  cantidad de mensajes que hay en la cola.
 */
int mensajes(t_queue* mensajesQueue, CCB myCOM){
	
	if(queue_size(mensajesQueue) != 0) return queue_size(mensajesQueue); else {
	
	int n, i; // n = cantidad de eventos que devuelve epoll, i = variable para recorrer los eventos

	//// ESPERO LAS NOVEDADES EN LOS SOCKETS QUE ESTOY OBSERVANDO
	n = epoll_wait (myCOM.instancia_epoll, myCOM.events, MAXEVENTS, 500);

	//// RECORRO LOS EVENTOS ATENDIENDO LAS NOVEDADES
	for (i = 0; i < n; i++)
	{
		//// SI EL EVENTO QUE ESTOY MIRANDO DIO ERROR O NO ESTA LISTO PARA SER LEIDO
		if ((myCOM.events[i].events & EPOLLERR) || (myCOM.events[i].events & EPOLLRDHUP) || (myCOM.events[i].events & EPOLLHUP) || (!(myCOM.events[i].events & EPOLLIN))) {
			fprintf (stderr, "epoll error\n");
			//CIERRO EL EVENTO
			Cerrar_Conexion(myCOM.events[i].data.fd);
			continue;
		}

		//// HAY NOVEDADES EN EL SOCKET MAESTRO (NUEVAS CONEXIONES)!!!
		else if (myCOM.masterfd == myCOM.events[i].data.fd) {

			////ACEPTO TODAS LAS INCOMING CONNECTIONS
			while (1) {
				struct sockaddr their_addr;
				socklen_t in_len;
				int infd;
				char hbuf[30], sbuf[30];
				in_len = sizeof their_addr;


				////ASIGNO EL NUEVO SOCKET DESCRIPTOR
				infd = accept (myCOM.masterfd, &their_addr, &in_len);


				////SI YA HABIA ACEPTADO TODAS O AL ACEPTAR UNA CONEXION ME DA ERROR
				if (infd == -1) {
					if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
						//YA ACEPTÉ TODAS LAS CONEXIONES NUEVAS, SALGO DEL BUCLE
						break;
					}
					else
					{
						//ERROR AL QUERER ACEPTAR, SALGO DEL BUCLE
						perror ("accept");
						break;
					}
				}

				////OBTENGO LOS DATOS DEL CLIENTE
				int s = getnameinfo (&their_addr, in_len,
						hbuf, sizeof hbuf,
						sbuf, sizeof sbuf,
						0);
				if (s == 0)
				{
					//printf("Accepted connection on descriptor %d "
					//		"(host=%s, port=%s)\n", infd, hbuf, sbuf);
				}
				/*
				//CREO LA NUEVA INSTANCIA CONNECTION TODO LIBERAR ESTRUCTURA MENSAJE
				Conexion* NuevaConexion;
				if((NuevaConexion = malloc(sizeof(Conexion))) == NULL){
					perror ("malloc");
					exit(1);
				}

							//ASIGNO LOS DATOS DEL MENSAJE
							strcpy((NuevaConexion->name), hbuf );
							NuevaConexion->fd = infd;

							list_add (conexionesList, NuevaConexion);
				 */
				//SETEO EL SOCKET COMO NO BLOQUEANTE
				if ((s = (make_socket_non_blocking (infd))) == -1) abort ();

				////ASOCIO EL FD DE LA NUEVA CONEXION
				myCOM.event.data.fd = infd;
				myCOM.event.events = EPOLLIN | EPOLLET;

				//AGREGO LA NUEVA CONEXION A LA INSTANCIA EPOLL
				if ((s = epoll_ctl (myCOM.instancia_epoll, EPOLL_CTL_ADD, infd, &(myCOM.event))) == -1){
					perror ("epoll_ctl");
					exit(1);
				}
			}//CIERRE DEL WHILE; NO ERAN CONEXIONES NUEVAS
			continue;
		}//NO ERA NI ERROR; NI NUEVA CONEXION; ES DATA

		////HAY DATOS EN ALGUNA CONEXION, TENGO QUE LEER TODO PORQUE ESTOY EN edge-trigger
		else {

			int done = 0;

			while (1){
				ssize_t count;
				char buf[512];
				ssize_t datos_restantes_en_buf;
				//LEO LOS DATOS
				count = read (myCOM.events[i].data.fd, buf, sizeof buf);

				//CHECKEO SI YA LEI TODOS O EL CLIENTE CERRO CONEXION
				if (count == -1)
				{
					//LEI TODOS
					if (errno != EAGAIN){
						perror ("read");
						done=1;
					}
					break;
				}
				else if (count == 0)
				{
					//EL CLIENTE CERRO CONEXION
					done=1;
					break;
				}
				
				datos_restantes_en_buf = count;
				//MIENTRAS HAYA MENSAJES EN EL BUFFER
				while(datos_restantes_en_buf!=0){

					//CREO LA NUEVA INSTANCIA MENSAJE TODO LIBERAR ESTRUCTURA MENSAJE
					Mensaje* NuevoMensaje;
					if((NuevoMensaje = malloc(sizeof(Mensaje))) == NULL){
						perror ("malloc");
						exit(1);
					}

					//ASIGNO LOS DATOS DEL MENSAJE
					NuevoMensaje->from = myCOM.events[i].data.fd;
					NuevoMensaje->type = buf[0];
					memcpy(&(NuevoMensaje->lenght),(void*)(&(buf[1])),2);

					//SOLICITO MEMORIA PARA LA DATA DEL MENSAJE TODO LIBERAR DATA
					if((NuevoMensaje->data = malloc(NuevoMensaje->lenght)) == NULL){
						perror ("malloc");
						exit(1);
					}

					//COPIO LA DATA AL ELEMENTO DEL MENSAJE
					memcpy(NuevoMensaje->data, (void*) buf+3, NuevoMensaje->lenght);

					queue_push (mensajesQueue, NuevoMensaje);

					//cambio la cantidad que me queda en buf
					datos_restantes_en_buf=datos_restantes_en_buf-3-(NuevoMensaje->lenght);
					memmove(buf,  (void*) (buf+3+(NuevoMensaje->lenght)), datos_restantes_en_buf);



				}//YA NO HAY MAS MENSAJES EN EL BUFFER
				if(datos_restantes_en_buf==0) break;

			}
			//SI TERMINE CIERRO CONEXION
			if (done){
				Cerrar_Conexion(myCOM.events[i].data.fd);
			}
		}
	}


	return queue_size(mensajesQueue);

}
}

/*
 * @NAME: mandarMensaje
 * @DESC: Envia un mensaje al descriptor que recibe como parametro. El resto son:
 * 		  type: codigo de mensaje, lenght: size of (variable que se adjunta),
 * 		  data: dirección de la variable a adjuntar.
 */
int mandarMensaje( int fd , char type, uint16_t lenght, void*data){
	char mensaje[lenght+3];
	mensaje[0]=type;
	memcpy(&mensaje[1],(void*)(&lenght),2);
	memcpy(&mensaje[3],(void*)data,lenght);


	return (send(fd , mensaje, lenght+3, 0));

}

/*
 * @NAME: borrarMensaje
 * @DESC: Libera la memoria reservada para un mensaje en particular, y su contenido.
 */
void borrarMensaje(Mensaje* miMensaje){
	free(miMensaje->data);
	free(miMensaje);

}

/*
 * @NAME: obtenerData
 * @DESC: Recibe como parametro un puntero a la variable que se quiera recuperar
 * 		  del dato adjunto en el mensaje que recibe como parametro.
 */
int obtenerData(void* destino, Mensaje* miMensaje){
	memcpy(destino, (void*) miMensaje->data, miMensaje->lenght);
	return 0;

}

///////////////////////////////PRIVATE////////////////////////////////


void Cerrar_Conexion (int fd){

	printf ("Closed connection on descriptor %d\n", fd);
	//CIERRO CONEXION
	close (fd);

}

int make_socket_non_blocking (int sfd)
{
	int flags, s;

	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror ("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);
	if (s == -1)
	{
		perror ("fcntl");
		return -1;
	}

	return 0;
}
