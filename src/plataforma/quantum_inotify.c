#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>

#include "quantum_inotify.h"
#include "plataforma.h"


/* quantum_inotify.c
 * Hilo que detecta cambios en el archivo de quantum (pasado por parametro)
 * y modifica la variable global de quantum. En caso de error, loggea y
 * setea el quantum a DEFAULT_QUANTUM.
 *
 * El hilo termina si hay un error irrecuperable o si el semaforo de plataforma
 * se libera.
 *
 * El formato del archivo de configuracion del quantum es:
 * 		quantum=n
 */
void *monitorear_quantum(void *filename)
{
	t_config *quantum_config;
	int msgs_length, i;
	char inotify_buffer[QUANTUM_IN_BUF_LEN];	// Buffer de mensajes
	int fd;										// File descriptor
	int wd;										// Watch descriptor

	fd = inotify_init();

	if (fd < 0) {
		log_error(Logger, "FATAL: No se pudo crear un FD de inotify! (algo salio muy mal)\n");
		exit(EXIT_FAILURE);
	}

	// Lectura inicial del archivo de configuracion
	quantum_config = config_create((char *)filename);
	wd = inotify_add_watch(fd, (char *)filename, IN_CLOSE_WRITE | IN_DELETE_SELF);

	if (quantum_config != NULL && wd >= 0) {
		actualizar_quantum(quantum_config);
	} else {
		log_error(Logger, "No se pudo leer el archivo de configuracion de quantum! No se podra modificar en runtime.");
		set_default_quantum();
		return NULL;
	}

	// Monitorear eventos todo el tiempo
	while (plataforma_activo(&mutex_plataforma)) {
		i = 0;
		// Leer los eventos del buffer (bloqueante)
		msgs_length = read(fd, inotify_buffer, QUANTUM_IN_BUF_LEN);

		// Manejar los eventos leidos
		while (i < msgs_length) {
			struct inotify_event *event = (struct inotify_event *) & inotify_buffer[i];
			if (event->mask & IN_CLOSE_WRITE) {
				log_info(Logger, "Archivo de quantum modificado");
				actualizar_quantum(quantum_config);
			}
			else if (event->mask & IN_DELETE_SELF) {
				log_warning(Logger, "Archivo de quantum eliminado! No se podra modificar mas en runtime.");
				break;
			}
			i += QUANTUM_IN_EVENT_SIZE + event->len; // Pasar al siguiente mensaje
		}
	}
	
	// Cerrar todo
	log_info(Logger, "Hilo monitor de quantum terminando.");
	inotify_rm_watch(fd, wd);
	close(fd);
	return NULL;
}

/* Devuelve true si plataforma sigue activo, y false si libero el semaforo
 * correspondiente para que el hilo termine correctamente
 */
bool plataforma_activo(pthread_mutex_t *mutex)
{
	if (pthread_mutex_trylock(mutex) == 0) {
		pthread_mutex_unlock(mutex);
		return false;
	} else {
		return true;
	}
}

/* Setea el valor del quantum a QUANTUM_DEFAULT y loggea un warning */
void set_default_quantum(void)
{
	log_warning(Logger, "Seteando valor de quantum al default de %d", QUANTUM_DEFAULT);
	quantum_inicial = QUANTUM_DEFAULT;
}

/* Lee el nuevo valor de quantum del archivo de configuracion y lo actualiza.
 * Loggea y setea a QUANTUM_DEFAULT si hay un error.
 */
void actualizar_quantum(t_config *quantum_config)
{
	int quantum_leido = config_get_int_value(quantum_config, QUANTUM_CONF_KEY);

	if (quantum_leido > 0) {
		log_info(Logger, "Actualizando quantum a %d", quantum_leido);
		quantum_inicial = quantum_leido;
	} else {
		log_warning(Logger, "Valor de quantum invalido o archivo de configuracion incorrecto");
		set_default_quantum();
	}
}
