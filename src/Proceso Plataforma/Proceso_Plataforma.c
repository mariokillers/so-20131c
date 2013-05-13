
#include "Proceso_Plataforma.h"


t_queue* queue_listos = queue_create();
t_queue* queue_florbloq = queue_create();
t_queue* queue_hongobloq = queue_create();
t_queue* queue_monedabloq = queue_create();
t_list* niveles = list_create();
t_list* planificadores = list_create();

int main (){
	pthread_t orq;
	pthread_create( &orq, NULL, orq, NULL );


	return 0;
}

Elemento_personaje* personaje_crear(char* nombre) {
	Elemento_personaje* personaje = malloc(sizeof(Elemento_personaje));
	strncpy(personaje->nombre, nombre, 24);
	return personaje;
}



void* initPanif(void* nivel){
	Nivel* miNivel = (Nivel*)nivel;
	int error;
	Planificador* this_Planificador;
	//CREO LA INSTANCIA DEL PLANIFICADOR
	if((this_Planificador= malloc(sizeof(Planificador)))==-1){
		exit(1);
	}
	//Asigno ID="PX"
	strcpy(this_Planificador->ID, miNivel->ID);
	this_Planificador->ID[0]='P';
	//Los puertos de los planificadores son 5501 5502 5503...
	this_Planificador->PORT= miNivel->ID[1]+5500;
	this_Planificador->IP="localhost";

	list_add(planificadores, this_Planificador);
	initServer(this_Planificador.PORT);


	return 0;


}



void initOrq(void){

	initServer("localhost",5000);


}

void orq (void){
	char estado = STANDBY;

	Mensaje* miMensaje;
	t_queue* queue_mensajes = queue_create();

	while (1){
		switch (estado){
			case STANDBY:
				if(mensajes(queue_mensajes)){
					miMensaje = queue_pop(queue_mensajes);
					switch(miMensaje->type){
						case HANDSHAKE:
							//CONEXION DE NIVEL
							if (miMensaje->data[0]=='N'){
								//CREO LA INSTANCIA DEL THREAD PLANIFICADOR
								pthread_t thr;
								//CREO LA INSTANCIA NIVEL, COPIO LOS PARAMETROS, Y LA AGREGO A LA LISTA
								Nivel* miNivel = malloc(sizeof(Nivel));
								miNivel->FD =
								strcpy(miNivel->ID, ((Nivel*) miMensaje->data)->ID);
								strcpy(miNivel->IP, ((Nivel*) miMensaje->data)->IP);
								miNivel->PORT= ((Nivel*) miMensaje->data)->PORT;
								miNivel->FD= miMensaje->from;
								list_add(miNivel);

								//CREO LA INSTANCIA PLANIFICADOR CORRESPONDIENTE A ESE NIVEL

								//CREO EL THREAD, EL PARAMETRO ES EL NUMERO DE NIVEL
								pthread_create( &thr, NULL, initPanif, (void*) miNivel);

							}
							//CONEXION DE PERSONAJE
							if(miMensaje->data[0]=='P'){
								Personaje* miPersonaje = malloc (sizeof(Personaje));
								strcpy(miPersonaje->ID, ((Personaje*) miMensaje->data)->ID);
								miPersonaje->FD=miMensaje->from;


							}


						break;
						case REQUESTDATANIVEL:
						break;


					}
				}
			break;



		}
	}
}