#include "Proceso Server.h"

int instancia_epoll;			// Instancia epoll
int sockfd, new_fd; 			// Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
struct epoll_event event;		//
struct epoll_event *events;

int main(void){
	int instancia_epoll;			// Instancia epoll
	int sockfd; 			// Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
	struct sockaddr_in my_addr; 	// Información sobre mi dirección




	////PIDO EL SOCKET
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	////SETEO CONFIGURACIONES DE IP + PUERTO
	my_addr.sin_family = AF_INET;  // Ordenación de bytes de la máquina
	my_addr.sin_port = htons(MYPORT); // short, Ordenación de bytes de la	red
	my_addr.sin_addr.s_addr = INADDR_ANY; // Rellenar con mi dirección IP
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

	////ASIGNO AL SOCKET LAS CONFIGURACIONES DE IP + PUERTO
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	////HAGO EL SOCKET NO BLOQUEANTE

	  if ((make_socket_non_blocking (sockfd)) == -1)
	    abort ();

	////LISTEN (BACKLOG => MAX CANTIDAD DE CLIENTES EN COLA)
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	////CREO LA INSTANCIA DE EPOLL
	if ((instancia_epoll = epoll_create1 (0)) == -1) {
		perror ("epoll_create");
		exit(1);
	}

	////ASOCIO LOS FD DE LA CONEXION MAESTRA
	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;

	////ASIGNO EL CCONTROL DE EPOLL
	if ((epoll_ctl (instancia_epoll, EPOLL_CTL_ADD, sockfd, &event)) == -1) {
	    perror ("epoll_ctl");
	    exit(1);
	}

	////RESERVO ESPACIO PARA LAS CONEXIONES
	events = calloc (MAXEVENTS, sizeof event);

	////ESPERO MENSAJES O CONEXIONES
	while(1){
		int n, i; // n = cantidad de eventos que devuelve epoll, i = variable para recorrer los eventos


			//// ESPERO LAS NOVEDADES EN LOS SOCKETS QUE ESTOY OBSERVANDO
			n = epoll_wait (instancia_epoll, events, MAXEVENTS, -1);

			//// RECORRO LOS EVENTOS ATENDIENDO LAS NOVEDADES
			for (i = 0; i < n; i++)
			{
				//// SI EL EVENTO QUE ESTOY MIRANDO DIO ERROR O NO ESTA LISTO PARA SER LEIDO
				if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
				          fprintf (stderr, "epoll error\n");
				          //CIERRO EL EVENTO
				          Cerrar_Conexion(events[i].data.fd);
					      continue;
				}

				//// HAY NOVEDADES EN EL SOCKET MAESTRO (NUEVAS CONEXIONES)!!!
				else if (sockfd == events[i].data.fd) {

					////ACEPTO TODAS LAS INCOMING CONNECTIONS
					while (1) {
						struct sockaddr their_addr;
				        socklen_t in_len;
				        int infd;
				        char hbuf[30], sbuf[30];
		                in_len = sizeof their_addr;

		                ////ASIGNO EL NUEVO SOCKET DESCRIPTOR
		                infd = accept (sockfd, &their_addr, &in_len);


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
		                	printf("Accepted connection on descriptor %d "
		                			"(host=%s, port=%s)\n", infd, hbuf, sbuf);
		                }

		                //SETEO EL SOCKET COMO NO BLOQUEANTE
						if (make_socket_non_blocking (infd) == -1) abort ();

						////ASOCIO EL FD DE LA NUEVA CONEXION
		                event.data.fd = infd;
		                event.events = EPOLLIN | EPOLLET;

		                //AGREGO LA NUEVA CONEXION A LA INSTANCIA EPOLL
		                if ((s = epoll_ctl (instancia_epoll, EPOLL_CTL_ADD, infd, &event)) == -1){
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
						//LEO LOS DATOS
						count = read (events[i].data.fd, buf, sizeof buf);

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


						//MUESTRO LO QUE RECIBI
						if (write (1, buf, count) == -1){
							perror ("write");
							abort ();
						}

						//Respondo
						send(events[i].data.fd, "Gracias por comunicarse", 23, 0);


					}
					//SI TERMINE CIERRO CONEXION
					if (done){
						Cerrar_Conexion(events[i].data.fd);
					}
				}
			}
	}
	return 0;



}


void Cerrar_Conexion (int fd){

	printf ("Closed connection on descriptor %d\n", fd);
	//CIERRO CONEXION
	close (fd);

}


static int
make_socket_non_blocking (int sfd)
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
